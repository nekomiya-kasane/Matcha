#pragma once

/**
 * @file LineEditEventFilter.h
 * @brief Qt event filter that drives LineEditFsmController (§5.3).
 *
 * Richer FSM than generic InteractionEventFilter: distinguishes
 * Hover, Click→Focused, FocusLost, ValidationFail/Pass, ReadOnly.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Foundation/WidgetFsm.h>

#include <QObject>

class QWidget;

namespace matcha::fw {
class WidgetNode;
} // namespace matcha::fw

namespace matcha::gui {

class MATCHA_EXPORT LineEditEventFilter final : public QObject {
    Q_OBJECT
public:
    explicit LineEditEventFilter(QWidget* watched, fw::WidgetNode* owner);

    [[nodiscard]] auto Controller() const -> const fw::LineEditFsmController& { return _ctrl; }
    [[nodiscard]] auto Controller() -> fw::LineEditFsmController& { return _ctrl; }

    void NotifyValidationFail();
    void NotifyValidationPass();
    void NotifyTextChanged();
    void SetReadOnly(bool readOnly);

protected:
    auto eventFilter(QObject* obj, QEvent* event) -> bool override;

private:
    fw::LineEditFsmController _ctrl;
    fw::WidgetNode*           _owner;
};

} // namespace matcha::gui
