#pragma once

/**
 * @file StatusItemNode.h
 * @brief UiNode child of StatusBarNode representing a single status bar item.
 *
 * Each StatusItemNode wraps a QWidget that is displayed inside the NyanStatusBar.
 * Items can be labels, progress bars, or arbitrary widgets.
 *
 * @see StatusBarNode for the parent container.
 */

#include "Matcha/Tree/UiNode.h"

#include <string_view>

class QWidget;

namespace matcha::gui {
class NyanLabel;
class NyanProgressBar;
} // namespace matcha::gui

namespace matcha::fw {

/**
 * @brief The kind of widget a StatusItemNode wraps.
 */
enum class StatusItemKind : uint8_t {
    Label,
    Progress,
    Custom,
};

/**
 * @brief A single item inside a StatusBarNode.
 *
 * Created by StatusBarNode::AddLabel / AddProgress / AddWidget.
 * The item owns a QWidget that is displayed in the NyanStatusBar layout.
 */
class MATCHA_EXPORT StatusItemNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    StatusItemNode(std::string id, StatusItemKind kind, QWidget* widget);
    ~StatusItemNode() override;

    StatusItemNode(const StatusItemNode&) = delete;
    auto operator=(const StatusItemNode&) -> StatusItemNode& = delete;
    StatusItemNode(StatusItemNode&&) = delete;
    auto operator=(StatusItemNode&&) -> StatusItemNode& = delete;

    /// @brief The kind of item.
    [[nodiscard]] auto Kind() const -> StatusItemKind { return _kind; }

    /// @brief Get the underlying QWidget.
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Label item API --

    /// @brief Set text (only valid for Label kind).
    void SetText(std::string_view text);

    /// @brief Get text (only valid for Label kind).
    [[nodiscard]] auto Text() const -> std::string_view;

    // -- Progress item API --

    /// @brief Set progress value 0-100 (only valid for Progress kind).
    void SetValue(int percent);

    /// @brief Get progress value (only valid for Progress kind).
    [[nodiscard]] auto Value() const -> int;

private:
    StatusItemKind _kind;
    QWidget*       _widget;
    std::string    _text; // cached for Label kind
};

} // namespace matcha::fw
