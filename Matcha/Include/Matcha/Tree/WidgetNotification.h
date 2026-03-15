#pragma once

/**
 * @file WidgetNotification.h
 * @brief Notification types for UiNode widget layer (Layer 1).
 *
 * Contains all notifications emitted by WidgetNode and its subclasses:
 *  - Common (WidgetNode base): Visibility, Enabled, Focus, InteractionState
 *  - Drag & Drop: DragAction, DragEntered, DragMoved, DragLeft, Dropped
 *  - Container nodes: TabSwitched, CollapsedChanged, ButtonClicked, CloseRequested, DialogClosed
 *  - Widget controls: Activated, Clicked, Toggled, TextChanged, ...
 *  - Context menu: ContextMenuRequest, ContextMenuItemActivated
 *  - Cross-cutting: LocaleChanged, HelpRequested, Animation lifecycle
 *
 * Higher-layer notifications live in separate headers:
 *  - TabNotification.h       (TabBarNode tab-page drag/reorder)
 *  - DocumentNotification.h  (DocumentArea, DocumentManager, ViewportGroup)
 *  - WorkbenchNotification.h (Workshop/Workbench activation, CommandInvoked)
 */

#include "Matcha/Event/Notification.h"
#include "Matcha/Theming/Token/TokenEnums.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw { class UiNode; }

namespace matcha::fw {

// =========================================================================== //
//  Common notifications (WidgetNode base)
// =========================================================================== //

class MATCHA_EXPORT VisibilityChanged final : public Notification {
public:
    explicit VisibilityChanged(bool visible = false) : _visible(visible) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VisibilityChanged"; }
    [[nodiscard]] auto IsVisible() const -> bool { return _visible; }
private:
    bool _visible;
};

class MATCHA_EXPORT EnabledChanged final : public Notification {
public:
    explicit EnabledChanged(bool enabled = true) : _enabled(enabled) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "EnabledChanged"; }
    [[nodiscard]] auto IsEnabled() const -> bool { return _enabled; }
private:
    bool _enabled;
};

class MATCHA_EXPORT FocusChanged final : public Notification {
public:
    explicit FocusChanged(bool hasFocus = false) : _hasFocus(hasFocus) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "FocusChanged"; }
    [[nodiscard]] auto HasFocus() const -> bool { return _hasFocus; }
private:
    bool _hasFocus;
};

class MATCHA_EXPORT InteractionStateChanged final : public Notification {
public:
    explicit InteractionStateChanged(InteractionState oldState = InteractionState::Normal,
                                     InteractionState newState = InteractionState::Normal)
        : _oldState(oldState), _newState(newState) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "InteractionStateChanged"; }
    [[nodiscard]] auto OldState() const -> InteractionState { return _oldState; }
    [[nodiscard]] auto NewState() const -> InteractionState { return _newState; }
private:
    InteractionState _oldState;
    InteractionState _newState;
};

// =========================================================================== //
//  Drag & Drop notifications (WidgetNode base, any UiNode)
// =========================================================================== //

/** @brief Qt-free drag action enum, maps 1:1 with Qt::DropAction. */
enum class DragAction : uint8_t {
    Copy   = 0,
    Move   = 1,
    Link   = 2,
    Ignore = 3
};

class MATCHA_EXPORT DragEntered final : public Notification {
public:
    explicit DragEntered(std::vector<std::string> mimeTypes = {},
                         DragAction proposedAction = DragAction::Copy)
        : _mimeTypes(std::move(mimeTypes)), _proposedAction(proposedAction) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DragEntered"; }
    [[nodiscard]] auto MimeTypes() const -> const std::vector<std::string>& { return _mimeTypes; }
    [[nodiscard]] auto ProposedAction() const -> DragAction { return _proposedAction; }
private:
    std::vector<std::string> _mimeTypes;
    DragAction _proposedAction;
};

