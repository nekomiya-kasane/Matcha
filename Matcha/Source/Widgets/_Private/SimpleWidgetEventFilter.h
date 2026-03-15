#pragma once

/**
 * @file SimpleWidgetEventFilter.h
 * @brief Qt event filter driving SimpleWidgetFsmController for generic widgets.
 *
 * Replaces the old InteractionEventFilter. Handles basic hover/press/focus/disabled
 * events and maps them through the unified WidgetFsm architecture.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Tree/FSM/WidgetFsm.h>

#include <QObject>

class QWidget;

namespace matcha::fw {
class WidgetNode;
} // namespace matcha::fw

namespace matcha::gui {

class MATCHA_EXPORT SimpleWidgetEventFilter final : public QObject {
    Q_OBJECT
public:
    explicit SimpleWidgetEventFilter(QWidget* watched, fw::WidgetNode* owner);

    [[nodiscard]] auto Controller() const -> const fw::SimpleWidgetFsmController& { return _ctrl; }
    [[nodiscard]] auto Controller() -> fw::SimpleWidgetFsmController& { return _ctrl; }

protected:
    auto eventFilter(QObject* obj, QEvent* event) -> bool override;

private:
    fw::SimpleWidgetFsmController _ctrl;
    fw::WidgetNode*               _owner;
};

} // namespace matcha::gui
