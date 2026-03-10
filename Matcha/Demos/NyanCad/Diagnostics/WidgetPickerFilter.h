#pragma once

/**
 * @file WidgetPickerFilter.h
 * @brief QApplication-level event filter for widget picking (Chrome Inspect style).
 *
 * Non-intrusive: installs on QApplication, never modifies any widget.
 * On hover: emits Hovered(QWidget*). On click: emits Picked(QWidget*) and deactivates.
 */

#include <QObject>

class QWidget;

namespace nyancad {

class LayoutBoundsOverlay;

/**
 * @brief Application-level event filter for picking widgets by mouse hover + click.
 */
class WidgetPickerFilter : public QObject {
    Q_OBJECT

public:
    explicit WidgetPickerFilter(LayoutBoundsOverlay* overlay, QObject* parent = nullptr);
    ~WidgetPickerFilter() override;

    /// @brief Activate pick mode (installs filter on QApplication).
    void Activate();

    /// @brief Deactivate pick mode (removes filter from QApplication).
    void Deactivate();

    /// @brief Check if picker is active.
    [[nodiscard]] auto IsActive() const -> bool;

Q_SIGNALS:
    /// @brief Emitted when a widget is hovered during pick mode.
    void Hovered(QWidget* widget);

    /// @brief Emitted when a widget is clicked (selected) during pick mode.
    void Picked(QWidget* widget);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    LayoutBoundsOverlay* _overlay = nullptr;
    bool _active = false;
};

} // namespace nyancad