class MATCHA_EXPORT DragMoved final : public Notification {
public:
    explicit DragMoved(int x = 0, int y = 0,
                       std::vector<std::string> mimeTypes = {},
                       DragAction proposedAction = DragAction::Copy)
        : _x(x), _y(y), _mimeTypes(std::move(mimeTypes)), _proposedAction(proposedAction) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DragMoved"; }
    [[nodiscard]] auto X() const -> int { return _x; }
    [[nodiscard]] auto Y() const -> int { return _y; }
    [[nodiscard]] auto MimeTypes() const -> const std::vector<std::string>& { return _mimeTypes; }
    [[nodiscard]] auto ProposedAction() const -> DragAction { return _proposedAction; }
private:
    int _x;
    int _y;
    std::vector<std::string> _mimeTypes;
    DragAction _proposedAction;
};

class MATCHA_EXPORT DragLeft final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DragLeft"; }
};

class MATCHA_EXPORT Dropped final : public Notification {
public:
    explicit Dropped(int x = 0, int y = 0,
                     std::string mimeType = {},
                     std::vector<uint8_t> data = {},
                     DragAction proposedAction = DragAction::Copy)
        : _x(x), _y(y), _mimeType(std::move(mimeType)),
          _data(std::move(data)), _proposedAction(proposedAction) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "Dropped"; }
    [[nodiscard]] auto X() const -> int { return _x; }
    [[nodiscard]] auto Y() const -> int { return _y; }
    [[nodiscard]] auto MimeType() const -> std::string_view { return _mimeType; }
    [[nodiscard]] auto Data() const -> const std::vector<uint8_t>& { return _data; }
    [[nodiscard]] auto ProposedAction() const -> DragAction { return _proposedAction; }
private:
    int _x;
    int _y;
    std::string _mimeType;
    std::vector<uint8_t> _data;
    DragAction _proposedAction;
};

// =========================================================================== //
//  Container-node notifications
// =========================================================================== //

class MATCHA_EXPORT TabSwitched final : public Notification {
public:
    explicit TabSwitched(std::string tabId = {}) : _tabId(std::move(tabId)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "TabSwitched"; }
    [[nodiscard]] auto TabId() const -> std::string_view { return _tabId; }
private:
    std::string _tabId;
};

class MATCHA_EXPORT CollapsedChanged final : public Notification {
public:
    explicit CollapsedChanged(bool collapsed = false) : _collapsed(collapsed) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "CollapsedChanged"; }
    [[nodiscard]] auto IsCollapsed() const -> bool { return _collapsed; }
private:
    bool _collapsed;
};

class MATCHA_EXPORT ButtonClicked final : public Notification {
public:
    explicit ButtonClicked(bool checked = false) : _checked(checked) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "ButtonClicked"; }
    [[nodiscard]] auto IsChecked() const -> bool { return _checked; }
private:
    bool _checked;
};

class MATCHA_EXPORT CloseRequested final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "CloseRequested"; }
    void SetCancel(bool cancel) { _cancel = cancel; }
    [[nodiscard]] auto IsCancelled() const -> bool { return _cancel; }
private:
    bool _cancel = false;
};

class MATCHA_EXPORT DialogClosed final : public Notification {
public:
    explicit DialogClosed(uint8_t result = 0) : _result(result) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DialogClosed"; }
    [[nodiscard]] auto Result() const -> uint8_t { return _result; }
private:
    uint8_t _result;
};

// =========================================================================== //
//  Widget-node notifications
// =========================================================================== //

class MATCHA_EXPORT Activated final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "Activated"; }
};

class MATCHA_EXPORT Clicked final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "Clicked"; }
};

class MATCHA_EXPORT Pressed final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "Pressed"; }
};

class MATCHA_EXPORT Released final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "Released"; }
};

class MATCHA_EXPORT RightClicked final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "RightClicked"; }
};

class MATCHA_EXPORT Toggled final : public Notification {
public:
    explicit Toggled(bool checked = false) : _checked(checked) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "Toggled"; }
    [[nodiscard]] auto IsChecked() const -> bool { return _checked; }
private:
    bool _checked;
};

class MATCHA_EXPORT TextChanged final : public Notification {
public:
    explicit TextChanged(std::string text = {}) : _text(std::move(text)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "TextChanged"; }
    [[nodiscard]] auto Text() const -> std::string_view { return _text; }
private:
    std::string _text;
};

