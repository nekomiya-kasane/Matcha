#pragma once

/**
 * @file DialogNode.h
 * @brief UiNode wrapper for NyanDialog -- framework-managed dialog container.
 *
 * Supports 3 modality types: Modal, SemiModal, Modeless.
 *
 * @see docs/02_Architecture_Design.md section 2.5.3 (Dialog)
 */

#include "Matcha/UiNodes/Core/UiNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include <string>
#include <string_view>

namespace matcha::gui {
class IThemeService;
class NyanDialog;
enum class DialogModality : uint8_t;
enum class DialogResult : uint8_t;
} // namespace matcha::gui

class QWidget;

namespace matcha::fw {

/** @brief Preset dialog widths. */
enum class DialogWidth : int {
    Small  = 400,
    Medium = 600,
    Large  = 800,
};

/**
 * @brief UiNode wrapper for NyanDialog.
 */
class MATCHA_EXPORT DialogNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification {
        using Closed = matcha::fw::DialogClosed;
    };

    DialogNode(std::string id, UiNode* parentHint = nullptr);
    ~DialogNode() override;

    DialogNode(const DialogNode&) = delete;
    auto operator=(const DialogNode&) -> DialogNode& = delete;
    DialogNode(DialogNode&&) = delete;
    auto operator=(DialogNode&&) -> DialogNode& = delete;

    [[nodiscard]] auto Dialog() -> gui::NyanDialog*;
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Title --

    void SetTitle(std::string_view title);
    [[nodiscard]] auto Title() const -> std::string;

    // -- Content --

    /// @note This accepts a raw QWidget* as an intentional escape hatch
    ///       for embedding arbitrary dialog content.
    void SetContent(QWidget* content);

    /// @brief Set dialog content from a UiNode subtree (Qt-free alternative).
    /// Ownership is transferred to this DialogNode.
    void SetContentNode(std::unique_ptr<UiNode> contentNode);

    // -- Modality --

    void SetModality(gui::DialogModality modality);
    [[nodiscard]] auto Modality() const -> gui::DialogModality;

    // -- Show --

    /// @brief Show as modal. Blocks until user closes. Returns DialogResult.
    [[nodiscard]] auto ShowModal() -> gui::DialogResult;

    /// @brief Show as semi-modal (blocks parent + siblings, allows viewport interaction).
    void ShowSemiModal();

    /// @brief Show as modeless (non-blocking).
    void ShowModeless();

    /// @brief Set dialog width from preset.
    void SetWidth(DialogWidth width);

    /// @brief Close the dialog.
    void Close();

private:
    gui::NyanDialog* _dialog;
};

} // namespace matcha::fw
