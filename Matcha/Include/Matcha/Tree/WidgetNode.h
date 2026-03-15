#pragma once

/**
 * @file WidgetNode.h
 * @brief Abstract base for typed atomic widget nodes (Scheme D).
 *
 * WidgetNode sits between UiNode and concrete typed nodes (LineEditNode,
 * ComboBoxNode, etc.). It provides common QWidget property forwarding
 * so business layer never touches QWidget directly.
 *
 * The underlying QWidget is created lazily via the pure-virtual
 * CreateWidget() factory hook when Widget() is first called.
 */

#include "Matcha/Tree/A11yRole.h"
#include "Matcha/Theming/Token/TokenEnums.h"
#include "Matcha/Tree/TooltipSpec.h"
#include "Matcha/Tree/UiNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include <string>
#include <string_view>

class QWidget;

namespace matcha::fw {

/**
 * @brief Abstract intermediate class for typed atomic widget nodes.
 *
 * Subclasses implement CreateWidget() to instantiate their specific
 * NyanXxx widget. Common API (SetEnabled, SetVisible, etc.) forwards
 * to the underlying QWidget.
 */
class MATCHA_EXPORT WidgetNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification {
        using VisibilityChanged = matcha::fw::VisibilityChanged;
        using EnabledChanged    = matcha::fw::EnabledChanged;
        using FocusChanged      = matcha::fw::FocusChanged;
        using DragEntered       = matcha::fw::DragEntered;
        using DragMoved         = matcha::fw::DragMoved;
        using DragLeft          = matcha::fw::DragLeft;
        using Dropped           = matcha::fw::Dropped;
    };

    WidgetNode(std::string id, NodeType type);
    ~WidgetNode() override;

    // -- Common QWidget property forwarding --

    void SetEnabled(bool enabled);
    [[nodiscard]] auto IsEnabled() const -> bool;

    void SetVisible(bool visible);
    [[nodiscard]] auto IsVisible() const -> bool;

    void SetToolTip(std::string_view tip);

    void SetMinimumSize(int w, int h);
    void SetMaximumSize(int w, int h);
    void SetFixedSize(int w, int h);

    void SetFocus();
    [[nodiscard]] auto HasFocus() const -> bool;

    // -- Focus chain properties (S4) --

    void SetFocusable(bool focusable);
    [[nodiscard]] auto IsFocusable() const -> bool;

    void SetTabIndex(int index);
    [[nodiscard]] auto TabIndex() const -> int;

    // -- Accessibility (S5) --

    void SetA11yRole(A11yRole role);
    [[nodiscard]] auto GetA11yRole() const -> A11yRole;

    void SetAccessibleName(std::string name);
    [[nodiscard]] auto AccessibleName() const -> const std::string&;

    // -- Tooltip (S9) --

    void SetTooltip(TooltipSpec spec);
    [[nodiscard]] auto Tooltip() const -> const TooltipSpec&;
    [[nodiscard]] auto HasTooltip() const -> bool;

    // -- Help system (S13) --

    void SetStatusHint(std::string hint);
    [[nodiscard]] auto StatusHint() const -> const std::string&;

    void SetWhatsThis(std::string text);
    [[nodiscard]] auto WhatsThis() const -> const std::string&;

    void SetHelpId(std::string id);
    [[nodiscard]] auto HelpId() const -> const std::string&;

    /// @brief Enable drag & drop on the underlying widget.
    /// When enabled, DnD QEvents are intercepted and converted to
    /// DragEntered / DragMoved / DragLeft / Dropped Notifications.
    void SetAcceptDrops(bool enabled);

    /// @brief Enable context menu on the underlying widget.
    /// When enabled, right-click sends ContextMenuRequest notification
    /// that bubbles up the command tree for ancestor contribution.
    void SetContextMenuEnabled(bool enabled);

    // -- Icon properties (fw-layer, asset:// URI) --

    /// @brief Set the icon for this widget by asset URI.
    /// @param iconId Icon URI (e.g. "asset://matcha/icons/save"). Empty = no icon.
    void SetIcon(std::string_view iconId);

