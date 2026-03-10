#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "Matcha/Services/IViewportRenderer.h"

using matcha::fw::InputEvent;
using matcha::fw::InputEventType;
using matcha::fw::KeyModifier;
using matcha::fw::MouseButton;

TEST_CASE("InputEvent: default construction zeroes all fields") {
    InputEvent evt {};
    CHECK(evt.type == InputEventType::MousePress);
    CHECK(evt.button == MouseButton::None);
    CHECK(evt.modifiers == KeyModifier::None);
    CHECK(evt.key == 0);
    CHECK(evt.x == 0.0);
    CHECK(evt.y == 0.0);
    CHECK(evt.globalX == 0.0);
    CHECK(evt.globalY == 0.0);
    CHECK(evt.wheelDelta == 0.0);
    CHECK(evt.pressure == 1.0);
    CHECK(evt.timestamp == 0);
}

TEST_CASE("InputEventType: enum values are distinct") {
    CHECK(InputEventType::MousePress != InputEventType::MouseRelease);
    CHECK(InputEventType::KeyPress != InputEventType::KeyRelease);
    CHECK(InputEventType::Wheel != InputEventType::MouseMove);
    CHECK(InputEventType::TabletPress != InputEventType::MousePress);
}

TEST_CASE("MouseButton: bitwise OR for multiple buttons") {
    auto combined = static_cast<uint8_t>(MouseButton::Left) |
                    static_cast<uint8_t>(MouseButton::Right);
    CHECK((combined & static_cast<uint8_t>(MouseButton::Left)) != 0);
    CHECK((combined & static_cast<uint8_t>(MouseButton::Right)) != 0);
    CHECK((combined & static_cast<uint8_t>(MouseButton::Middle)) == 0);
}

TEST_CASE("KeyModifier: bitwise OR for multiple modifiers") {
    auto combined = static_cast<uint8_t>(KeyModifier::Shift) |
                    static_cast<uint8_t>(KeyModifier::Control) |
                    static_cast<uint8_t>(KeyModifier::Alt);
    CHECK((combined & static_cast<uint8_t>(KeyModifier::Shift)) != 0);
    CHECK((combined & static_cast<uint8_t>(KeyModifier::Control)) != 0);
    CHECK((combined & static_cast<uint8_t>(KeyModifier::Alt)) != 0);
    CHECK((combined & static_cast<uint8_t>(KeyModifier::Meta)) == 0);
}
