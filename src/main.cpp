#include <iostream>
#include "analyze.hpp"
#include <chrono>

int main() {
    auto t0 = std::chrono::steady_clock::now();
    auto tracks = analyze("simple_output.root", 5);
    auto t1 = std::chrono::steady_clock::now();
    for(const auto& t : tracks) 
    {
        std::cout << "[" << t.event << ", " << t.index
                  << " : " << t.x0 << ", " << t.xSlope
                  << "; " << t.y0 << ", " << t.ySlope 
                  << "; " << t.z0 << "]\n";
    }
    std::cout << tracks.size() << " tracks in "
          << std::chrono::duration<double>(t1 - t0).count() << " s\n";

    return 0;
}
