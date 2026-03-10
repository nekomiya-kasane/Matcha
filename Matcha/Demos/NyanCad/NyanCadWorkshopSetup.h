#pragma once

/**
 * @file NyanCadWorkshopSetup.h
 * @brief Registers NyanCad Workshop/Workbench descriptors into the framework registry.
 */

namespace matcha::fw {
class WorkshopRegistry;
} // namespace matcha::fw

namespace nyancad {

/**
 * @brief Register all NyanCad Workshop and Workbench descriptors.
 *
 * Call once after Application::Initialize(), before ActivateWorkshop().
 * Creates:
 *   - "mesh" Workshop (base tabs: File, Edit, Mesh)
 *   - "surface_mesh" Workbench (task tab: Surface Mesh)
 *   - "tet_mesh" Workbench (task tab: Tet Mesh)
 *   - "sketch" Workbench (task tab: Sketch)
 */
void RegisterNyanCadWorkshops(matcha::fw::WorkshopRegistry& registry);

} // namespace nyancad
