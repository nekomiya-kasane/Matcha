#pragma once

/**
 * @file NyanActionToolbar.h
 * @brief Separator-delimited button group for ActionBar.
 *
 * NyanActionToolbar provides:
 * - Button group with separators
 * - Uses NyanToolButton for buttons
 * - Horizontal layout with configurable spacing
 *
 * @see NyanActionTab for tab container.
 * @see NyanActionBar for the main ribbon container.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

#include <functional>
#include <vector>

class QBoxLayout;

namespace matcha::gui {

class NyanToolButton;

/**
 * @brief Button info for toolbar.
 */
struct ActionButtonInfo {
    QString id;
    QString text;
    QIcon   icon;
    QString tooltip;
    bool    checkable = false;
};

/**
 * @brief Separator-delimited button group for ActionBar.
 */
class MATCHA_EXPORT NyanActionToolbar : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct an action toolbar.
     * @param theme Theme service reference.
     * @param parent Optional parent widget.
     */
    explicit NyanActionToolbar(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanActionToolbar() override;

    NyanActionToolbar(const NyanActionToolbar&)            = delete;
    NyanActionToolbar& operator=(const NyanActionToolbar&) = delete;
    NyanActionToolbar(NyanActionToolbar&&)                 = delete;
    NyanActionToolbar& operator=(NyanActionToolbar&&)      = delete;

    // -- Buttons --

    /// @brief Add a button to the toolbar.
    /// @param info Button information.
    /// @return Button index, or -1 on failure.
    [[nodiscard]] auto AddButton(const ActionButtonInfo& info) -> int;

    /// @brief Add a separator.
    /// @return Separator index.
    [[nodiscard]] auto AddSeparator() -> int;

    /// @brief Remove item at index.
    void RemoveItem(int index);

    /// @brief Get button count (excluding separators).
    [[nodiscard]] auto ButtonCount() const -> int;

    /// @brief Get total item count (buttons + separators).
    [[nodiscard]] auto ItemCount() const -> int;

    /// @brief Get button by ID.
    [[nodiscard]] auto Button(const QString& id) -> NyanToolButton*;

    /// @brief Get button by index.
    [[nodiscard]] auto ButtonAt(int index) -> NyanToolButton*;

    /// @brief Check if item at index is a separator.
    [[nodiscard]] auto IsSeparator(int index) const -> bool;

    // -- State --

    /// @brief Set button checked state.
    void SetButtonChecked(const QString& id, bool checked);

    /// @brief Get button checked state.
    [[nodiscard]] auto IsButtonChecked(const QString& id) const -> bool;

    /// @brief Set button enabled state.
    void SetButtonEnabled(const QString& id, bool enabled);

    /// @brief Get button enabled state.
    [[nodiscard]] auto IsButtonEnabled(const QString& id) const -> bool;

    // -- Orientation --

    /// @brief Set layout orientation (Horizontal or Vertical).
    void SetOrientation(Qt::Orientation orientation);

    /// @brief Get current orientation.
    [[nodiscard]] auto Orientation() const -> Qt::Orientation;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when a button is clicked.
    void ButtonClicked(const QString& buttonId, bool checked);

protected:
    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void UpdateButtonStyles();

    static constexpr int kButtonSize    = 32;
    static constexpr int kSeparatorSize = 1;
    static constexpr int kSpacing       = 2;

    struct Item {
        bool isSeparator = false;
        QString id;
        NyanToolButton* button = nullptr;
        QWidget* separator = nullptr;
    };

    void RebuildLayout();

    QBoxLayout*       _layout      = nullptr;
    Qt::Orientation   _orientation = Qt::Horizontal;
    std::vector<Item> _items;
};

} // namespace matcha::gui
