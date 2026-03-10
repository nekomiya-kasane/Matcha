#pragma once

/**
 * @file PluginHost.h
 * @brief Plugin loading and lifecycle management service.
 *
 * Loads IExpansionPlugin implementations from shared libraries,
 * manages their Start/Stop lifecycle, and provides query APIs.
 *
 * @see 05_Greenfield_Plan.md ss 8.2.3 for the PluginHost spec.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Foundation/Types.h>

#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

class Shell;
class IExpansionPlugin;

/**
 * @brief Service for loading and managing expansion plugins.
 *
 * **Usage**:
 * ```cpp
 * PluginHost host;
 * host.LoadPluginsFromDirectory("plugins/", shell);
 * // ... application runs ...
 * host.StopAll();
 * ```
 */
class MATCHA_EXPORT PluginHost {
public:
    PluginHost();
    ~PluginHost();

    PluginHost(const PluginHost&)            = delete;
    PluginHost& operator=(const PluginHost&) = delete;
    PluginHost(PluginHost&&)                 = delete;
    PluginHost& operator=(PluginHost&&)      = delete;

    /** @brief Load a single plugin from a shared library path. Returns plugin ID on success. */
    [[nodiscard]] auto LoadPlugin(std::string_view libraryPath, Shell& shell)
        -> Expected<std::string>;

    /** @brief Load all plugins from a directory (scans for .dll/.so/.dylib). Returns loaded IDs. */
    [[nodiscard]] auto LoadPluginsFromDirectory(std::string_view directoryPath, Shell& shell)
        -> Expected<std::vector<std::string>>;

    /** @brief Stop a single plugin by ID. */
    auto StopPlugin(std::string_view pluginId) -> Expected<void>;

    /** @brief Stop all loaded plugins (calls Stop() on each in reverse order). */
    void StopAll();

    /** @brief Query a loaded plugin by ID. Returns nullptr if not found. */
    [[nodiscard]] auto QueryPlugin(std::string_view pluginId) const
        -> observer_ptr<IExpansionPlugin>;

    /** @brief Number of currently loaded plugins. */
    [[nodiscard]] auto PluginCount() const -> size_t;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace matcha::fw