class MATCHA_EXPORT EditingFinished final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "EditingFinished"; }
};

class MATCHA_EXPORT ReturnPressed final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "ReturnPressed"; }
};

class MATCHA_EXPORT SearchSubmitted final : public Notification {
public:
    explicit SearchSubmitted(std::string text = {}) : _text(std::move(text)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "SearchSubmitted"; }
    [[nodiscard]] auto Text() const -> std::string_view { return _text; }
private:
    std::string _text;
};

class MATCHA_EXPORT TextActivated final : public Notification {
public:
    explicit TextActivated(std::string text = {}) : _text(std::move(text)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "TextActivated"; }
    [[nodiscard]] auto Text() const -> std::string_view { return _text; }
private:
    std::string _text;
};

class MATCHA_EXPORT IndexChanged final : public Notification {
public:
    explicit IndexChanged(int index = -1) : _index(index) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "IndexChanged"; }
    [[nodiscard]] auto Index() const -> int { return _index; }
private:
    int _index;
};

class MATCHA_EXPORT IntValueChanged final : public Notification {
public:
    explicit IntValueChanged(int value = 0) : _value(value) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "IntValueChanged"; }
    [[nodiscard]] auto Value() const -> int { return _value; }
private:
    int _value;
};

class MATCHA_EXPORT DoubleValueChanged final : public Notification {
public:
    explicit DoubleValueChanged(double value = 0.0) : _value(value) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DoubleValueChanged"; }
    [[nodiscard]] auto Value() const -> double { return _value; }
private:
    double _value;
};

class MATCHA_EXPORT SliderPressed final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "SliderPressed"; }
};

class MATCHA_EXPORT SliderReleased final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "SliderReleased"; }
};

class MATCHA_EXPORT ColorChanged final : public Notification {
public:
    explicit ColorChanged(uint32_t rgba = 0) : _rgba(rgba) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "ColorChanged"; }
    [[nodiscard]] auto Rgba() const -> uint32_t { return _rgba; }
private:
    uint32_t _rgba;
};

class MATCHA_EXPORT CellSelected final : public Notification {
public:
    explicit CellSelected(int row = -1, int col = -1) : _row(row), _col(col) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "CellSelected"; }
    [[nodiscard]] auto Row() const -> int { return _row; }
    [[nodiscard]] auto Col() const -> int { return _col; }
private:
    int _row;
    int _col;
};

class MATCHA_EXPORT CellChanged final : public Notification {
public:
    explicit CellChanged(int row = -1, int col = -1, std::string value = {})
        : _row(row), _col(col), _value(std::move(value)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "CellChanged"; }
    [[nodiscard]] auto Row() const -> int { return _row; }
    [[nodiscard]] auto Col() const -> int { return _col; }
    [[nodiscard]] auto Value() const -> std::string_view { return _value; }
private:
    int _row;
    int _col;
    std::string _value;
};

class MATCHA_EXPORT DataChanged final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DataChanged"; }
};

class MATCHA_EXPORT RowAdded final : public Notification {
public:
    explicit RowAdded(int row = -1) : _row(row) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "RowAdded"; }
    [[nodiscard]] auto Row() const -> int { return _row; }
private:
    int _row;
};

class MATCHA_EXPORT ItemDoubleClicked final : public Notification {
public:
    explicit ItemDoubleClicked(int row = -1) : _row(row) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "ItemDoubleClicked"; }
    [[nodiscard]] auto Row() const -> int { return _row; }
private:
    int _row;
};

class MATCHA_EXPORT SelectionChanged final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "SelectionChanged"; }
};

class MATCHA_EXPORT SortChanged final : public Notification {
public:
    explicit SortChanged(int column = -1, int order = 0)
        : _column(column), _order(order) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "SortChanged"; }
    [[nodiscard]] auto Column() const -> int { return _column; }
    [[nodiscard]] auto Order() const -> int { return _order; }
private:
    int _column;
    int _order;
};

