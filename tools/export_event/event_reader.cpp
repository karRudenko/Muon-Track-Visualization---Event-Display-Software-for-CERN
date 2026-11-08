#include "event_reader.hpp"

#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

#include <memory>
#include <stdexcept>

namespace evexport {
namespace {


struct OpenTree {
    std::unique_ptr<TFile> file;
    TTree* tree = nullptr;
};

OpenTree openTree(const std::string& fname) {
    OpenTree t;
    t.file.reset(TFile::Open(fname.c_str(), "READ"));
    if (!t.file || t.file->IsZombie()) {
        throw std::runtime_error("cannot open " + fname);
    }
    t.tree = t.file->Get<TTree>("cbmsim");
    if (!t.tree) {
        throw std::runtime_error("no tree cbmsim in " + fname);
    }
    return t;
}

}  // namespace

long long countEvents(const std::string& rootFile) {
    return openTree(rootFile).tree->GetEntries();
}

Event readEvent(const std::string& rootFile, long long event) {
    OpenTree t = openTree(rootFile);

    const long long n = t.tree->GetEntries();
    if (event < 0 || event >= n) {
        throw std::out_of_range("event " + std::to_string(event) + " out of range (0.." + std::to_string(n - 1) + ")");
    }

    TTreeReader reader(t.tree);

    // hits
    TTreeReaderArray<double> hZ (reader, "ReconstructedHits.Z");
    TTreeReaderArray<double> hPos (reader, "ReconstructedHits.Position");
    TTreeReaderArray<double> hPerp(reader, "ReconstructedHits.PositionPerpendicular");
    TTreeReaderArray<int> hSt (reader, "ReconstructedHits.StationID");
    TTreeReaderArray<int> hMod (reader, "ReconstructedHits.ModuleID");

    // reconstructed tracks: straight line through (x0, y0, z0) with slopes
    TTreeReaderArray<double> tX0(reader, "ReconstructedTracks.X0");
    TTreeReaderArray<double> tXS(reader, "ReconstructedTracks.XSlope");
    TTreeReaderArray<double> tY0(reader, "ReconstructedTracks.Y0");
    TTreeReaderArray<double> tYS(reader, "ReconstructedTracks.YSlope");
    TTreeReaderArray<double> tZ0(reader, "ReconstructedTracks.Z0");

    // best vertices
    TTreeReaderArray<double> vX (reader, "BestReconstructedVertices.XPositionFit");
    TTreeReaderArray<double> vY (reader, "BestReconstructedVertices.YPositionFit");
    TTreeReaderArray<double> vZ (reader, "BestReconstructedVertices.ZPositionFit");
    TTreeReaderArray<int> vSt(reader, "BestReconstructedVertices.StationIndex");
    TTreeReaderArray<double> vMu(reader, "BestReconstructedVertices.MuonTheta");
    TTreeReaderArray<double> vEl(reader, "BestReconstructedVertices.ElectronTheta");

    if (reader.SetEntry(event) != TTreeReader::kEntryValid) {
        throw std::runtime_error("cannot read event " + std::to_string(event));
    }

    Event ev;
    ev.number = event;
    for (size_t i = 0; i < hZ.GetSize(); ++i) {
        ev.hits.push_back({hSt[i], hMod[i], hZ[i], hPerp[i], hPos[i]});
    }
    for (size_t i = 0; i < tX0.GetSize(); ++i) {
        ev.tracks.push_back({tX0[i], tXS[i], tY0[i], tYS[i], tZ0[i]});
    }
    for (size_t i = 0; i < vX.GetSize(); ++i) {
        ev.vertices.push_back({vX[i], vY[i], vZ[i], vSt[i], vMu[i], vEl[i]});
    }
    return ev;
}

}  // namespace evexport
