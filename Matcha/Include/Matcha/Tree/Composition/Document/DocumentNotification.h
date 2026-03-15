#pragma once

/**
 * @file DocumentNotification.h
 * @brief Notification types for DocumentArea, DocumentManager, and ViewportGroup.
 *
 * These notifications belong to the document/viewport service layer.
 * They use StrongId types (PageId, DocumentId, WindowId, ViewportId).
 */

#include "Matcha/Core/StrongId.h"
#include "Matcha/Event/Notification.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace matcha::fw {

// =========================================================================== //
//  DocumentArea notifications
// =========================================================================== //

class MATCHA_EXPORT PageSwitched final : public Notification {
public:
    explicit PageSwitched(PageId pageId = {}) : _pageId(pageId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "PageSwitched"; }
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }
private:
    PageId _pageId;
};

class MATCHA_EXPORT PageCreated final : public Notification {
public:
    explicit PageCreated(PageId pageId = {}, DocumentId docId = {})
        : _pageId(pageId), _docId(docId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "PageCreated"; }
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }
    [[nodiscard]] auto DocId() const -> DocumentId { return _docId; }
private:
    PageId _pageId;
    DocumentId _docId;
};

class MATCHA_EXPORT PageRemoved final : public Notification {
public:
    explicit PageRemoved(PageId pageId = {}) : _pageId(pageId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "PageRemoved"; }
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }
private:
    PageId _pageId;
};

// =========================================================================== //
//  DocumentManager notifications
// =========================================================================== //

class MATCHA_EXPORT DocumentCreated final : public Notification {
public:
    explicit DocumentCreated(DocumentId docId = {}, std::string name = {})
        : _docId(docId), _name(std::move(name)) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DocumentCreated"; }
    [[nodiscard]] auto DocId() const -> DocumentId { return _docId; }
    [[nodiscard]] auto Name() const -> std::string_view { return _name; }
private:
    DocumentId _docId;
    std::string _name;
};

