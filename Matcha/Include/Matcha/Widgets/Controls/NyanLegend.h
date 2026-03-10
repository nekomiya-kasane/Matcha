#pragma once

/**
 * @file NyanLegend.h
 * @brief Theme-aware legend widget displaying color-label-value items.
 *
 * Renders a list of legend items, each with a color swatch, name label,
 * and value string. Supports horizontal and vertical layouts.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanLegend.h` (LegendInfoData struct, Add/Insert/Remove)
 * - Old struct: name, flag, value, color
 *
 * @par Visual specification
 * - Each item: 12px color square + 4px gap + name text + " " + flag + value
 * - Item height: 20px
 * - Text: Foreground1, small font
 * - Hover: PrimaryHover text
 * - Click: ItemClicked(index) signal
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

#include <vector>

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Data for a single legend item.
 */
struct MATCHA_EXPORT LegendItem {
    QString name;                        ///< Display name
    QString flag  = QStringLiteral("<"); ///< Comparison flag (e.g. "<", ">", "=")
    QString value = QStringLiteral("0"); ///< Display value
    QColor  color;                       ///< Swatch color
};

/**
 * @brief Theme-aware legend widget with color-label-value items.
 *
 * Displays a vertical list of legend items. Each item shows a small
 * color square, a name, and a flag+value string.
 */
class MATCHA_EXPORT NyanLegend : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a legend.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanLegend(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanLegend() override;

    NyanLegend(const NyanLegend&)            = delete;
    NyanLegend& operator=(const NyanLegend&) = delete;
    NyanLegend(NyanLegend&&)                 = delete;
    NyanLegend& operator=(NyanLegend&&)      = delete;

    /// @brief Set all legend items at once.
    void SetItems(const std::vector<LegendItem>& items);

    /// @brief Get the current items.
    [[nodiscard]] auto Items() const -> const std::vector<LegendItem>&;

    /// @brief Add a single item at the end. Returns the new index.
    auto AddItem(const LegendItem& item) -> int;

    /// @brief Insert an item at a specific index. Returns the actual index.
    auto InsertItem(int index, const LegendItem& item) -> int;

    /// @brief Modify the item at index in-place.
    void SetItem(int index, const LegendItem& item);

    /// @brief Remove the item at index.
    void RemoveItem(int index);

    /// @brief Remove all items.
    void ClearItems();

    /// @brief Number of items.
    [[nodiscard]] auto ItemCount() const -> int;

    /// @brief Enable or disable the default legend row (shown at bottom, index -1).
    void SetUseDefault(bool use);

    /// @brief Whether the default legend row is enabled.
    [[nodiscard]] auto UseDefault() const -> bool;

    /// @brief Enable or disable item selection on click.
    void SetSelectable(bool selectable);

    /// @brief Whether items are selectable.
    [[nodiscard]] auto IsSelectable() const -> bool;

    /// @brief Size hint based on item count.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Same as sizeHint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when an item is clicked. index=-1 for default item.
    void ItemClicked(int index);

protected:
    /// @brief Custom paint: item rows with color swatch + text.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle item click.
    void mouseReleaseEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    /// @brief Get the item index at a given y position, or -1.
    [[nodiscard]] auto ItemAtY(int y) const -> int;

    static constexpr int kItemHeight  = 20; ///< Height per item row
    static constexpr int kSwatchSize  = 12; ///< Color swatch square size
    static constexpr int kGap         = 4;  ///< Gap between swatch and text
    static constexpr int kHPadding    = 4;  ///< Horizontal padding

    std::vector<LegendItem> _items;
    bool _selectable  = true;
    bool _useDefault  = true;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
