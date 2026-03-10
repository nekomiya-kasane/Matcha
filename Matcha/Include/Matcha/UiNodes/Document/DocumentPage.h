#pragma once

/**
 * @file DocumentPage.h
 * @brief DocumentPage UiNode -- one view of a Document within a WindowNode.
 *
 * Supports the 1:N Document:DocumentPage model (CATIA multi-view).
 * Each DocumentPage owns a ViewportGroup for independent camera/layout.
 * Multiple DocumentPages may reference the same DocumentId in different windows.
 *
 * @see 05_Greenfield_Plan.md ss 4.2 for the three-layer model.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Foundation/Types.h>
#include <Matcha/UiNodes/Core/UiNode.h>

#include <string>

namespace matcha::fw {

class ViewportGroup;
class WindowNode;

/**
 * @brief UiNode representing a single view of a Document.
 *
 * Owned by DocumentArea. Each DocumentPage appears as a tab in the
 * document tab bar. It contains a ViewportGroup child for viewport layout.
 *
 * **Identity**: `PageId` (unique across all windows).
 * **Ownership**: DocumentArea owns DocumentPage via unique_ptr.
 */
class MATCHA_EXPORT DocumentPage : public UiNode {
public:
    DocumentPage(std::string name, PageId pageId, DocumentId docId);
    ~DocumentPage() override;

    DocumentPage(const DocumentPage&)            = delete;
    DocumentPage& operator=(const DocumentPage&) = delete;
    DocumentPage(DocumentPage&&)                 = delete;
    DocumentPage& operator=(DocumentPage&&)      = delete;

    // -- Identity --

    /** @brief Unique page identity. */
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }

    /** @brief Document this page views. */
    [[nodiscard]] auto GetDocId() const -> DocumentId { return _docId; }

    // -- Viewport access --

    /** @brief Access the ViewportGroup child (binary split tree root). */
    [[nodiscard]] auto GetViewportGroup() -> observer_ptr<ViewportGroup>;

    // -- Tab appearance --

    void SetTabTitle(std::string_view title);
    [[nodiscard]] auto TabTitle() const -> std::string_view { return _tabTitle; }

    void SetTabIcon(std::string_view iconPath);
    [[nodiscard]] auto TabIcon() const -> std::string_view { return _tabIcon; }

    void SetModified(bool modified);
    [[nodiscard]] auto IsModified() const -> bool { return _modified; }

    // -- Window association --

    /** @brief The WindowNode containing this page's DocumentArea. */
    [[nodiscard]] auto GetWindow() const -> observer_ptr<WindowNode>;

private:
    PageId     _pageId;
    DocumentId _docId;
    std::string _tabTitle;
    std::string _tabIcon;
    bool        _modified = false;
};

} // namespace matcha::fw