class MATCHA_EXPORT EmptyClicked final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "EmptyClicked"; }
};

class MATCHA_EXPORT RowRemoved final : public Notification {
public:
    explicit RowRemoved(int row = -1) : _row(row) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "RowRemoved"; }
    [[nodiscard]] auto Row() const -> int { return _row; }
private:
    int _row;
};

/**
 * @brief Bubbling context-menu request.
 *
 * Sent by any UiNode on right-click. Propagates UP the parent chain.
 * Each ancestor can call AddNode() in AnalyseNotification() to contribute
 * arbitrary UiNode controls (MenuItemNode, custom widgets, etc.).
 * After bubbling completes, the framework transfers all collected nodes
 * into the persistent ContextMenu (a MenuNode), then shows it.
 *
 * Nodes are stored as unique_ptr -- ownership transfers to the ContextMenu
 * on popup. The ContextMenu clears and rebuilds its children each time.
 */
class MATCHA_EXPORT ContextMenuRequest final : public Notification {
public:
    explicit ContextMenuRequest(int globalX = 0, int globalY = 0);
    ~ContextMenuRequest() override;

    ContextMenuRequest(ContextMenuRequest&&) noexcept;
    auto operator=(ContextMenuRequest&&) noexcept -> ContextMenuRequest&;

    [[nodiscard]] auto ClassName() const -> std::string_view override { return "ContextMenuRequest"; }

    [[nodiscard]] auto GlobalX() const -> int { return _globalX; }
    [[nodiscard]] auto GlobalY() const -> int { return _globalY; }

    /// @brief Contribute a UiNode to the context menu (call from AnalyseNotification).
    /// Ownership is held until the ContextMenu takes it on popup.
    void AddNode(std::unique_ptr<UiNode> node);

    /// @brief Number of contributed nodes.
    [[nodiscard]] auto NodeCount() const -> size_t;

    /// @brief Take all contributed nodes (ownership transfer to caller).
    [[nodiscard]] auto TakeNodes() -> std::vector<std::unique_ptr<UiNode>>;

private:
    int _globalX;
    int _globalY;
    std::vector<std::unique_ptr<UiNode>> _nodes;
};

/**
 * @brief Notification sent when a context menu item is activated.
 *
 * Dispatched by the ContextMenu (MenuNode) when the user clicks an item.
 * Carries only POD: the actionId string identifying which item was triggered.
 * Business layer subscribes to this notification to handle the action.
 */
class MATCHA_EXPORT ContextMenuItemActivated final : public Notification {
public:
    explicit ContextMenuItemActivated(std::string actionId = {})
        : _actionId(std::move(actionId)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "ContextMenuItemActivated"; }
    [[nodiscard]] auto ActionId() const -> std::string_view { return _actionId; }
private:
    std::string _actionId;
};

class MATCHA_EXPORT PropertyChanged final : public Notification {
public:
    explicit PropertyChanged(std::string key = {}, std::string value = {})
        : _key(std::move(key)), _value(std::move(value)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "PropertyChanged"; }
    [[nodiscard]] auto Key() const -> std::string_view { return _key; }
    [[nodiscard]] auto Value() const -> std::string_view { return _value; }
private:
    std::string _key;
    std::string _value;
};

class MATCHA_EXPORT RangeChanged final : public Notification {
public:
    explicit RangeChanged(int low = 0, int high = 0) : _low(low), _high(high) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "RangeChanged"; }
    [[nodiscard]] auto Low() const -> int { return _low; }
    [[nodiscard]] auto High() const -> int { return _high; }
private:
    int _low;
    int _high;
};

class MATCHA_EXPORT PageChanged final : public Notification {
public:
    explicit PageChanged(int page = 0) : _page(page) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "PageChanged"; }
    [[nodiscard]] auto Page() const -> int { return _page; }
private:
    int _page;
};

class MATCHA_EXPORT ResetClicked final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "ResetClicked"; }
};

class MATCHA_EXPORT Dismissed final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "Dismissed"; }
};

class MATCHA_EXPORT ActionClicked final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "ActionClicked"; }
};

