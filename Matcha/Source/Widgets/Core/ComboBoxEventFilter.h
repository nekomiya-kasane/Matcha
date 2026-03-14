#pragma once

/**
 * @file ComboBoxEventFilter.h
 * @brief Qt event filter that drives ComboBoxFsmController (§5.6).
 *
 * Distinguishes Closed/Hover/Open/Disabled states and handles
 * ActivateOpen, ClickOutside, Escape, SelectItem, ArrowNavigate events.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Foundation/WidgetFsm.h>

#include <QObject>

class QWidget;

namespace matcha::fw {
class WidgetNode;
} // namespace matcha::fw

namespace matcha::gui {

class MATCHA_EXPORT ComboBoxEventFilter final : public QObject {
    Q_OBJECT
public:
    explicit ComboBoxEventFilter(QWidget* watched, fw::WidgetNode* owner);

    [[nodiscard]] auto Controller() const -> const fw::ComboBoxFsmController& { return _ctrl; }
    [[nodiscard]] auto Controller() -> fw::ComboBoxFsmController& { return _ctrl; }

    void NotifyPopupOpened();
    void NotifyPopupClosed();
    void NotifyItemSelected();
    void NotifyEscape();

protected:
    auto eventFilter(QObject* obj, QEvent* event) -> bool override;

private:
    fw::ComboBoxFsmController _ctrl;
    fw::WidgetNode*           _owner;
};

} // namespace matcha::gui
