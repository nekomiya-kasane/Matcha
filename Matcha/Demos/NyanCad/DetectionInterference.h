#pragma once

/**
 * @file DetectionInterference.h
 * @brief Interference detection command (stub).
 */

#include <string>

namespace nyancad {

struct DocumentState;

/**
 * @brief Detects interference between parts in the active document.
 */
class DetectionInterference {
public:
    /// @brief Execute stub analysis, return result message for StatusBar.
    [[nodiscard]] auto Execute(DocumentState& doc) -> std::string;
};

} // namespace nyancad
