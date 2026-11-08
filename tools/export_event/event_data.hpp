#pragma once
// Plain data describing one reconstructed event

#include <vector>

namespace evexport {

// One measurement in one module.
struct Hit {
    int station;
    int module;
    double z;
    double coord;
    double posInModule;
};

// Straight line:  x(z) = x0 + xSlope * (z - z0),  y(z) = y0 + ySlope * (z - z0)
struct Track {
    double x0, xSlope;
    double y0, ySlope;
    double z0;
};

struct Vertex {
    double x, y, z;
    int station;
    double muonTheta;
    double electronTheta;
};

struct Event {
    long long number = 0;
    std::vector<Hit> hits;
    std::vector<Track> tracks;
    std::vector<Vertex> vertices;
};

}  // namespace evexport
