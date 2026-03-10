#pragma once

/**
 * @file NyanFwSketchEditor.h
 * @brief Sketch editor — registers ActionBar tab with stub tools.
 */

namespace matcha::fw {
class Application;
} // namespace matcha::fw

namespace nyancad {

/**
 * @brief Sketch editor. Registers "Sketch" ActionBar tab.
 */
class NyanFwSketchEditor {
public:
    void Start(matcha::fw::Application& app);
    void Stop();

private:
    bool _started = false;
};

} // namespace nyancad
