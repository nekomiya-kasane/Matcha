#pragma once

/**
 * @file NyanBreadcrumb.h
 * @brief Theme-aware breadcrumb path navigation widget.
 *
 * A horizontal QWidget that displays clickable path segments separated by
 * a configurable separator string. Supports overflow ellipsis when the
 * path exceeds available width.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Segments: Foreground1 text, PrimaryHover on hover, PrimaryClick on press
 * - Separator: Foreground3 text (e.g. " > " or " / ")
 * - Overflow: leading "..." when segments exceed width
 * - Height: matches font line height + padding
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

#include <vector>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware breadcrumb path navigation.
 *
 * Displays a list of clickable string segments with a separator.
 * Emits `ItemClicked(int)` when a segment is clicked.
 */
class MATCHA_EXPORT NyanBreadcrumb : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a breadcrumb.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanBreadcrumb(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanBreadcrumb() override;

    NyanBreadcrumb(const NyanBreadcrumb&)            = delete;
    NyanBreadcrumb& operator=(const NyanBreadcrumb&) = delete;
    NyanBreadcrumb(NyanBreadcrumb&&)                 = delete;
    NyanBreadcrumb& operator=(NyanBreadcrumb&&)      = delete;

    /// @brief Set the path items.
    void SetItems(const std::vector<QString>& items);

    /// @brief Get the current items.
    [[nodiscard]] auto Items() const -> const std::vector<QString>&;

    /// @brief Set the separator string (default: " > ").
    void SetSeparator(const QString& separator);

    /// @brief Get the current separator.
    [[nodiscard]] auto Separator() const -> QString;

    /// @brief Size hint based on total text width.
    [[nodiscard]] auto sizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when a path segment is clicked.
    void ItemClicked(int index);

protected:
    /// @brief Custom paint: segments + separators + overflow ellipsis.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle segment click.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    /// @brief Cached segment layout info for hit testing and painting.
    struct SegmentLayout {
        int x;      ///< Left edge in widget coords
        int width;  ///< Pixel width
        int index;  ///< Index into _items
    };

    /// @brief Rebuild the layout cache from current items.
    void RebuildLayout();

    static constexpr int kHPadding    = 4;  ///< Horizontal padding
    static constexpr int kVPadding    = 4;  ///< Vertical padding
    static constexpr int kFixedHeight = 24; ///< Fixed height

    std::vector<QString>       _items;
    QString                    _separator = QStringLiteral(" > ");
    std::vector<SegmentLayout> _layout;
    int                        _hoveredIndex = -1;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
