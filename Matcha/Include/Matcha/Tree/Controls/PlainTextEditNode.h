#pragma once

/**
 * @file PlainTextEditNode.h
 * @brief Typed WidgetNode wrapping QPlainTextEdit for UiNode tree.
 *
 * Provides a read-only or editable multi-line plain text area.
 * Primary use case: log viewers, code display, debug output.
 *
 * Dispatches: TextChanged (when editable text is modified by the user).
 */

#include "Matcha/Tree/WidgetNode.h"

#include <string>
#include <string_view>

class QPlainTextEdit;

namespace matcha::fw {

/**
 * @brief UiNode wrapper for QPlainTextEdit (Scheme D typed node).
 *
 * Dispatches: TextChanged (on user edit, not programmatic changes).
 */
class MATCHA_EXPORT PlainTextEditNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using TextChanged = matcha::fw::TextChanged;
    };

    explicit PlainTextEditNode(std::string id);
    ~PlainTextEditNode() override;

    PlainTextEditNode(const PlainTextEditNode&) = delete;
    auto operator=(const PlainTextEditNode&) -> PlainTextEditNode& = delete;
    PlainTextEditNode(PlainTextEditNode&&) = delete;
    auto operator=(PlainTextEditNode&&) -> PlainTextEditNode& = delete;

    // -- Content --

    void SetPlainText(std::string_view text);
    [[nodiscard]] auto PlainText() const -> std::string;

    void AppendPlainText(std::string_view text);
    void Clear();

    // -- Properties --

    void SetReadOnly(bool readOnly);
    [[nodiscard]] auto IsReadOnly() const -> bool;

    void SetMaximumBlockCount(int count);
    void SetFont(std::string_view family, int pointSize);

    void SetStyleSheet(std::string_view css);

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;

private:
    QPlainTextEdit* _edit = nullptr;
};

} // namespace matcha::fw
