#pragma once

/**
 * @file DocumentArea.h
 * @brief DocumentArea UiNode -- manages DocumentPage tabs within a WindowNode.
 *
 * Each WindowNode contains one DocumentArea. The DocumentArea manages
 * the document tab bar and the active DocumentPage display.
 *
 * @see 05_Greenfield_Plan.md ss 8.2.3 for the API specification.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Core/Types.h>
#include <Matcha/Tree/WidgetNode.h>

#include <memory>
#include <optional>
#include <unordered_map>

class QStackedWidget;

namespace matcha::fw {

class DocumentPage;

/**
 * @brief UiNode container for DocumentPage tabs.
 *
 * **Responsibilities**:
 * - Owns DocumentPage children
 * - Manages tab bar (add/remove/switch tabs)
 * - Tracks active page
 *
 * **Ownership**: WindowNode owns DocumentArea.
 */
class MATCHA_EXPORT DocumentArea : public WidgetNode {
public:
    explicit DocumentArea(std::string name);
    ~DocumentArea() override;

    DocumentArea(const DocumentArea&)            = delete;
    DocumentArea& operator=(const DocumentArea&) = delete;
    DocumentArea(DocumentArea&&)                 = delete;
    DocumentArea& operator=(DocumentArea&&)      = delete;

    // -- Page management (UiNode tree) --

    /** @brief Create a new DocumentPage for the given document, add it, and return a non-owning pointer. */
    auto CreatePage(std::string name, DocumentId docId) -> observer_ptr<DocumentPage>;

    /** @brief Add a DocumentPage as a new tab. Takes ownership. */
    void AddPage(std::unique_ptr<DocumentPage> page);

    /** @brief Remove a page by its PageId. Returns ownership to caller. */
    auto RemovePage(PageId id) -> std::unique_ptr<DocumentPage>;

    /** @brief Get the currently active (visible) page. */
    [[nodiscard]] auto ActivePage() -> observer_ptr<DocumentPage>;

    /** @brief Get the active page ID (nullopt if no pages). */
    [[nodiscard]] auto ActivePageId() const -> std::optional<PageId> { return _activePageId; }

    /** @brief Switch the active tab to the given page. */
    auto SwitchPage(PageId id) -> Expected<void>;

    /** @brief Number of document tabs. */
    [[nodiscard]] auto PageCount() const -> std::size_t;

    /** @brief Find a page by its PageId. */
    [[nodiscard]] auto FindPage(PageId id) -> observer_ptr<DocumentPage>;

    /** @brief Find a page by its DocumentId. */
    [[nodiscard]] auto FindPageByDoc(DocumentId docId) -> observer_ptr<DocumentPage>;

    // -- Page widget management (QStackedWidget) --

    /** @brief Register a page content widget in the internal stack. */
    void AddPageWidget(PageId pageId, QWidget* pageWidget);

    /** @brief Remove and delete a page content widget. */
    void RemovePageWidget(PageId pageId);

    /** @brief Switch the visible page widget in the stack. */
    void SwitchPageWidget(PageId pageId);

    /** @brief Show placeholder when no pages are open. */
    void ShowPlaceholder();

    /** @brief Check if a page widget is registered. */
    [[nodiscard]] auto HasPageWidget(PageId pageId) const -> bool;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;

private:
    std::optional<PageId> _activePageId;
    uint32_t              _nextPageId = 1;

    QWidget*       _container     = nullptr;
    QStackedWidget* _contentStack = nullptr;
    QWidget*       _placeholder   = nullptr;
    std::unordered_map<uint64_t, QWidget*> _pageWidgets;
};

} // namespace matcha::fw
