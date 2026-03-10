/**
 * @file NyanCadDocument.cpp
 * @brief NyanCadDocumentHost implementation.
 */

#include "NyanCadDocument.h"

#include "Matcha/Services/DocumentManager.h"

namespace nyancad {

NyanCadDocumentHost::NyanCadDocumentHost(matcha::CommandNode* parent)
    : CommandNode(parent, "NyanCadDocumentHost", matcha::CommandMode::Undefined)
{
}

auto NyanCadDocumentHost::AnalyseNotification(
    matcha::CommandNode* /*sender*/,
    matcha::Notification& notif) -> matcha::PropagationMode
{
    using namespace matcha::fw;

    if (auto* n = notif.As<DocumentCreated>()) {
        OnDocCreated(n->DocId(), n->Name());
        return matcha::PropagationMode::TransmitToParent;
    }
    if (auto* n = notif.As<DocumentClosed>()) {
        OnDocClosed(n->DocId());
        return matcha::PropagationMode::TransmitToParent;
    }
    if (auto* n = notif.As<DocumentSwitched>()) {
        OnDocSwitched(n->DocId());
        return matcha::PropagationMode::TransmitToParent;
    }
    return matcha::PropagationMode::TransmitToParent;
}

auto NyanCadDocumentHost::CreateDocument(matcha::fw::DocumentManager& docMgr,
                                          std::string_view name)
    -> matcha::fw::Expected<matcha::fw::DocumentId>
{
    _pendingNames.emplace_back(name);
    return docMgr.CreateDocument(name);
}

void NyanCadDocumentHost::CreateInitialDocuments(
    matcha::fw::DocumentManager& docMgr)
{
    (void)CreateDocument(docMgr, "Bracket_v1");
    (void)CreateDocument(docMgr, "Housing_v2");
}

auto NyanCadDocumentHost::GetState(matcha::fw::DocumentId docId)
    -> DocumentState*
{
    auto it = _documents.find(docId.value);
    return it != _documents.end() ? &it->second : nullptr;
}

auto NyanCadDocumentHost::GetState(matcha::fw::DocumentId docId) const
    -> const DocumentState*
{
    auto it = _documents.find(docId.value);
    return it != _documents.end() ? &it->second : nullptr;
}

auto NyanCadDocumentHost::ActiveState() -> DocumentState*
{
    return GetState(_activeDocId);
}

auto NyanCadDocumentHost::DocumentCount() const -> size_t
{
    return _documents.size();
}

void NyanCadDocumentHost::OnDocCreated(matcha::fw::DocumentId docId,
                                        [[maybe_unused]] std::string_view name)
{
    DocumentState state;
    if (!_pendingNames.empty()) {
        state.name = _pendingNames.front();
        _pendingNames.erase(_pendingNames.begin());
    } else {
        state.name = "Document_" + std::to_string(++_defaultDocCounter);
    }
    state.entityCount = 0;
    state.modified = false;
    _documents[docId.value] = std::move(state);
    _activeDocId = docId;
}

void NyanCadDocumentHost::OnDocClosed(matcha::fw::DocumentId docId)
{
    _documents.erase(docId.value);
    if (_activeDocId == docId) {
        _activeDocId = {};
    }
}

void NyanCadDocumentHost::OnDocSwitched(matcha::fw::DocumentId docId)
{
    _activeDocId = docId;
}

} // namespace nyancad
