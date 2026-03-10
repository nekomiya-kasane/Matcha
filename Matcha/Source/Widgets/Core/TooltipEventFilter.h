#pragma once

/**
 * @file TooltipEventFilter.h
 * @brief Application-level event filter that bridges WidgetNode::TooltipSpec
 *        to the shared NyanRichTooltip instance.
 *
 * Install on QApplication to automatically show rich tooltips for any
 * WidgetNode that has a non-empty TooltipSpec. Replaces plain QToolTip.
 */

#include <Matcha/Foundation/Macros.h>

#include <QObject>

namespace matcha::gui {

class NyanRichTooltip;

/**
 * @brief Intercepts Enter/Leave/ToolTip events and shows NyanRichTooltip.
 *
 * On hover-enter: if the QWidget has a WidgetNode with a non-empty TooltipSpec,
 * populate the shared tooltip and start its timer sequence.
 * On hover-leave: hide the tooltip.
 * On ToolTip event: suppress Qt's default tooltip, we handle it ourselves.
 */
class MATCHA_EXPORT TooltipEventFilter : public QObject {
    Q_OBJECT

public:
    explicit TooltipEventFilter(NyanRichTooltip* tooltip, QObject* parent = nullptr);

protected:
    auto eventFilter(QObject* watched, QEvent* event) -> bool override;

private:
    NyanRichTooltip* _tooltip;
    QWidget* _currentTrigger = nullptr;
};

} // namespace matcha::gui
