#pragma once

/**
 * @file ContainerNode.h
 * @brief UiNode providing layout composition for dialogs and panels.
 *
 * ContainerNode wraps a QWidget with a QLayout (VBox, HBox, Grid, Form, Stack).
 * When child UiNodes are added via AddNode(), their Widget() is automatically
 * inserted into the layout via the overridden AddNode().
 *
 * This enables fully tree-based UI composition without manual QLayout manipulation.
 * Mirrors WPF StackPanel/Grid, QML RowLayout/ColumnLayout, 3DEXPERIENCE CATDlgGridLayout.
 *
 * @see docs/05_Greenfield_Plan.md Phase 4 ContainerNode spec
 */

#include "Matcha/Theming/Token/TokenEnums.h"
#include "Matcha/Tree/UiNode.h"

#include <cstdint>

class QLayout;
class QWidget;

namespace matcha::fw {

/**
 * @brief Layout kind for ContainerNode.
 */
enum class LayoutKind : uint8_t {
    Vertical,    ///< QVBoxLayout
    Horizontal,  ///< QHBoxLayout
    Grid,        ///< QGridLayout
    Form,        ///< QFormLayout (label-input pairs)
    Stack,       ///< QStackedLayout (only one child visible at a time)
    Flow,        ///< FlowLayout (wrap children to next row)
    HSplitter,   ///< NyanSplitter (horizontal, resizable children)
    VSplitter,   ///< NyanSplitter (vertical, resizable children)
};

/**
 * @brief UiNode providing layout composition.
 *
 * Children's Widget() pointers are auto-inserted into the internal QLayout
 * when added via AddNode(). Removal via RemoveNode() detaches them.
 */
class MATCHA_EXPORT ContainerNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    /**
     * @brief Construct a ContainerNode with a new QWidget.
     * @param id Node identifier.
     * @param kind Layout kind (Vertical/Horizontal/Grid/Form/Stack).
     * @param parent Optional parent QWidget.
     */
    ContainerNode(std::string id, LayoutKind kind, UiNode* parentHint = nullptr);
    ~ContainerNode() override;

    /**
     * @brief Wrap an existing QWidget as a ContainerNode (non-owning).
     * @param id Node identifier.
     * @param widget Existing widget to wrap (must already have a layout).
     * @return Unique pointer to the new ContainerNode.
     */
    static auto Wrap(std::string id, QWidget* widget) -> std::unique_ptr<ContainerNode>;

    ContainerNode(const ContainerNode&) = delete;
    auto operator=(const ContainerNode&) -> ContainerNode& = delete;
    ContainerNode(ContainerNode&&) = delete;
    auto operator=(ContainerNode&&) -> ContainerNode& = delete;

    // -- Layout properties --

    /// @brief Get the layout kind.
    [[nodiscard]] auto Kind() const -> LayoutKind;

    /// @brief Set spacing between children (token-aware, density-scaled).
    void SetSpacing(SpaceToken token);

    /// @brief Set uniform margins on all four sides (token-aware, density-scaled).
    void SetMargins(SpaceToken token);

    /// @brief Set per-side margins (token-aware, density-scaled).
    void SetMargins(SpaceToken left, SpaceToken top,
                    SpaceToken right, SpaceToken bottom);

    /// @brief Set layout direction (LTR/RTL). RTL swaps left/right margins
    /// and reverses HBox child order via Qt layoutDirection.
    void SetDirection(TextDirection dir);

    /// @brief Get the current layout direction.
    [[nodiscard]] auto Direction() const -> TextDirection;

    /// @brief Set stretch factor for a child by index (VBox/HBox only).
    void SetStretch(int childIndex, int factor);

    // -- Grid-specific --

    /// @brief Place a child at a specific grid position (Grid layout only).
    void PlaceChild(UiNode* child, int row, int col, int rowSpan = 1, int colSpan = 1);

    // -- Stack-specific --

    /// @brief Set the visible child index (Stack layout only).
    void SetCurrentIndex(int index);

    /// @brief Get the visible child index (Stack layout only).
    [[nodiscard]] auto CurrentIndex() const -> int;

    // -- Size --

    /// @brief Set minimum size.
    void SetMinimumSize(int w, int h);

    /// @brief Set maximum size.
    void SetMaximumSize(int w, int h);

    /// @brief Set fixed size.
    void SetFixedSize(int w, int h);

    // -- Widget bridge --

    /// @brief Get the underlying QWidget.
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Tree overrides (widget insertion into layout) --
    auto AddNode(std::unique_ptr<UiNode> child) -> UiNode* override;
    auto RemoveNode(UiNode* child) -> std::unique_ptr<UiNode> override;

private:
    void ApplySpacing();
    void ApplyMargins();

    QWidget* _container;
    QLayout* _layout;
    LayoutKind _kind;
    bool _ownsWidget = true;

    SpaceToken _spacingToken = SpaceToken::sizeUnit;
    SpaceToken _marginLeft   = SpaceToken::sizeUnit;
    SpaceToken _marginTop    = SpaceToken::sizeUnit;
    SpaceToken _marginRight  = SpaceToken::sizeUnit;
    SpaceToken _marginBottom = SpaceToken::sizeUnit;
    TextDirection _direction   = TextDirection::LTR;
};

} // namespace matcha::fw
