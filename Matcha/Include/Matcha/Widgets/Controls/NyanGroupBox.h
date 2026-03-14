#pragma once

/**
 * @file NyanGroupBox.h
 * @brief Theme-aware group box with animated collapse and checkable title.
 *
 * Inherits QGroupBox for Qt group box semantics and ThemeAware for design
 * token integration. Supports animated collapse and checkable title.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Title: Foreground1, FontRole::Body
 * - Border: Border2, 1px, rounded with RadiusToken::Default
 * - Background: Background1
 * - Collapse: animated height transition using AnimationToken::Normal
 * - Checkable: optional checkbox in title area
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken, AnimationToken.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QGroupBox>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware group box with animated collapse and checkable title.
 *
 * Provides collapsible content area with optional checkbox in title.
 * A11y role: Grouping.
 */
class MATCHA_EXPORT NyanGroupBox : public QGroupBox, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a group box.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanGroupBox(QWidget* parent = nullptr);

    /**
     * @brief Construct a group box with title.
     * @param theme Theme service reference.
     * @param title Group box title text.
     * @param parent Optional parent widget.
     */
    NyanGroupBox(const QString& title, QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanGroupBox() override;

    NyanGroupBox(const NyanGroupBox&)            = delete;
    NyanGroupBox& operator=(const NyanGroupBox&) = delete;
    NyanGroupBox(NyanGroupBox&&)                 = delete;
    NyanGroupBox& operator=(NyanGroupBox&&)      = delete;

    /// @brief Set whether the group box is collapsible.
    void SetCollapsible(bool collapsible);

    /// @brief Whether the group box is collapsible.
    [[nodiscard]] auto IsCollapsible() const -> bool;

    /// @brief Set the collapsed state (only effective if collapsible).
    void SetCollapsed(bool collapsed);

    /// @brief Whether the group box is currently collapsed.
    [[nodiscard]] auto IsCollapsed() const -> bool;

Q_SIGNALS:
    /// @brief Emitted when collapsed state changes.
    void CollapsedChanged(bool collapsed);

protected:
    /// @brief Handle mouse press on title to toggle collapse.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void ApplyStyle();
    void AnimateCollapse(bool collapsed);

    bool _collapsible = false;
    bool _collapsed   = false;
    int  _expandedHeight = 0;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
