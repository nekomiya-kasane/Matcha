#pragma once

/**
 * @file PopupNode.h
 * @brief Base class for popup-type UiNodes (dropdown, tooltip, floating panel).
 *
 * PopupNode encapsulates the boilerplate of transient popup windows:
 * - Qt window flag selection based on PopupBehavior
 * - Automatic positioning via PopupPositioner
 * - Escape-key and click-away close handling
 * - Lifecycle notifications (PopupOpened / PopupClosed)
 *
 * Subclasses override CreatePopupContent() and optionally PreferredSize()
 * to provide their concrete UI. The base class handles everything else.
 *
 * @note DialogNode and MenuNode are NOT derived from PopupNode —
 *       they have mature, behavior-specific implementations that predate this class.
 *       PopupNode targets *new* popup controls (ColorPicker dropdown, PopConfirm, etc.).
 *
 * @see PopupPositioner for the positioning engine.
 * @see WidgetNotification.h for PopupOpened / PopupClosed.
 */

#include "Matcha/Tree/UiNode.h"
#include "Matcha/Interaction/PopupPositioner.h"

class QWidget;
class QObject;

namespace matcha::fw {

/**
 * @brief Popup behavior determines Qt window flags and default close policy.
 *
 * | Behavior  | Qt Flag                           | click-away | Escape | Focus |
 * |-----------|-----------------------------------|:----------:|:------:|:-----:|
 * | Dropdown  | Qt::Popup | FramelessWindowHint   |     ✓      |   ✓    |   ✗   |
 * | Tooltip   | Qt::ToolTip | FramelessWindowHint |     ✓      |   ✗    |   ✗   |
 * | Floating  | Qt::Tool | FramelessWindowHint    |     ✗      |   ✓    |   ✓   |
 */
enum class PopupBehavior : uint8_t {
    Dropdown,
    Tooltip,
    Floating,
};

/**
 * @brief Base class for popup-type UiNodes.
 *
 * Usage:
 * @code
 *   class MyPickerPopup : public PopupNode {
 *   public:
 *       MyPickerPopup() : PopupNode("myPicker", NodeType::Popup, PopupBehavior::Dropdown) {}
 *   protected:
 *       auto CreatePopupContent(QWidget* parent) -> QWidget* override { ... }
 *       auto PreferredSize() -> Size override { return {280, 200}; }
 *   };
 *
 *   popup->Open(anchorNode, PopupPlacement::BottomStart);
 *   popup->Close();
 * @endcode
 */
class MATCHA_EXPORT PopupNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    PopupNode(std::string id, NodeType type, PopupBehavior behavior);
    ~PopupNode() override;

    PopupNode(const PopupNode&) = delete;
    auto operator=(const PopupNode&) -> PopupNode& = delete;
    PopupNode(PopupNode&&) = delete;
    auto operator=(PopupNode&&) -> PopupNode& = delete;

    // -- Open / Close --

    /// @brief Open the popup anchored to a UiNode.
    /// @param anchor The anchor node whose Widget() geometry is used for positioning.
    /// @param placement Desired placement relative to the anchor.
    void Open(UiNode* anchor, PopupPlacement placement = PopupPlacement::BottomStart);

    /// @brief Open the popup at a specific screen position (no anchor).
    void OpenAtPoint(Point screenPos);

    /// @brief Close the popup.
    void Close();

    /// @brief Check if the popup is currently visible.
    [[nodiscard]] auto IsOpen() const -> bool;

    /// @brief Get the popup behavior.
    [[nodiscard]] auto Behavior() const -> PopupBehavior;

    /// @brief Get the anchor node (set by Open(), nullptr if opened via OpenAtPoint).
    [[nodiscard]] auto Anchor() const -> UiNode*;

    // -- Configuration --

    /// @brief Enable/disable Escape key closing (default depends on behavior).
    void SetCloseOnEscape(bool v);

    /// @brief Enable/disable auto-positioning via PopupPositioner (default: true).
    void SetAutoPosition(bool v);

    /// @brief Set additional offset from the computed position.
    void SetOffset(Point offset);

    /// @brief Set minimum popup height (used by PopupPositioner flip logic).
    void SetMinHeight(int px);

    // -- Widget bridge --

    [[nodiscard]] auto Widget() -> QWidget* override;

protected:
    /// @brief Subclass factory: create the popup's content widget.
    /// @param parent The popup container widget (use as QWidget parent).
    /// @return The content widget (owned by parent via Qt parent-child).
    virtual auto CreatePopupContent(QWidget* parent) -> QWidget* = 0;

    /// @brief Subclass hint: preferred popup size. Override for custom sizing.
    virtual auto PreferredSize() -> Size { return {200, 150}; }

    /// @brief Called after the popup is shown. Override for post-show logic.
    virtual void OnOpened() {}

    /// @brief Called after the popup is hidden. Override for post-hide logic.
    virtual void OnClosed() {}

    /// @brief Access the internal popup QWidget (for subclass use).
    auto PopupWidget() -> QWidget*;

private:
    void EnsurePopupWidget();
    void ApplyPosition(UiNode* anchor, PopupPlacement placement);

    class PopupEventFilter;

    PopupBehavior   _behavior;
    QWidget*        _popupWidget    = nullptr;
    PopupEventFilter* _eventFilter  = nullptr;
    UiNode*         _anchor         = nullptr;
    PopupPlacement  _placement      = PopupPlacement::BottomStart;
    Point           _offset         = {0, 0};
    int             _minHeight      = 120;
    bool            _closeOnEscape  = true;
    bool            _autoPosition   = true;
    bool            _isOpen         = false;
};

} // namespace matcha::fw
