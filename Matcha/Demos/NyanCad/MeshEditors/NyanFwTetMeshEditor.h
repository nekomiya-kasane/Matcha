#pragma once

/**
 * @file NyanFwTetMeshEditor.h
 * @brief Tetrahedral mesh editor — registers ActionBar tab with stub tools.
 */

namespace matcha::fw {
class Application;
} // namespace matcha::fw

namespace nyancad {

/**
 * @brief Tet mesh editor. Registers "Tet Mesh" ActionBar tab.
 */
class NyanFwTetMeshEditor {
public:
    void Start(matcha::fw::Application& app);
    void Stop();

private:
    bool _started = false;
};

} // namespace nyancad
