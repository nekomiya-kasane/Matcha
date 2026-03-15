#pragma once

/**
 * @file WorkbenchNotification.h
 * @brief Notification types for Workshop/Workbench activation lifecycle.
 *
 * These notifications are dispatched by WorkbenchManager during
 * workshop/workbench activation and deactivation transitions.
 * They live at the Workbench orchestration layer.
 */

#include "Matcha/Event/Notification.h"

#include <string>
#include <string_view>

namespace matcha::fw {

// =========================================================================== //
//  Workbench notifications
// =========================================================================== //

class MATCHA_EXPORT WorkshopActivated final : public Notification {
public:
    explicit WorkshopActivated(std::string workshopId = {})
        : _workshopId(std::move(workshopId)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "WorkshopActivated"; }
    [[nodiscard]] auto GetWorkshopId() const -> std::string_view { return _workshopId; }
private:
    std::string _workshopId;
};

class MATCHA_EXPORT WorkshopDeactivated final : public Notification {
public:
    explicit WorkshopDeactivated(std::string workshopId = {})
        : _workshopId(std::move(workshopId)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "WorkshopDeactivated"; }
    [[nodiscard]] auto GetWorkshopId() const -> std::string_view { return _workshopId; }
private:
    std::string _workshopId;
};

class MATCHA_EXPORT WorkbenchActivated final : public Notification {
public:
    explicit WorkbenchActivated(std::string workbenchId = {}, std::string workshopId = {})
        : _workbenchId(std::move(workbenchId)), _workshopId(std::move(workshopId)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "WorkbenchActivated"; }
    [[nodiscard]] auto GetWorkbenchId() const -> std::string_view { return _workbenchId; }
    [[nodiscard]] auto GetWorkshopId() const -> std::string_view { return _workshopId; }
private:
    std::string _workbenchId;
    std::string _workshopId;
};

class MATCHA_EXPORT WorkbenchDeactivated final : public Notification {
public:
    explicit WorkbenchDeactivated(std::string workbenchId = {})
        : _workbenchId(std::move(workbenchId)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "WorkbenchDeactivated"; }
    [[nodiscard]] auto GetWorkbenchId() const -> std::string_view { return _workbenchId; }
private:
    std::string _workbenchId;
};

class MATCHA_EXPORT CommandInvoked final : public Notification {
public:
    explicit CommandInvoked(std::string cmdHeaderId = {})
        : _cmdHeaderId(std::move(cmdHeaderId)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "CommandInvoked"; }
    [[nodiscard]] auto GetCmdHeaderId() const -> std::string_view { return _cmdHeaderId; }
private:
    std::string _cmdHeaderId;
};

} // namespace matcha::fw
