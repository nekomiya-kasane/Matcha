#pragma once

/**
 * @file DateTimePickerNode.h
 * @brief Typed WidgetNode wrapping NyanDateTimePicker for UiNode tree.
 */

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <cstdint>
#include <string>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanDateTimePicker (Scheme D typed node).
 *
 * Date/time input supporting Date-only, Time-only, and DateTime modes.
 * Value is exposed as milliseconds since epoch (int64_t) to avoid
 * leaking QDateTime across the API boundary.
 *
 * Dispatches: DateTimeChanged (msecsSinceEpoch).
 */
class MATCHA_EXPORT DateTimePickerNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using DateTimeChanged = matcha::fw::DateTimeChanged;
    };

    explicit DateTimePickerNode(std::string id);
    ~DateTimePickerNode() override;

    DateTimePickerNode(const DateTimePickerNode&) = delete;
    auto operator=(const DateTimePickerNode&) -> DateTimePickerNode& = delete;
    DateTimePickerNode(DateTimePickerNode&&) = delete;
    auto operator=(DateTimePickerNode&&) -> DateTimePickerNode& = delete;

    void SetMode(uint8_t mode);
    [[nodiscard]] auto Mode() const -> uint8_t;

    void SetDateTimeMsecs(int64_t msecsSinceEpoch);
    [[nodiscard]] auto DateTimeMsecs() const -> int64_t;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
