#pragma once

/**
 * @file NyanCadDocument.h
 * @brief Business document model for NyanCad demo.
 *
 * Manages per-document state (stub mesh/entity data). Receives
 * DocumentManager Notifications via the command tree.
 */

#include "Matcha/Foundation/StrongId.h"
#include "Matcha/Foundation/Types.h"
#include "Matcha/UiNodes/Core/CommandNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace matcha::fw {
class DocumentManager;
} // namespace matcha::fw

namespace nyancad {

/**
 * @brief Per-document business state (stub geometry engine).
 */
struct DocumentState {
    std::string name;
    bool modified = false;
    int entityCount = 0; ///< Stub entity count.
};

/**
 * @brief Manages a map of DocumentId -> DocumentState.
 *
 * Receives DocumentManager Notifications via the command tree
 * to maintain a map of live documents.
 */
class NyanCadDocumentHost : public matcha::CommandNode {
public:
    /**
     * @brief Construct and attach as child of the given parent CommandNode.
     * @param parent Parent in the command tree (typically DocumentManager or Shell).
     */
    explicit NyanCadDocumentHost(matcha::CommandNode* parent = nullptr);

    /**
     * @brief Create a named document via the framework DocumentManager.
     * @param docMgr The framework DocumentManager.
     * @param name Display name for the document.
     * @return The new document ID, or error.
     */
    [[nodiscard]] auto CreateDocument(matcha::fw::DocumentManager& docMgr,
                                       std::string_view name)
        -> matcha::fw::Expected<matcha::fw::DocumentId>;

    /**
     * @brief Create initial demo documents.
     * @param docMgr The framework DocumentManager.
     */
    void CreateInitialDocuments(matcha::fw::DocumentManager& docMgr);

    /// @brief Get the state for a document. Returns nullptr if not found.
    [[nodiscard]] auto GetState(matcha::fw::DocumentId docId) -> DocumentState*;

    /// @brief Get the state for a document (const). Returns nullptr if not found.
    [[nodiscard]] auto GetState(matcha::fw::DocumentId docId) const
        -> const DocumentState*;

    /// @brief Get the active document state. Returns nullptr if none active.
    [[nodiscard]] auto ActiveState() -> DocumentState*;

    /// @brief Number of live documents.
    [[nodiscard]] auto DocumentCount() const -> size_t;

protected:
    [[nodiscard]] auto AnalyseNotification(matcha::CommandNode* sender,
                                            matcha::Notification& notif)
        -> matcha::PropagationMode override;

private:
    void OnDocCreated(matcha::fw::DocumentId docId, std::string_view name);
    void OnDocClosed(matcha::fw::DocumentId docId);
    void OnDocSwitched(matcha::fw::DocumentId docId);

    std::unordered_map<uint64_t, DocumentState> _documents;
    std::vector<std::string> _pendingNames;
    matcha::fw::DocumentId _activeDocId{};
    int _defaultDocCounter = 0;
};

} // namespace nyancad
