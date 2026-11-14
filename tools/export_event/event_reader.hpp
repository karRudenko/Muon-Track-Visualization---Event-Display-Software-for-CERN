#pragma once

#include "event_data.hpp"

#include <string>

namespace evexport {

// Number of events in the file.
long long countEvents(const std::string& rootFile);

// Hits, reconstructed tracks and best vertices of one event.
Event readEvent(const std::string& rootFile, long long event);

}  // namespace evexport
