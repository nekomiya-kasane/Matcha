#pragma once

/**
 * @file PopConfirmNode.h
 * @brief UiNode wrapper for NyanPopConfirm via PopupNode base.
 *
 * PopConfirmNode exposes NyanPopConfirm's confirmation dialog through the
 * UiNode tree, inheriting positioning and lifecycle management from PopupNode.
 *
 * @see PopupNode for base popup behavior.
 * @see NyanPopConfirm for the underlying Qt widget.
 */

#include "Matcha/Tree/PopupNode.h"

#include <string>
#include <string_view>

namespace matcha::gui {
class NyanPopConfirm;
enum class PopConfirmState : uint8_t;
enum class ArrowPosition : uint8_t;
} // namespace matcha::gui

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanPopConfirm, using PopupNode as base.
 *
 * Dispatches: PopupOpened, PopupClosed (inherited from PopupNode).
 */
class MATCHA_EXPORT PopConfirmNode : public PopupNode {
    MATCHA_DECLARE_CLASS

public:
    explicit PopConfirmNode(std::string id);
    ~PopConfirmNode() override;

    PopConfirmNode(const PopConfirmNode&) = delete;
    auto operator=(const PopConfirmNode&) -> PopConfirmNode& = delete;
    PopConfirmNode(PopConfirmNode&&) = delete;
    auto operator=(PopConfirmNode&&) -> PopConfirmNode& = delete;

    // -- Content --

    void SetTitle(std::string_view title);
    [[nodiscard]] auto Title() const -> std::string;

    void SetMessage(std::string_view message);
    [[nodiscard]] auto Message() const -> std::string;

    void SetState(gui::PopConfirmState state);

    // -- Access underlying widget --

    [[nodiscard]] auto PopConfirm() -> gui::NyanPopConfirm*;

protected:
    auto CreatePopupContent(QWidget* parent) -> QWidget* override;
    auto PreferredSize() -> Size override;

private:
    gui::NyanPopConfirm* _popConfirm = nullptr;
};

} // namespace matcha::fw
