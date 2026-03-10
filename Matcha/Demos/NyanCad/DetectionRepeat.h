#pragma once

/**
 * @file DetectionRepeat.h
 * @brief Repeat pattern detection command (stub).
 */

#include <string>

namespace nyancad {

struct DocumentState;

/**
 * @brief Detects repeated geometry patterns in the active document.
 */
class DetectionRepeat {
public:
    /// @brief Execute stub analysis, return result message for StatusBar.
    [[nodiscard]] auto Execute(DocumentState& doc) -> std::string;
};

} // namespace nyancad
