#include "json_writer.hpp"

#include <iomanip>

namespace evexport {
namespace {

const char* sep(size_t i, size_t n) { return i + 1 < n ? "," : ""; }

}  // namespace

void writeJson(std::ostream& out, const Event& ev, double zMin, double zMax) {
    out << std::setprecision(10);
    out << "{\n  \"event\": " << ev.number << ",\n";
    out << "  \"zRange\": [" << zMin << ", " << zMax << "],\n";

    out << "  \"hits\": [\n";
    for (size_t i = 0; i < ev.hits.size(); ++i) {
        const Hit& h = ev.hits[i];
        out << "    {\"station\": " << h.station << ", \"module\": " << h.module
            << ", \"z\": " << h.z << ", \"coord\": " << h.coord
            << ", \"posInModule\": " << h.posInModule << "}" << sep(i, ev.hits.size()) << "\n";
    }
    out << "  ],\n";

    out << "  \"tracks\": [\n";
    for (size_t i = 0; i < ev.tracks.size(); ++i) {
        const Track& t = ev.tracks[i];
        const double x1 = t.x0 + t.xSlope * (zMin - t.z0);
        const double y1 = t.y0 + t.ySlope * (zMin - t.z0);
        const double x2 = t.x0 + t.xSlope * (zMax - t.z0);
        const double y2 = t.y0 + t.ySlope * (zMax - t.z0);
        out << "    {\"p1\": [" << x1 << ", " << y1 << ", " << zMin << "], "
            << "\"p2\": [" << x2 << ", " << y2 << ", " << zMax << "]}"
            << sep(i, ev.tracks.size()) << "\n";
    }
    out << "  ],\n";

    out << "  \"vertices\": [\n";
    for (size_t i = 0; i < ev.vertices.size(); ++i) {
        const Vertex& v = ev.vertices[i];
        out << "    {\"pos\": [" << v.x << ", " << v.y << ", " << v.z << "], "
            << "\"station\": " << v.station << ", "
            << "\"muonTheta\": " << v.muonTheta << ", \"electronTheta\": " << v.electronTheta << "}"
            << sep(i, ev.vertices.size()) << "\n";
    }
    out << "  ]\n}\n";
}

}  // namespace evexport
