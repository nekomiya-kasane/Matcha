/**
 * @file DetectionInterference.cpp
 * @brief Interference detection command implementation.
 */

#include "DetectionInterference.h"

#include "NyanCadDocument.h"

namespace nyancad {

auto DetectionInterference::Execute(DocumentState& doc) -> std::string
{
    // Stub: pretend we found 0 interferences
    return "Interference: 0 collisions in '" + doc.name + "'";
}

} // namespace nyancad
