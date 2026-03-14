#pragma once

/**
 * @file PushButtonEventFilter.h
 * @brief Qt event filter that drives PushButtonFsmController (§5.1).
 *
 * Replaces InteractionEventFilter for NyanPushButton with a richer FSM
 * that distinguishes MouseUpInside vs MouseUpOutside, SpaceDown, EnterDown,
 * TabFocus, and integrates ActionGuard for double-click prevention.
 */

#include <Matcha/Foundation/EdgeCaseGuard.h>
#include <Matcha/Foundation/Macros.h>
#include <Matcha/Foundation/WidgetFsm.h>

#include <QObject>

class QWidget;

namespace matcha::fw {
class WidgetNode;
} // namespace matcha::fw

namespace matcha::gui {

class MATCHA_EXPORT PushButtonEventFilter final : public QObject {
    Q_OBJECT
public:
    explicit PushButtonEventFilter(QWidget* watched, fw::WidgetNode* owner);

    [[nodiscard]] auto Controller() const -> const fw::PushButtonFsmController& {
        return _ctrl;
    }
    [[nodiscard]] auto Controller() -> fw::PushButtonFsmController& {
        return _ctrl;
    }

    [[nodiscard]] auto Guard() -> fw::ActionGuard& { return _guard; }

protected:
    auto eventFilter(QObject* obj, QEvent* event) -> bool override;

private:
    fw::PushButtonFsmController _ctrl;
    fw::ActionGuard             _guard;
    fw::WidgetNode*             _owner;
    bool                        _mouseInside = false;
};

} // namespace matcha::gui
