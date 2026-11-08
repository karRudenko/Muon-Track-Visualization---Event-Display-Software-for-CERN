#pragma once
// Serialising an event to the JSON format read by web/ (see README, "Event JSON format").

#include "event_data.hpp"

#include <ostream>

namespace evexport {

// Tracks are written as straight segments between z = zMin and z = zMax.
void writeJson(std::ostream& out, const Event& ev, double zMin, double zMax);

}  // namespace evexport
