/**
 * @file TooltipEventFilter.cpp
 * @brief Application-level tooltip event filter implementation.
 */

#include "TooltipEventFilter.h"

#include <Matcha/Tree/WidgetNode.h>
#include <Matcha/Widgets/Controls/NyanRichTooltip.h>
#include <Matcha/Theming/IThemeService.h>

#include <QEvent>
#include <QHelpEvent>
#include <QString>
#include <QWidget>

namespace matcha::gui {

TooltipEventFilter::TooltipEventFilter(NyanRichTooltip* tooltip, QObject* parent)
    : QObject(parent)
    , _tooltip(tooltip)
{
}

auto TooltipEventFilter::eventFilter(QObject* watched, QEvent* event) -> bool
{
    auto* widget = qobject_cast<QWidget*>(watched);
    if (widget == nullptr || _tooltip == nullptr) {
        return false;
    }

    switch (event->type()) {
    case QEvent::Enter: {
        // Look up the WidgetNode for this widget
        auto* node = fw::WidgetNode::FromWidget(widget);
        if (node != nullptr && node->HasTooltip()) {
            const auto& spec = node->Tooltip();

            _tooltip->SetTitle(QString::fromStdString(spec.title));
            _tooltip->SetShortcut(QString::fromStdString(spec.shortcut));
            _tooltip->SetDescription(QString::fromStdString(spec.description));
            _tooltip->SetTier1Delay(spec.tier1DelayMs);
            _tooltip->SetTier2Delay(spec.tier2DelayMs);

            // Resolve icon if present
            if (!spec.iconId.empty() && HasThemeService()) {
                const QColor fg = GetThemeService().Color(ColorToken::colorText);
                const QPixmap pm = GetThemeService().ResolveIcon(
                    spec.iconId, fw::IconToken::iconSizeSM, fg);
                _tooltip->SetIcon(pm);
            } else {
                _tooltip->SetIcon(QPixmap());
            }

            _currentTrigger = widget;
            _tooltip->ShowForWidget(widget);
        }
        break;
    }

    case QEvent::Leave:
        if (widget == _currentTrigger) {
            _tooltip->Hide();
            _currentTrigger = nullptr;
        }
        break;

    case QEvent::ToolTip: {
        // Suppress Qt's default QToolTip if we have a rich tooltip spec
        auto* node = fw::WidgetNode::FromWidget(widget);
        if (node != nullptr && node->HasTooltip()) {
            event->accept();
            return true; // consumed — we handle tooltips ourselves
        }
        break;
    }

    default:
        break;
    }

    return false;
}

} // namespace matcha::gui
