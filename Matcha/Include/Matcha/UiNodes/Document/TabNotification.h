#pragma once

/**
 * @file TabNotification.h
 * @brief Notification types for TabBarNode tab-page operations.
 *
 * These notifications are emitted by TabBarNode when tabs are switched,
 * close-requested, dragged out, dropped in, or reordered.
 * They use PageId (StrongId) and live at the Document layer.
 */

#include "Matcha/Foundation/StrongId.h"
#include "Matcha/UiNodes/Core/Notification.h"

#include <string_view>

namespace matcha::fw {

// =========================================================================== //
//  Tab-page notifications (PageId-based, shared by TabBarNode variants)
// =========================================================================== //

class MATCHA_EXPORT TabPageSwitched final : public Notification {
public:
    explicit TabPageSwitched(PageId pageId = {}) : _pageId(pageId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "TabPageSwitched"; }
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }
private:
    PageId _pageId;
};

class MATCHA_EXPORT TabPageCloseRequested final : public Notification {
public:
    explicit TabPageCloseRequested(PageId pageId = {}) : _pageId(pageId) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "TabPageCloseRequested"; }
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }
private:
    PageId _pageId;
};

class MATCHA_EXPORT TabPageDraggedOut final : public Notification {
public:
    explicit TabPageDraggedOut(PageId pageId = {}, int globalX = 0, int globalY = 0)
        : _pageId(pageId), _globalX(globalX), _globalY(globalY) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "TabPageDraggedOut"; }
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }
    [[nodiscard]] auto GlobalX() const -> int { return _globalX; }
    [[nodiscard]] auto GlobalY() const -> int { return _globalY; }
private:
    PageId _pageId;
    int _globalX;
    int _globalY;
};

class MATCHA_EXPORT TabDroppedIn final : public Notification {
public:
    explicit TabDroppedIn(PageId pageId = {}, int insertIndex = -1)
        : _pageId(pageId), _insertIndex(insertIndex) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "TabDroppedIn"; }
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }
    [[nodiscard]] auto InsertIndex() const -> int { return _insertIndex; }
private:
    PageId _pageId;
    int _insertIndex;
};

class MATCHA_EXPORT TabReordered final : public Notification {
public:
    explicit TabReordered(PageId pageId = {}, int oldIndex = -1, int newIndex = -1)
        : _pageId(pageId), _oldIndex(oldIndex), _newIndex(newIndex) {}
    [[nodiscard]] auto ClassName() const -> std::string_view override { return "TabReordered"; }
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }
    [[nodiscard]] auto OldIndex() const -> int { return _oldIndex; }
    [[nodiscard]] auto NewIndex() const -> int { return _newIndex; }
private:
    PageId _pageId;
    int _oldIndex;
    int _newIndex;
};

} // namespace matcha::fw