class MATCHA_EXPORT DocumentSwitched final : public Notification {
public:
    explicit DocumentSwitched(DocumentId docId = {}) : _docId(docId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DocumentSwitched"; }
    [[nodiscard]] auto DocId() const -> DocumentId { return _docId; }
private:
    DocumentId _docId;
};

class MATCHA_EXPORT DocumentClosing final : public Notification {
public:
    explicit DocumentClosing(DocumentId docId = {}) : _docId(docId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DocumentClosing"; }
    [[nodiscard]] auto DocId() const -> DocumentId { return _docId; }
    void SetCancel(bool cancel) { _cancel = cancel; }
    [[nodiscard]] auto IsCancelled() const -> bool { return _cancel; }
private:
    DocumentId _docId;
    bool _cancel = false;
};

class MATCHA_EXPORT DocumentClosed final : public Notification {
public:
    explicit DocumentClosed(DocumentId docId = {}) : _docId(docId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DocumentClosed"; }
    [[nodiscard]] auto DocId() const -> DocumentId { return _docId; }
private:
    DocumentId _docId;
};

class MATCHA_EXPORT DocumentPageMoved final : public Notification {
public:
    explicit DocumentPageMoved(PageId pageId = {}, WindowId from = {}, WindowId to = {})
        : _pageId(pageId), _from(from), _to(to) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "DocumentPageMoved"; }
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }
    [[nodiscard]] auto FromWindow() const -> WindowId { return _from; }
    [[nodiscard]] auto ToWindow() const -> WindowId { return _to; }
private:
    PageId _pageId;
    WindowId _from;
    WindowId _to;
};

// =========================================================================== //
//  ViewportGroup notifications
// =========================================================================== //

class MATCHA_EXPORT VpCreated final : public Notification {
public:
    explicit VpCreated(ViewportId newId = {}, ViewportId splitFrom = {}, uint8_t direction = 0)
        : _newId(newId), _splitFrom(splitFrom), _direction(direction) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VpCreated"; }
    [[nodiscard]] auto NewId() const -> ViewportId { return _newId; }
    [[nodiscard]] auto SplitFrom() const -> ViewportId { return _splitFrom; }
    [[nodiscard]] auto Direction() const -> uint8_t { return _direction; }
private:
    ViewportId _newId;
    ViewportId _splitFrom;
    uint8_t _direction;
};

class MATCHA_EXPORT VpRemoved final : public Notification {
public:
    explicit VpRemoved(ViewportId removedId = {}) : _removedId(removedId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VpRemoved"; }
    [[nodiscard]] auto RemovedId() const -> ViewportId { return _removedId; }
private:
    ViewportId _removedId;
};

class MATCHA_EXPORT VpSwapped final : public Notification {
public:
    explicit VpSwapped(ViewportId a = {}, ViewportId b = {}) : _a(a), _b(b) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VpSwapped"; }
    [[nodiscard]] auto IdA() const -> ViewportId { return _a; }
    [[nodiscard]] auto IdB() const -> ViewportId { return _b; }
private:
    ViewportId _a;
    ViewportId _b;
};

class MATCHA_EXPORT VpMoved final : public Notification {
public:
    explicit VpMoved(ViewportId vpId = {}) : _vpId(vpId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VpMoved"; }
    [[nodiscard]] auto GetViewportId() const -> ViewportId { return _vpId; }
private:
    ViewportId _vpId;
};

class MATCHA_EXPORT VpMaximized final : public Notification {
public:
    explicit VpMaximized(ViewportId vpId = {}) : _vpId(vpId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VpMaximized"; }
    [[nodiscard]] auto GetViewportId() const -> ViewportId { return _vpId; }
private:
    ViewportId _vpId;
};

class MATCHA_EXPORT VpRestored final : public Notification {
public:
    explicit VpRestored(ViewportId vpId = {}) : _vpId(vpId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VpRestored"; }
    [[nodiscard]] auto GetViewportId() const -> ViewportId { return _vpId; }
private:
    ViewportId _vpId;
};

class MATCHA_EXPORT ActiveVpChanged final : public Notification {
public:
    explicit ActiveVpChanged(ViewportId vpId = {}) : _vpId(vpId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "ActiveVpChanged"; }
    [[nodiscard]] auto GetViewportId() const -> ViewportId { return _vpId; }
private:
    ViewportId _vpId;
};

class MATCHA_EXPORT VpStateChanged final : public Notification {
public:
    explicit VpStateChanged(uint8_t oldState = 0, uint8_t newState = 0)
        : _oldState(oldState), _newState(newState) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VpStateChanged"; }
    [[nodiscard]] auto OldState() const -> uint8_t { return _oldState; }
    [[nodiscard]] auto NewState() const -> uint8_t { return _newState; }
private:
    uint8_t _oldState;
    uint8_t _newState;
};

class MATCHA_EXPORT VpSplitRatioChanged final : public Notification {
public:
    explicit VpSplitRatioChanged(ViewportId first = {}, ViewportId second = {}, double ratio = 0.5)
        : _first(first), _second(second), _ratio(ratio) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VpSplitRatioChanged"; }
    [[nodiscard]] auto First() const -> ViewportId { return _first; }
    [[nodiscard]] auto Second() const -> ViewportId { return _second; }
    [[nodiscard]] auto Ratio() const -> double { return _ratio; }
private:
    ViewportId _first;
    ViewportId _second;
    double _ratio;
};

class MATCHA_EXPORT VpLayoutRebuilt final : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VpLayoutRebuilt"; }
};

// =========================================================================== //
//  Viewport notifications
// =========================================================================== //

class MATCHA_EXPORT VpFocusChanged final : public Notification {
public:
    explicit VpFocusChanged(ViewportId vpId = {}, bool focused = false)
        : _vpId(vpId), _focused(focused) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "VpFocusChanged"; }
    [[nodiscard]] auto GetViewportId() const -> ViewportId { return _vpId; }
    [[nodiscard]] auto IsFocused() const -> bool { return _focused; }
private:
    ViewportId _vpId;
    bool _focused;
};

} // namespace matcha::fw
