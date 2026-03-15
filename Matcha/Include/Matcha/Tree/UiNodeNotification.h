#pragma once

/**
 * @file UiNodeNotification.h
 * @brief Umbrella header -- includes all notification sub-headers.
 *
 * Existing code that includes this file continues to compile unchanged.
 * New code should prefer the narrower sub-headers:
 *
 *  - WidgetNotification.h        Widget/Control/Container/DnD/Crosscut
 *  - TabNotification.h           TabBarNode tab-page operations
 *  - DocumentNotification.h      DocumentArea/DocumentManager/ViewportGroup
 *  - WorkbenchNotification.h     Workshop/Workbench activation lifecycle
 */

#include "Matcha/Tree/WidgetNotification.h"
#include "Matcha/Tree/Composition/Document/TabNotification.h"
#include "Matcha/Tree/Composition/Document/DocumentNotification.h"
#include "Matcha/Tree/Composition/Workbench/WorkbenchNotification.h"
