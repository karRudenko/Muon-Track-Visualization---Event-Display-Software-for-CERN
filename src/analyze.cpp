#include "analyze.hpp"

#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TError.h"
#include "TROOT.h"

#include <vector>
#include <string>

std::vector<Track> analyze(const std::string& fname) {
    gErrorIgnoreLevel = kError;
    ROOT::EnableImplicitMT(); 
    TFile *f = TFile::Open(fname.c_str(), "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "Error: Cannot open file '*.root'.\n";
        return {};
    }
    TTreeReader reader("cbmsim", f);

    TTreeReaderArray<double> x0(reader, "ReconstructedTracks.X0");
    TTreeReaderArray<double> xs(reader, "ReconstructedTracks.XSlope");
    TTreeReaderArray<double> y0(reader, "ReconstructedTracks.Y0");
    TTreeReaderArray<double> ys(reader, "ReconstructedTracks.YSlope");
    TTreeReaderArray<double> z0(reader, "ReconstructedTracks.Z0");

    std::vector<Track> h;

    while (reader.Next()) {
        long long ev = reader.GetCurrentEntry();
        for (size_t i = 0; i < x0.GetSize(); ++i) {
            h.push_back({ev, (int)i, x0[i], xs[i], y0[i], ys[i], z0[i]});
        }
    }
    f->Close(); delete f;
    return h;
}

std::vector<Track> analyze(const std::string& fname, Long64_t event) {
    gErrorIgnoreLevel = kError;
    ROOT::EnableImplicitMT(); 
    TFile *f = TFile::Open(fname.c_str(), "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "Error: Cannot open file '*.root'.\n";
        return {};
    }
    TTreeReader reader("cbmsim", f);

    TTreeReaderArray<double> x0(reader, "ReconstructedTracks.X0");
    TTreeReaderArray<double> xs(reader, "ReconstructedTracks.XSlope");
    TTreeReaderArray<double> y0(reader, "ReconstructedTracks.Y0");
    TTreeReaderArray<double> ys(reader, "ReconstructedTracks.YSlope");
    TTreeReaderArray<double> z0(reader, "ReconstructedTracks.Z0");

    std::vector<Track> h;

    if (reader.SetEntry(event) != TTreeReader::kEntryValid) {
        std::cerr << "Event " << event << " does not exist (file has "
                  << reader.GetEntries() << " events)\n";
        f->Close(); delete f;
        return h;
    }

    for (size_t i = 0; i < x0.GetSize(); ++i) {
        h.push_back({event, (int)i, x0[i], xs[i], y0[i], ys[i], z0[i]});
    }

    f->Close(); delete f;
    return h;
}