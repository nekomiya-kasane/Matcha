#pragma once

/**
 * @file InteractionEventFilter.h
 * @brief Reusable QObject event filter that drives InteractionFSM (D4).
 *
 * Install on any QWidget to automatically track hover/press/focus state
 * via the InteractionFSM and emit InteractionStateChanged via a WidgetNode.
 *
 * Usage in widget constructor:
 * @code
 * _interactionFilter = new InteractionEventFilter(this, ownerNode);
 * @endcode
 */

#include <Matcha/Foundation/Macros.h>
#include "Matcha/UiNodes/Core/InteractionFSM.h"

#include <QObject>

namespace matcha::fw {
class WidgetNode;
} // namespace matcha::fw

namespace matcha::gui {

/**
 * @brief Event filter that drives InteractionFSM for a QWidget.
 *
 * Intercepts QEvent::Enter, Leave, MouseButtonPress, MouseButtonRelease,
 * FocusIn, FocusOut, EnabledChange and feeds them to InteractionFSM.
 * On state change, emits InteractionStateChanged notification via the
 * owning WidgetNode.
 *
 * Ownership: parented to the watched widget (auto-deleted).
 */
class MATCHA_EXPORT InteractionEventFilter final : public QObject {
    Q_OBJECT
public:
    /**
     * @param watched  Widget to install the filter on.
     * @param owner    WidgetNode that owns this widget (for notification dispatch).
     */
    explicit InteractionEventFilter(QWidget* watched, fw::WidgetNode* owner);

    [[nodiscard]] auto Fsm() const -> const fw::InteractionFSM& { return _fsm; }

protected:
    auto eventFilter(QObject* obj, QEvent* event) -> bool override;

private:
    fw::InteractionFSM _fsm;
    fw::WidgetNode* _owner;
};

} // namespace matcha::gui
