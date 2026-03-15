#pragma once

/**
 * @file WhatsThisEventFilter.h
 * @brief Application-level Shift+F1 "What's This" mode event filter.
 *
 * Activates a question-mark cursor mode. Clicking any widget that has
 * WidgetNode::WhatsThis() text shows a popup bubble with the help text.
 * Press Escape or click outside to exit the mode.
 */

#include <Matcha/Core/Macros.h>

#include <QObject>

class QLabel;

namespace matcha::gui {

/**
 * @brief Intercepts Shift+F1 to toggle What's This mode.
 *
 * In What's This mode:
 * - Cursor changes to Qt::WhatsThisCursor
 * - Next mouse click on a WidgetNode shows its WhatsThis text in a popup
 * - Escape or click-outside exits the mode
 */
class MATCHA_EXPORT WhatsThisEventFilter : public QObject {
    Q_OBJECT

public:
    explicit WhatsThisEventFilter(QObject* parent = nullptr);
    ~WhatsThisEventFilter() override;

    [[nodiscard]] auto IsActive() const -> bool { return _active; }

    void Activate();
    void Deactivate();

protected:
    auto eventFilter(QObject* watched, QEvent* event) -> bool override;

private:
    void ShowBubble(QWidget* anchor, const QString& text);
    void HideBubble();

    bool _active = false;
    QLabel* _bubble = nullptr;
};

} // namespace matcha::gui
