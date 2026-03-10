#pragma once

/**
 * @file NyanCadApp.h
 * @brief NyanCad demo application entry point.
 *
 * Owns the full lifecycle: single-instance guard, Application init,
 * MainWindow setup, PluginHost, business main loop, shutdown.
 */

#include <memory>

namespace nyancad {

/**
 * @brief Top-level NyanCad demo application.
 *
 * Usage from main():
 * ```cpp
 * return nyancad::NyanCadApp{}.Run(argc, argv);
 * ```
 */
class NyanCadApp {
public:
    NyanCadApp();
    ~NyanCadApp();

    NyanCadApp(const NyanCadApp&)            = delete;
    NyanCadApp& operator=(const NyanCadApp&) = delete;
    NyanCadApp(NyanCadApp&&)                 = delete;
    NyanCadApp& operator=(NyanCadApp&&)      = delete;

    /**
     * @brief Run the full application lifecycle.
     * @param argc Argument count from main().
     * @param argv Argument values from main().
     * @return Process exit code (0 = success, 1 = another instance running).
     */
    auto Run(int argc, char** argv) -> int;

private:
    /// @brief Check if another NyanCad instance is already running.
    /// @return true if this is the only instance; false if another exists.
    auto AcquireSingleInstanceLock() -> bool;

    /// @brief Release the single-instance lock.
    void ReleaseSingleInstanceLock();

    /// @brief One tick of business-layer logic (stub — no real CAD engine).
    void BusinessStep();

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace nyancad
