#include "analyze.hpp"

#include "TFile.h"
#include "TTree.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

// ---- minimal test helpers ----
static int g_failed = 0;
#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  "      \
                      << #cond << "\n";                                      \
            ++g_failed;                                                      \
        }                                                                    \
    } while (0)

static bool near(double a, double b) { return std::fabs(a - b) < 1e-12; }

// ---- builds a tiny file shaped like the real one ----
// event 0: 2 tracks, event 1: 0 tracks, event 2: 3 tracks
static const char* kFile = "test_tracks.root";

static void makeTestFile() {
    TFile f(kFile, "RECREATE");
    TTree t("cbmsim", "test");

    std::vector<double> x0, xs, y0, ys, z0;
    t.Branch("ReconstructedTracks.X0", &x0);
    t.Branch("ReconstructedTracks.XSlope", &xs);
    t.Branch("ReconstructedTracks.Y0", &y0);
    t.Branch("ReconstructedTracks.YSlope", &ys);
    t.Branch("ReconstructedTracks.Z0", &z0);

    auto fill = [&](int n, double base) {
        x0.clear(); xs.clear(); y0.clear(); ys.clear(); z0.clear();
        for (int i = 0; i < n; ++i) {
            x0.push_back(base + i);
            xs.push_back(base * 0.001 + i * 0.0001);
            y0.push_back(-base - i);
            ys.push_back(-base * 0.001);
            z0.push_back(0.0);
        }
        t.Fill();
    };
    fill(2, 10.0);   // event 0
    fill(0, 0.0);    // event 1 (empty)
    fill(3, 20.0);   // event 2

    t.Write();
    f.Close();
}

// ---- tests ----
static void testAllEvents() {
    auto h = analyze(kFile);
    CHECK(h.size() == 5);

    // event numbering and track index
    CHECK(h[0].event == 0 && h[0].index == 0);
    CHECK(h[1].event == 0 && h[1].index == 1);
    CHECK(h[2].event == 2 && h[2].index == 0);
    CHECK(h[4].event == 2 && h[4].index == 2);

    // values
    CHECK(near(h[0].x0, 10.0));
    CHECK(near(h[1].x0, 11.0));
    CHECK(near(h[1].y0, -11.0));
    CHECK(near(h[2].x0, 20.0));
    CHECK(near(h[4].x0, 22.0));
    CHECK(near(h[0].z0, 0.0));
}

static void testSingleEvent() {
    auto e0 = analyze(kFile, 0);
    CHECK(e0.size() == 2);
    for (const auto& t : e0) CHECK(t.event == 0);

    auto e1 = analyze(kFile, 1);
    CHECK(e1.empty());

    auto e2 = analyze(kFile, 2);
    CHECK(e2.size() == 3);
    CHECK(e2[0].index == 0 && e2[2].index == 2);
    CHECK(near(e2[2].x0, 22.0));
}

static void testSingleMatchesAll() {
    auto all = analyze(kFile);
    for (long long ev = 0; ev < 3; ++ev) {
        auto one = analyze(kFile, ev);
        std::vector<Track> filtered;
        for (const auto& t : all)
            if (t.event == ev) filtered.push_back(t);

        CHECK(one.size() == filtered.size());
        for (size_t i = 0; i < one.size() && i < filtered.size(); ++i) {
            CHECK(one[i].index == filtered[i].index);
            CHECK(near(one[i].x0, filtered[i].x0));
            CHECK(near(one[i].xSlope, filtered[i].xSlope));
            CHECK(near(one[i].y0, filtered[i].y0));
            CHECK(near(one[i].ySlope, filtered[i].ySlope));
        }
    }
}

static void testInvalidInput() {
    CHECK(analyze("does_not_exist.root").empty());
    CHECK(analyze("does_not_exist.root", 0).empty());
    CHECK(analyze(kFile, 3).empty());
    CHECK(analyze(kFile, 9999).empty());
    CHECK(analyze(kFile, -1).empty());
}

int main() {
    makeTestFile();

    testAllEvents();
    testSingleEvent();
    testSingleMatchesAll();
    testInvalidInput();

    std::remove(kFile);

    if (g_failed == 0) {
        std::cout << "All tests passed\n";
        return 0;
    }
    std::cerr << g_failed << " check(s) failed\n";
    return 1;
}