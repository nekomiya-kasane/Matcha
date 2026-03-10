#include "NyanTetrahedronMeshPlugin.h"

#ifdef _MSC_VER
#define PLUGIN_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define PLUGIN_EXPORT
#endif

namespace nyancad {

auto NyanTetrahedronMeshPlugin::Id() const -> std::string_view { return "nyan-tetrahedron-mesh"; }

auto NyanTetrahedronMeshPlugin::Start([[maybe_unused]] matcha::fw::Shell& shell)
    -> matcha::fw::Expected<void>
{
    return {};
}

void NyanTetrahedronMeshPlugin::Stop() {}

} // namespace nyancad

extern "C" PLUGIN_EXPORT auto CreateExpansionPlugin() -> matcha::fw::IExpansionPlugin* // NOLINT
{
    return new nyancad::NyanTetrahedronMeshPlugin(); // NOLINT
}
