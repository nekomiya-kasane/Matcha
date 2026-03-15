#pragma once

/**
 * @file IExpansionPlugin.h
 * @brief Interface for expansion plugins loaded by PluginHost.
 *
 * Plugins implement this interface to integrate with the Matcha framework.
 * The PluginHost calls Start(Shell&) after loading and Stop() before unloading.
 *
 * @see 05_Greenfield_Plan.md ss 8.2.3 for the plugin lifecycle spec.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Core/Types.h>

#include <string_view>

namespace matcha::fw {

class Shell;

/**
 * @brief Abstract interface for expansion plugins.
 *
 * **Lifecycle**:
 * 1. PluginHost loads the shared library
 * 2. Factory function creates an IExpansionPlugin instance
 * 3. PluginHost calls `Start(Shell&)` — plugin registers ActionTabs, callbacks, etc.
 * 4. On shutdown, PluginHost calls `Stop()` — plugin cleans up
 * 5. PluginHost destroys the instance and unloads the library
 */
class MATCHA_EXPORT IExpansionPlugin {
public:
    virtual ~IExpansionPlugin() = default;

    IExpansionPlugin(const IExpansionPlugin&)            = default;
    IExpansionPlugin& operator=(const IExpansionPlugin&) = default;
    IExpansionPlugin(IExpansionPlugin&&)                 = default;
    IExpansionPlugin& operator=(IExpansionPlugin&&)      = default;

protected:
    IExpansionPlugin() = default;

public:

    /** @brief Unique plugin identifier string. */
    [[nodiscard]] virtual auto Id() const -> std::string_view = 0;

    /** @brief Called after loading. Plugin should register its UI components. */
    [[nodiscard]] virtual auto Start(Shell& shell) -> Expected<void> = 0;

    /** @brief Called before unloading. Plugin should clean up. */
    virtual void Stop() = 0;
};

} // namespace matcha::fw
