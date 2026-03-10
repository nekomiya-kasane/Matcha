#pragma once

/**
 * @file NyanMenuCheckItem.h
 * @brief Toggle menu item with check indicator.
 *
 * NyanMenuCheckItem extends NyanMenuItem with:
 * - Check indicator (checkmark icon)
 * - Toggle behavior
 *
 * @par Visual specification
 * - Same as NyanMenuItem but with checkmark in icon area when checked
 *
 * @see NyanMenuItem for base functionality.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Widgets/Menu/NyanMenuItem.h>

namespace matcha::gui {

/**
 * @brief Toggle menu item with check indicator.
 *
 * A11y role: MenuItemCheckbox.
 */
class MATCHA_EXPORT NyanMenuCheckItem : public NyanMenuItem {
    Q_OBJECT

public:
    /**
     * @brief Construct a check menu item.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanMenuCheckItem(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanMenuCheckItem() override;

    NyanMenuCheckItem(const NyanMenuCheckItem&)            = delete;
    NyanMenuCheckItem& operator=(const NyanMenuCheckItem&) = delete;
    NyanMenuCheckItem(NyanMenuCheckItem&&)                 = delete;
    NyanMenuCheckItem& operator=(NyanMenuCheckItem&&)      = delete;

    // -- Check State --

    /// @brief Set the checked state.
    void SetChecked(bool checked);

    /// @brief Get the checked state.
    [[nodiscard]] auto IsChecked() const -> bool;

    /// @brief Toggle the checked state.
    void Toggle();

Q_SIGNALS:
    /// @brief Emitted when the checked state changes.
    void Toggled(bool checked);

protected:
    /// @brief Draw content with check indicator.
    void DrawContent(QPainter& painter, const QRect& rect, bool hovered, bool pressed) const override;

private:
    void DrawCheckmark(QPainter& painter, const QRect& rect) const;

    bool _checked = false;
};

} // namespace matcha::gui
