#pragma once

/**
 * @file NyanFwSurfaceMeshEditor.h
 * @brief Surface mesh editor — registers ActionBar tab with stub tools.
 */

namespace matcha::fw {
class Application;
} // namespace matcha::fw

namespace nyancad {

/**
 * @brief Surface mesh editor. Registers "Surface Mesh" ActionBar tab.
 */
class NyanFwSurfaceMeshEditor {
public:
    void Start(matcha::fw::Application& app);
    void Stop();

private:
    bool _started = false;
};

} // namespace nyancad