    /// @brief Get the current icon URI.
    [[nodiscard]] auto Icon() const -> const IconId&;

    /// @brief Set the icon size for this widget.
    void SetIconSize(fw::IconSize size);

    /// @brief Get the current icon size.
    [[nodiscard]] auto GetIconSize() const -> fw::IconSize;

    // -- Opacity --

    /// @brief Set widget opacity (0.0 = transparent, 1.0 = opaque).
    /// Internally manages QGraphicsOpacityEffect so callers never need Qt headers.
    void SetOpacity(double opacity);

    /// @brief Get current widget opacity (1.0 if no effect installed).
    [[nodiscard]] auto Opacity() const -> double;

    // -- Animation (RFC-08) --

    /// @brief Animate a property from one value to another.
    /// @param property Identifies which property to animate.
    /// @param from     Start value (Qt-free).
    /// @param to       End value (Qt-free).
    /// @param duration Animation speed token.
    /// @param easing   Easing curve token.
    /// @return Handle for the running animation (Invalid if service unavailable).
    auto AnimateProperty(AnimationPropertyId property,
                         AnimatableValue from,
                         AnimatableValue to,
                         AnimationToken duration = AnimationToken::Normal,
                         EasingToken easing = EasingToken::OutCubic) -> TransitionHandle;

    /// @brief Animate a property with custom spring dynamics.
    /// @param property Property to animate.
    /// @param from     Start value.
    /// @param to       End value.
    /// @param spring   Spring dynamics parameters (mass, stiffness, damping).
    /// @return Handle for the running animation (Invalid if service unavailable).
    auto AnimateSpring(AnimationPropertyId property,
                       AnimatableValue from,
                       AnimatableValue to,
                       SpringSpec spring) -> TransitionHandle;

    /// @brief Cancel a running animation by handle.
    void CancelAnimation(TransitionHandle handle);

    /// @brief Check if a property is currently being animated.
    [[nodiscard]] auto IsAnimating(AnimationPropertyId property) -> bool;

    /// @brief Return the underlying QWidget (created lazily).
    [[nodiscard]] auto Widget() -> QWidget* override;

    /// @brief Return an opaque native handle to the underlying widget.
    /// Intended for C ABI / cross-language bindings where QWidget* cannot
    /// be expressed. Returns the same pointer as Widget() cast to void*.
    [[nodiscard]] auto NativeHandle() -> void*;

    /// @brief Retrieve the WidgetNode* associated with a QWidget, or nullptr.
    /// Uses the "matcha_widgetnode" dynamic property set during EnsureWidget().
    [[nodiscard]] static auto FromWidget(QWidget* widget) -> WidgetNode*;

protected:
    /// @brief Subclass factory: create the concrete NyanXxx widget.
    /// @param parent Qt parent widget for lifetime management.
    /// @return The created widget. Must not be nullptr.
    virtual auto CreateWidget(QWidget* parent) -> QWidget* = 0;

    /// @brief Called after SetIcon() or SetIconSize() when _widget exists.
    /// Subclasses override to resolve the icon via IThemeService and apply
    /// the resulting QPixmap/QIcon to their concrete widget.
    virtual void OnIconChanged();

    /// @brief Ensure the widget has been created. Call before forwarding.
    void EnsureWidget();

    QWidget* _widget = nullptr;

    IconId        _iconId;
    fw::IconSize  _iconSize  = fw::IconSize::Sm;
    bool          _focusable = false;
    int           _tabIndex  = -1; ///< -1 = tree order (default)
    TooltipSpec   _tooltip;
    std::string   _statusHint;
    std::string   _whatsThis;
    std::string   _helpId;
    A11yRole      _a11yRole = A11yRole::None;
    std::string   _accessibleName;

private:
    void InstallFocusFilter();

    class FocusTabEventFilter;
    FocusTabEventFilter* _focusFilter = nullptr;
    class DndEventFilter;
    DndEventFilter* _dndFilter = nullptr;
    class ContextMenuEventFilter;
    ContextMenuEventFilter* _ctxMenuFilter = nullptr;
};

} // namespace matcha::fw
