#pragma once
#include <vector>
#include <string>

struct Track {
    long long event;
    int index;
    double x0, xSlope;
    double y0, ySlope;
    double z0;
};

std::vector<Track> analyze(const std::string& fname);
std::vector<Track> analyze(const std::string& fname, long long event);
