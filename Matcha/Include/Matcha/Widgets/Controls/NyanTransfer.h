#pragma once

/**
 * @file NyanTransfer.h
 * @brief Theme-aware dual-list transfer selector.
 *
 * Left (available) + Right (selected) lists with transfer arrows.
 * Multi-select. Search filter per list. Used for batch entity selection,
 * output variable configuration.
 *
 * @see 05_Greenfield_Plan.md ss 3.3, widget #43.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

#include <string>
#include <vector>

class QListWidget;
class QPushButton;

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Theme-aware dual-list transfer selector.
 *
 * A11y role: List.
 */
class MATCHA_EXPORT NyanTransfer : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    explicit NyanTransfer(QWidget* parent = nullptr);
    ~NyanTransfer() override;

    NyanTransfer(const NyanTransfer&)            = delete;
    NyanTransfer& operator=(const NyanTransfer&) = delete;
    NyanTransfer(NyanTransfer&&)                 = delete;
    NyanTransfer& operator=(NyanTransfer&&)      = delete;

    /// @brief Set items in the source (left) list.
    void SetSourceItems(const std::vector<std::string>& items);

    /// @brief Set items in the target (right) list.
    void SetTargetItems(const std::vector<std::string>& items);

    /// @brief Get current source items.
    [[nodiscard]] auto SourceItems() const -> std::vector<std::string>;

    /// @brief Get current target items.
    [[nodiscard]] auto TargetItems() const -> std::vector<std::string>;

    /// @brief Move selected source items to target.
    void MoveSelectedToTarget();

    /// @brief Move selected target items back to source.
    void MoveSelectedToSource();

    /// @brief Move all source items to target.
    void MoveAllToTarget();

    /// @brief Move all target items back to source.
    void MoveAllToSource();

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the transfer state changes.
    void TransferChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void OnThemeChanged() override;

private:
    void SetupUi();

    QListWidget* _sourceList = nullptr;
    QListWidget* _targetList = nullptr;
    QPushButton* _moveRightBtn = nullptr;
    QPushButton* _moveLeftBtn = nullptr;
    QPushButton* _moveAllRightBtn = nullptr;
    QPushButton* _moveAllLeftBtn = nullptr;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
