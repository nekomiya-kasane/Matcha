#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Services/PluginHost.h>
#include <Matcha/Services/IExpansionPlugin.h>
#include <Matcha/UiNodes/Shell/Shell.h>

#include <filesystem>
#include <string>

using namespace matcha::fw;

// MATCHA_TEST_MOCK_PLUGIN_PATH is set by CMake as the full path to MatchaMockPlugin DLL.
#ifndef MATCHA_TEST_MOCK_PLUGIN_PATH
#define MATCHA_TEST_MOCK_PLUGIN_PATH ""
#endif

static const std::string kMockPluginPath = MATCHA_TEST_MOCK_PLUGIN_PATH; // NOLINT

// ============================================================================
// Construction / Destruction
// ============================================================================

TEST_CASE("PluginHost: default construction") {
    PluginHost host;
    CHECK(host.PluginCount() == 0);
}

TEST_CASE("PluginHost: destructor calls StopAll") {
    Shell shell;
    {
        PluginHost host;
        auto result = host.LoadPlugin(kMockPluginPath, shell);
        REQUIRE(result.has_value());
        CHECK(host.PluginCount() == 1);
    }
    // If destructor didn't call StopAll, we'd leak -- no crash = pass
}

// ============================================================================
// LoadPlugin
// ============================================================================

TEST_CASE("PluginHost: LoadPlugin success returns plugin ID") {
    Shell shell;
    PluginHost host;
    auto result = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(result.has_value());
    CHECK(result.value() == "mock-plugin");
    CHECK(host.PluginCount() == 1);
}

TEST_CASE("PluginHost: LoadPlugin invalid path returns PluginLoadFailed") {
    Shell shell;
    PluginHost host;
    auto result = host.LoadPlugin("nonexistent_path.dll", shell);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == ErrorCode::PluginLoadFailed);
    CHECK(host.PluginCount() == 0);
}

TEST_CASE("PluginHost: LoadPlugin duplicate ID returns AlreadyExists") {
    Shell shell;
    PluginHost host;
    auto r1 = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(r1.has_value());
    auto r2 = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE_FALSE(r2.has_value());
    CHECK(r2.error() == ErrorCode::AlreadyExists);
    CHECK(host.PluginCount() == 1);
}

// ============================================================================
// QueryPlugin
// ============================================================================

TEST_CASE("PluginHost: QueryPlugin returns loaded plugin") {
    Shell shell;
    PluginHost host;
    auto result = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(result.has_value());

    auto plugin = host.QueryPlugin("mock-plugin");
    REQUIRE(plugin);
    CHECK(plugin->Id() == "mock-plugin");
}

TEST_CASE("PluginHost: QueryPlugin returns nullptr for unknown ID") {
    PluginHost host;
    auto plugin = host.QueryPlugin("nonexistent");
    CHECK_FALSE(plugin);
}

// ============================================================================
// StopPlugin
// ============================================================================

TEST_CASE("PluginHost: StopPlugin removes plugin") {
    Shell shell;
    PluginHost host;
    auto result = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(result.has_value());

    auto stopResult = host.StopPlugin("mock-plugin");
    CHECK(stopResult.has_value());
    CHECK(host.PluginCount() == 0);
    CHECK_FALSE(host.QueryPlugin("mock-plugin"));
}

TEST_CASE("PluginHost: StopPlugin unknown ID returns NotFound") {
    PluginHost host;
    auto result = host.StopPlugin("nonexistent");
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == ErrorCode::NotFound);
}

// ============================================================================
// StopAll
// ============================================================================

TEST_CASE("PluginHost: StopAll clears all plugins") {
    Shell shell;
    PluginHost host;
    auto r = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(r.has_value());

    host.StopAll();
    CHECK(host.PluginCount() == 0);
}

// ============================================================================
// LoadPluginsFromDirectory
// ============================================================================

TEST_CASE("PluginHost: LoadPluginsFromDirectory nonexistent dir returns NotFound") {
    Shell shell;
    PluginHost host;
    auto result = host.LoadPluginsFromDirectory("nonexistent_directory", shell);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == ErrorCode::NotFound);
}

TEST_CASE("PluginHost: LoadPluginsFromDirectory empty dir returns empty list") {
    Shell shell;
    PluginHost host;

    // Use temp dir that exists but has no plugins
    namespace fs = std::filesystem;
    auto tempDir = fs::temp_directory_path() / "matcha_test_empty_plugins";
    fs::create_directories(tempDir);

    auto result = host.LoadPluginsFromDirectory(tempDir.string(), shell);
    REQUIRE(result.has_value());
    CHECK(result.value().empty());

    fs::remove(tempDir);
}

TEST_CASE("PluginHost: LoadPluginsFromDirectory with mock plugin") {
    Shell shell;
    PluginHost host;

    // Create temp dir and copy/symlink the mock plugin into it
    namespace fs = std::filesystem;
    auto tempDir = fs::temp_directory_path() / "matcha_test_plugins_dir";
    fs::create_directories(tempDir);

    auto mockPath = fs::path(kMockPluginPath);
    auto destPath = tempDir / mockPath.filename();
    std::error_code ec;
    fs::copy_file(mockPath, destPath, fs::copy_options::overwrite_existing, ec);
    REQUIRE_FALSE(ec);

    auto result = host.LoadPluginsFromDirectory(tempDir.string(), shell);
    REQUIRE(result.has_value());
    CHECK(result.value().size() == 1);
    CHECK(result.value()[0] == "mock-plugin");
    CHECK(host.PluginCount() == 1);

    host.StopAll();
    std::error_code cleanupEc;
    fs::remove_all(tempDir, cleanupEc); // May fail on Windows -- DLL stays loaded by design
}

// ============================================================================
// PluginCount
// ============================================================================

TEST_CASE("PluginHost: PluginCount tracks load and stop") {
    Shell shell;
    PluginHost host;
    CHECK(host.PluginCount() == 0);

    auto r = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(r.has_value());
    CHECK(host.PluginCount() == 1);

    host.StopAll();
    CHECK(host.PluginCount() == 0);
}
