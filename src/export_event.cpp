#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

// Имя ветки: пробуем без префикса и с префиксом ReconstructionOutput
static std::string br(TTree* t, const std::string& name) {
    if (t->GetBranch(name.c_str())) {
        return name;
    }
    const std::string alt = "ReconstructionOutput." + name;
    if (t->GetBranch(alt.c_str())) {
        return alt;
    }
    std::cerr << "WARNING: branch not found: " << name << "\n";
    return name;
}

static const char* sep(size_t i, size_t n) { return i + 1 < n ? "," : ""; }

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "usage: export_event <file.root> <event> <out.json> [zMin zMax]\n";
        return 1;
    }
    const std::string fname = argv[1];
    const Long64_t event = std::stoll(argv[2]);
    const std::string outPath = argv[3];
    const double zMin = argc > 4 ? std::stod(argv[4]) : 550.0;  //начало детектора, см
    const double zMax = argc > 5 ? std::stod(argv[5]) : 880.0;  //конец детектора, см

    std::unique_ptr<TFile> f(TFile::Open(fname.c_str(), "READ"));
    if (!f || f->IsZombie()) { 
        std::cerr << "cannot open " << fname << "\n"; 
        return 1; 
    }

    TTree* tree = f->Get<TTree>("cbmsim");
    if (!tree) { 
        std::cerr << "no tree cbmsim\n"; 
        return 1; 
    }
    if (event < 0 || event >= tree->GetEntries()) {
        std::cerr << "event " << event << " out of range (0.." << tree->GetEntries() - 1 << ")\n";
        return 1;
    }

    TTreeReader reader(tree);

    // ---хиты (одно измерение на хит, см. раздел про полоски)
    TTreeReaderArray<double> hZ  (reader, br(tree, "ReconstructedHits.Z").c_str());
    TTreeReaderArray<double> hPos(reader, br(tree, "ReconstructedHits.Position").c_str());
    TTreeReaderArray<double> hPerp(reader, br(tree, "ReconstructedHits.PositionPerpendicular").c_str());
    TTreeReaderArray<int>    hSt (reader, br(tree, "ReconstructedHits.StationID").c_str());
    TTreeReaderArray<int>    hMod(reader, br(tree, "ReconstructedHits.ModuleID").c_str());

    // ----- реконструированные треки: прямая через(x0, y0, z0) с наклонами 
    TTreeReaderArray<double> tX0(reader, br(tree, "ReconstructedTracks.X0").c_str());
    TTreeReaderArray<double> tXS(reader, br(tree, "ReconstructedTracks.XSlope").c_str());
    TTreeReaderArray<double> tY0(reader, br(tree, "ReconstructedTracks.Y0").c_str());
    TTreeReaderArray<double> tYS(reader, br(tree, "ReconstructedTracks.YSlope").c_str());
    TTreeReaderArray<double> tZ0(reader, br(tree, "ReconstructedTracks.Z0").c_str());

    // ----лучшие вершины(по одной на мишень)
    TTreeReaderArray<double> vX (reader, br(tree, "BestReconstructedVertices.XPositionFit").c_str());
    TTreeReaderArray<double> vY (reader, br(tree, "BestReconstructedVertices.YPositionFit").c_str());
    TTreeReaderArray<double> vZ (reader, br(tree, "BestReconstructedVertices.ZPositionFit").c_str());
    TTreeReaderArray<int>   vSt(reader, br(tree, "BestReconstructedVertices.StationIndex").c_str());
    TTreeReaderArray<double> vMu(reader, br(tree, "BestReconstructedVertices.MuonTheta").c_str());
    TTreeReaderArray<double> vEl(reader, br(tree, "BestReconstructedVertices.ElectronTheta").c_str());

    if (reader.SetEntry(event) != TTreeReader::kEntryValid) {
        std::cerr << "cannot read event " << event << "\n";
        return 1;
    }

    std::ofstream out(outPath);
    out << std::setprecision(10);
    out << "{\n  \"event\": " << event << ",\n";
    out << "  \"zRange\": [" << zMin << ", " << zMax << "],\n";

    // hits: coord = PositionPerpendicular = координата вдоль направления измерения модуля
    out << "  \"hits\": [\n";
    for (size_t i = 0; i < hZ.GetSize(); ++i){
        out << "    {\"station\": " << hSt[i] << ", \"module\": " << hMod[i]
            << ", \"z\": " << hZ[i] << ", \"coord\": " << hPerp[i]
            << ", \"posInModule\": " << hPos[i] << "}" << sep(i, hZ.GetSize()) << "\n";
    }       
    out << "  ],\n";

    
    //   tracks:   x(z) = X0 + XSlope * (z - Z0),   y(z) = Y0 + YSlope * (z - Z0)
    out << "  \"tracks\": [\n";
    for(size_t i = 0; i < tX0.GetSize(); ++i) {
        const double x1 = tX0[i] + tXS[i] * (zMin - tZ0[i]);
        const double y1 = tY0[i] + tYS[i] * (zMin - tZ0[i]);
        const double x2 = tX0[i] + tXS[i] * (zMax - tZ0[i]);
        const double y2 = tY0[i] + tYS[i] * (zMax - tZ0[i]);
        out << "    {\"p1\": [" << x1 << ", " << y1 << ", " << zMin << "], "
            << "\"p2\": [" << x2 << ", " << y2 << ", " << zMax << "]}"
            << sep(i, tX0.GetSize()) << "\n";
    }
    out << "  ],\n";

    out << "  \"vertices\": [\n";
    for (size_t i = 0; i < vX.GetSize(); ++i){
        out << "    {\"pos\": [" << vX[i] << ", " << vY[i] << ", " << vZ[i] << "], "
            << "\"station\": " << vSt[i] << ", "
            << "\"muonTheta\": " << vMu[i] << ", \"electronTheta\": " << vEl[i] << "}"
            << sep(i, vX.GetSize()) << "\n";
    }   
    out << "  ]\n}\n";

    std::cout << "written " << outPath << "\n";
    return 0;
}