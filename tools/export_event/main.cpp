// export_event: dumps one event of the reconstruction output (ROOT) to a JSON file for the web viewer.
//
//  export_event <file.root> --count                            print the number of events
//  export_event <file.root> <event> <out.json> [zMin zMax]     export one event

#include "event_reader.hpp"
#include "json_writer.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr double kDefaultZMin = 550.0;  // start of the detector
constexpr double kDefaultZMax = 880.0;  // end of the detector

int usage() {
    std::cerr << "usage: export_event <file.root> --count\n       export_event <file.root> <event> <out.json> [zMin zMax]\n";
    return 1;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) return usage();
    const std::string rootFile = argv[1];
    const std::string what = argv[2];

    try {
        if (what == "--count") {
            std::cout << evexport::countEvents(rootFile) << "\n";
            return 0;
        }
        if (argc < 4) return usage();

        const long long event = std::stoll(what);
        const std::string outPath = argv[3];
        const double zMin = argc > 4 ? std::stod(argv[4]) : kDefaultZMin;
        const double zMax = argc > 5 ? std::stod(argv[5]) : kDefaultZMax;

        const evexport::Event ev = evexport::readEvent(rootFile, event);
        std::ofstream out(outPath);
        if (!out) throw std::runtime_error("cannot write " + outPath);
        evexport::writeJson(out, ev, zMin, zMax);

        std::cout << "written " << outPath << "\n";
        return 0;
    } catch (const std::invalid_argument&) {
        std::cerr << "error: <event>, zMin and zMax must be numbers\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
    }
    return 1;
}
