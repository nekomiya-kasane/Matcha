#pragma once

/**
 * @file NyanCollapsibleSection.h
 * @brief Borderless collapsible content group with title row and expand/collapse arrow.
 *
 * Custom-painted QWidget with title row containing expand/collapse arrow.
 * Animated height transition using AnimationToken. No border (vs GroupBox).
 * Used in property panels, settings dialogs.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Title row: 24px height, Foreground1 text, FontRole::Body
 * - Arrow: 12x12 chevron, Foreground3, rotates 90° when expanded
 * - Content: no border, Background1 fill
 * - Collapse: animated height transition using AnimationToken::Normal
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken, AnimationToken.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Borderless collapsible content group with title row and expand/collapse arrow.
 *
 * A11y role: Group.
 */
class MATCHA_EXPORT NyanCollapsibleSection : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a collapsible section.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanCollapsibleSection(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanCollapsibleSection() override;

    NyanCollapsibleSection(const NyanCollapsibleSection&)            = delete;
    NyanCollapsibleSection& operator=(const NyanCollapsibleSection&) = delete;
    NyanCollapsibleSection(NyanCollapsibleSection&&)                 = delete;
    NyanCollapsibleSection& operator=(NyanCollapsibleSection&&)      = delete;

    /// @brief Set the title text.
    void SetTitle(const QString& title);

    /// @brief Get the title text.
    [[nodiscard]] auto Title() const -> QString;

    /// @brief Set the expanded state.
    void SetExpanded(bool expanded);

    /// @brief Whether the section is expanded.
    [[nodiscard]] auto IsExpanded() const -> bool;

    /// @brief Set the content widget. Takes ownership.
    void SetContent(QWidget* content);

    /// @brief Get the content widget.
    [[nodiscard]] auto Content() const -> QWidget*;

    /// @brief Size hint: width from content, height = title + content (if expanded).
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Minimum size: title height only.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when expanded state changes.
    void ExpandToggled(bool expanded);

protected:
    /// @brief Custom paint: title row with arrow.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle mouse press on title to toggle expand.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void AnimateExpand(bool expanded);
    void UpdateContentVisibility();

    static constexpr int kTitleHeight = 24;
    static constexpr int kArrowSize   = 12;
    static constexpr int kHPadding    = 8;

    QString  _title;
    bool     _expanded = true;
    QWidget* _content  = nullptr;
    int      _contentHeight = 0;
    qreal    _arrowRotation = 90.0;  ///< Arrow rotation: 0=collapsed (right), 90=expanded (down)
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