class MATCHA_EXPORT ExpandToggled final : public Notification {
public:
    explicit ExpandToggled(bool expanded = false) : _expanded(expanded) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "ExpandToggled"; }
    [[nodiscard]] auto IsExpanded() const -> bool { return _expanded; }
private:
    bool _expanded;
};

class MATCHA_EXPORT DateTimeChanged final : public Notification {
public:
    explicit DateTimeChanged(int64_t msecsSinceEpoch = 0) : _msecs(msecsSinceEpoch) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DateTimeChanged"; }
    [[nodiscard]] auto MsecsSinceEpoch() const -> int64_t { return _msecs; }
private:
    int64_t _msecs;
};

class MATCHA_EXPORT LinkActivated final : public Notification {
public:
    explicit LinkActivated(std::string url = {}) : _url(std::move(url)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "LinkActivated"; }
    [[nodiscard]] auto Url() const -> std::string_view { return _url; }
private:
    std::string _url;
};

// =========================================================================== //
//  Popup lifecycle notifications (PopupNode)
// =========================================================================== //

class MATCHA_EXPORT PopupOpened final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "PopupOpened"; }
};

class MATCHA_EXPORT PopupClosed final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "PopupClosed"; }
};

// =========================================================================== //
//  Locale / i18n notifications (C8)
// =========================================================================== //

class MATCHA_EXPORT LocaleChanged final : public Notification {
public:
    explicit LocaleChanged(std::string locale = {})
        : _locale(std::move(locale)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "LocaleChanged"; }
    [[nodiscard]] auto Locale() const -> const std::string& { return _locale; }
private:
    std::string _locale;
};

// =========================================================================== //
//  Help system notifications (C9/C10)
// =========================================================================== //

class MATCHA_EXPORT HelpRequested final : public Notification {
public:
    explicit HelpRequested(std::string helpId = {}, bool whatsThisMode = false)
        : _helpId(std::move(helpId)), _whatsThisMode(whatsThisMode) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "HelpRequested"; }
    [[nodiscard]] auto GetHelpId() const -> const std::string& { return _helpId; }
    [[nodiscard]] auto IsWhatsThisMode() const -> bool { return _whatsThisMode; }
private:
    std::string _helpId;
    bool _whatsThisMode;
};

// =========================================================================== //
//  Animation lifecycle notifications (RFC-08)
// =========================================================================== //

class MATCHA_EXPORT AnimationStarted final : public Notification {
public:
    explicit AnimationStarted(AnimationPropertyId prop = AnimationPropertyId::Opacity,
                              TransitionHandle handle = TransitionHandle::Invalid)
        : _property(prop), _handle(handle) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "AnimationStarted"; }
    [[nodiscard]] auto Property() const -> AnimationPropertyId { return _property; }
    [[nodiscard]] auto Handle() const -> TransitionHandle { return _handle; }
private:
    AnimationPropertyId _property;
    TransitionHandle    _handle;
};

class MATCHA_EXPORT AnimationCompleted final : public Notification {
public:
    explicit AnimationCompleted(AnimationPropertyId prop = AnimationPropertyId::Opacity,
                                TransitionHandle handle = TransitionHandle::Invalid)
        : _property(prop), _handle(handle) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "AnimationCompleted"; }
    [[nodiscard]] auto Property() const -> AnimationPropertyId { return _property; }
    [[nodiscard]] auto Handle() const -> TransitionHandle { return _handle; }
private:
    AnimationPropertyId _property;
    TransitionHandle    _handle;
};

class MATCHA_EXPORT AnimationCancelled final : public Notification {
public:
    explicit AnimationCancelled(AnimationPropertyId prop = AnimationPropertyId::Opacity,
                                TransitionHandle handle = TransitionHandle::Invalid)
        : _property(prop), _handle(handle) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "AnimationCancelled"; }
    [[nodiscard]] auto Property() const -> AnimationPropertyId { return _property; }
    [[nodiscard]] auto Handle() const -> TransitionHandle { return _handle; }
private:
    AnimationPropertyId _property;
    TransitionHandle    _handle;
};

} // namespace matcha::fw
