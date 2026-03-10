#pragma once

/**
 * @file DetectionFreedomEdge.h
 * @brief Freedom edge detection command (stub).
 */

#include <string>

namespace nyancad {

struct DocumentState;

/**
 * @brief Detects freedom (free) edges in the active document mesh.
 */
class DetectionFreedomEdge {
public:
    /// @brief Execute stub analysis, return result message for StatusBar.
    [[nodiscard]] auto Execute(DocumentState& doc) -> std::string;
};

} // namespace nyancad
