#pragma once

/**
 * @file DetectionFailureArea.h
 * @brief Failure area detection command (stub).
 */

#include <string>

namespace nyancad {

struct DocumentState;

/**
 * @brief Detects potential failure areas (thin walls, sharp edges) in the active document.
 */
class DetectionFailureArea {
public:
    /// @brief Execute stub analysis, return result message for StatusBar.
    [[nodiscard]] auto Execute(DocumentState& doc) -> std::string;
};

} // namespace nyancad
