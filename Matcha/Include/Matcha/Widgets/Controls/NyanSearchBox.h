#pragma once

/**
 * @file NyanSearchBox.h
 * @brief Theme-aware search input with icon, clear button, and history.
 *
 * A composite QWidget containing a NyanLineEdit for text input, a leading
 * search icon, a trailing clear button, and optional history dropdown.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification (from specs/details/24-search.md)
 * - Height: 24px
 * - Leading search icon (magnifying glass)
 * - Trailing clear button (appears when text is non-empty)
 * - Placeholder text
 * - Optional history dropdown on focus
 * - Full-round variant for navigation bar use
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

#include <vector>

namespace matcha::gui {

class InteractionEventFilter;
class NyanLineEdit;

/**
 * @brief Search trigger mode.
 */
enum class SearchMode : uint8_t {
    Instant, ///< Filter-as-you-type (emits SearchChanged on every keystroke)
    OnEnter, ///< Search on Enter key only (emits SearchSubmitted)

    Count_
};

/**
 * @brief Theme-aware search input with icon, clear button, and history.
 *
 * Composes NyanLineEdit internally. Leading search icon, trailing clear
 * button that appears when text is non-empty. Optional history dropdown.
 */
class MATCHA_EXPORT NyanSearchBox : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a search box.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanSearchBox(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanSearchBox() override;

    NyanSearchBox(const NyanSearchBox&)            = delete;
    NyanSearchBox& operator=(const NyanSearchBox&) = delete;
    NyanSearchBox(NyanSearchBox&&)                 = delete;
    NyanSearchBox& operator=(NyanSearchBox&&)      = delete;

    /// @brief Set placeholder text.
    void SetPlaceholder(const QString& text);

    /// @brief Get the current placeholder text.
    [[nodiscard]] auto Placeholder() const -> QString;

    /// @brief Get the current search text.
    [[nodiscard]] auto Text() const -> QString;

    /// @brief Set the search text programmatically.
    void SetText(const QString& text);

    /// @brief Set the search mode.
    void SetSearchMode(SearchMode mode);

    /// @brief Get the current search mode.
    [[nodiscard]] auto GetSearchMode() const -> SearchMode;

    /// @brief Set history items for the dropdown.
    void SetHistory(const std::vector<QString>& items);

    /// @brief Get the current history items.
    [[nodiscard]] auto History() const -> const std::vector<QString>&;

    /// @brief Clear the search text.
    void Clear();

    /// @brief Size hint: width from content, height 24px.
    [[nodiscard]] auto sizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when text changes (Instant mode: every keystroke).
    void SearchChanged(const QString& text);

    /// @brief Emitted when search is submitted (Enter key or Instant mode).
    void SearchSubmitted(const QString& text);

protected:
    /// @brief Custom paint: search icon, clear button, border.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    /// @brief Compute the clear button hit rect.
    [[nodiscard]] auto ClearButtonRect() const -> QRect;

    static constexpr int kFixedHeight = 24;
    static constexpr int kIconSize    = 16;
    static constexpr int kIconGap     = 4;
    static constexpr int kHPadding    = 6;

    NyanLineEdit*        _lineEdit = nullptr;
    SearchMode           _searchMode = SearchMode::Instant;
    std::vector<QString> _history;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
