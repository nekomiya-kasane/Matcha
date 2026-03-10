/**
 * @file DetectionFailureArea.cpp
 * @brief Failure area detection command implementation.
 */

#include "DetectionFailureArea.h"

#include "NyanCadDocument.h"

namespace nyancad {

auto DetectionFailureArea::Execute(DocumentState& doc) -> std::string
{
    // Stub: pretend we found 1 failure area
    doc.modified = true;
    return "FailureArea: 1 thin-wall region in '" + doc.name + "'";
}

} // namespace nyancad
