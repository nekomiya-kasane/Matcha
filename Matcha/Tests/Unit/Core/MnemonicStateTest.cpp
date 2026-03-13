#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Widgets/Core/MnemonicState.h>

#include <QApplication>
#include <QPainter>
#include <QPixmap>

#include <memory>

using namespace matcha::gui;

// ============================================================================
// Parse tests
// ============================================================================

TEST_SUITE("MnemonicState::Parse") {

TEST_CASE("Simple mnemonic at start") {
    auto r = MnemonicState::Parse("&File");
    CHECK(r.displayText == "File");
    CHECK(r.mnemonicChar == QChar('F'));
    CHECK(r.underlineIndex == 0);
}

TEST_CASE("Mnemonic in middle of text") {
    auto r = MnemonicState::Parse("Save &As...");
    CHECK(r.displayText == "Save As...");
    CHECK(r.mnemonicChar == QChar('A'));
    CHECK(r.underlineIndex == 5);
}

TEST_CASE("Mnemonic on apostrophe word") {
    auto r = MnemonicState::Parse("Do&n't Save");
    CHECK(r.displayText == "Don't Save");
    CHECK(r.mnemonicChar == QChar('n'));
    CHECK(r.underlineIndex == 2);
}

TEST_CASE("Escaped ampersand produces literal") {
    auto r = MnemonicState::Parse("Zoom && Pan");
    CHECK(r.displayText == "Zoom & Pan");
    CHECK(r.mnemonicChar == QChar::Null);
    CHECK(r.underlineIndex == -1);
}

TEST_CASE("No mnemonic in plain text") {
    auto r = MnemonicState::Parse("Exit");
    CHECK(r.displayText == "Exit");
    CHECK(r.mnemonicChar == QChar::Null);
    CHECK(r.underlineIndex == -1);
}

TEST_CASE("Empty string") {
    auto r = MnemonicState::Parse("");
    CHECK(r.displayText == "");
    CHECK(r.mnemonicChar == QChar::Null);
    CHECK(r.underlineIndex == -1);
}

TEST_CASE("Trailing ampersand is dropped") {
    auto r = MnemonicState::Parse("Test&");
    CHECK(r.displayText == "Test");
    CHECK(r.mnemonicChar == QChar::Null);
    CHECK(r.underlineIndex == -1);
}

TEST_CASE("Only first mnemonic is used") {
    auto r = MnemonicState::Parse("&File &Edit");
    CHECK(r.displayText == "File Edit");
    CHECK(r.mnemonicChar == QChar('F'));
    CHECK(r.underlineIndex == 0);
}

TEST_CASE("Escaped ampersand before mnemonic") {
    auto r = MnemonicState::Parse("A && &B");
    CHECK(r.displayText == "A & B");
    CHECK(r.mnemonicChar == QChar('B'));
    CHECK(r.underlineIndex == 4);
}

TEST_CASE("Mnemonic with number") {
    auto r = MnemonicState::Parse("&1 Recent File");
    CHECK(r.displayText == "1 Recent File");
    CHECK(r.mnemonicChar == QChar('1'));
    CHECK(r.underlineIndex == 0);
}

TEST_CASE("CJK appended mnemonic style") {
    auto r = MnemonicState::Parse(QString::fromUtf8("ファイル(&F)"));
    CHECK(r.mnemonicChar == QChar('F'));
    CHECK(r.underlineIndex > 0);
    // displayText should have the parenthesized F without &
    CHECK(r.displayText.contains(QChar('F')));
    CHECK(!r.displayText.contains(QChar('&')));
}

TEST_CASE("Multiple escaped ampersands") {
    auto r = MnemonicState::Parse("A && B && C");
    CHECK(r.displayText == "A & B & C");
    CHECK(r.mnemonicChar == QChar::Null);
    CHECK(r.underlineIndex == -1);
}

} // TEST_SUITE

// ============================================================================
// Visibility state tests
// ============================================================================

TEST_SUITE("MnemonicState visibility") {

TEST_CASE("Default state: underline not shown") {
    MnemonicState state;
    CHECK_FALSE(state.ShouldShowUnderline());
    CHECK_FALSE(state.IsAltHeld());
    CHECK_FALSE(state.IsAlwaysShow());
}

TEST_CASE("SetAltHeld makes underline visible") {
    MnemonicState state;
    state.SetAltHeld(true);
    CHECK(state.ShouldShowUnderline());
    CHECK(state.IsAltHeld());

    state.SetAltHeld(false);
    CHECK_FALSE(state.ShouldShowUnderline());
}

TEST_CASE("SetAlwaysShow overrides Alt state") {
    MnemonicState state;
    state.SetAlwaysShow(true);
    CHECK(state.ShouldShowUnderline());

    // Still visible even if Alt is not held
    CHECK_FALSE(state.IsAltHeld());
    CHECK(state.ShouldShowUnderline());

    // Releasing AlwaysShow when Alt is not held -> hidden
    state.SetAlwaysShow(false);
    CHECK_FALSE(state.ShouldShowUnderline());
}

TEST_CASE("AlwaysShow + AltHeld both true") {
    MnemonicState state;
    state.SetAlwaysShow(true);
    state.SetAltHeld(true);
    CHECK(state.ShouldShowUnderline());

    // Removing Alt still visible due to AlwaysShow
    state.SetAltHeld(false);
    CHECK(state.ShouldShowUnderline());
}

TEST_CASE("UnderlineVisibilityChanged signal emitted") {
    MnemonicState state;
    int signalCount = 0;
    bool lastValue = false;
    QObject::connect(&state, &MnemonicState::UnderlineVisibilityChanged,
                     [&](bool visible) { ++signalCount; lastValue = visible; });

    state.SetAltHeld(true);
    CHECK(signalCount == 1);
    CHECK(lastValue == true);

    state.SetAltHeld(false);
    CHECK(signalCount == 2);
    CHECK(lastValue == false);

    // Setting same value again -> no signal
    state.SetAltHeld(false);
    CHECK(signalCount == 2);
}

TEST_CASE("No signal when AlwaysShow masks Alt change") {
    MnemonicState state;
    state.SetAlwaysShow(true); // signal: false -> true
    int signalCount = 0;
    QObject::connect(&state, &MnemonicState::UnderlineVisibilityChanged,
                     [&](bool) { ++signalCount; });

    // Alt held/released while AlwaysShow=true -> no effective change
    state.SetAltHeld(true);
    CHECK(signalCount == 0);
    state.SetAltHeld(false);
    CHECK(signalCount == 0);
}

TEST_CASE("Alt-tap activated mode makes underline visible") {
    MnemonicState state;
    CHECK_FALSE(state.IsAltActivated());

    state.SetAltActivated(true);
    CHECK(state.IsAltActivated());
    CHECK(state.ShouldShowUnderline());

    // Alt not held, but activated -> still visible
    CHECK_FALSE(state.IsAltHeld());
    CHECK(state.ShouldShowUnderline());

    state.SetAltActivated(false);
    CHECK_FALSE(state.IsAltActivated());
    CHECK_FALSE(state.ShouldShowUnderline());
}

TEST_CASE("Deactivate is shorthand for SetAltActivated(false)") {
    MnemonicState state;
    state.SetAltActivated(true);
    CHECK(state.ShouldShowUnderline());

    state.Deactivate();
    CHECK_FALSE(state.IsAltActivated());
    CHECK_FALSE(state.ShouldShowUnderline());
}

TEST_CASE("AltActivated signal emission") {
    MnemonicState state;
    int signalCount = 0;
    bool lastValue = false;
    QObject::connect(&state, &MnemonicState::UnderlineVisibilityChanged,
                     [&](bool visible) { ++signalCount; lastValue = visible; });

    state.SetAltActivated(true);
    CHECK(signalCount == 1);
    CHECK(lastValue == true);

    state.SetAltActivated(false);
    CHECK(signalCount == 2);
    CHECK(lastValue == false);

    // No signal for same value
    state.SetAltActivated(false);
    CHECK(signalCount == 2);
}

TEST_CASE("AltActivated masked by AlwaysShow") {
    MnemonicState state;
    state.SetAlwaysShow(true);
    int signalCount = 0;
    QObject::connect(&state, &MnemonicState::UnderlineVisibilityChanged,
                     [&](bool) { ++signalCount; });

    // Toggling activated while AlwaysShow=true -> no effective change
    state.SetAltActivated(true);
    CHECK(signalCount == 0);
    state.SetAltActivated(false);
    CHECK(signalCount == 0);
}

TEST_CASE("QueryOsKeyboardCues returns bool without crash") {
    // Just verify it doesn't crash; actual value depends on OS setting
    [[maybe_unused]] bool result = MnemonicState::QueryOsKeyboardCues();
    CHECK(true);
}

} // TEST_SUITE

// ============================================================================
// Global accessor tests
// ============================================================================

TEST_SUITE("MnemonicState global accessor") {

TEST_CASE("Default is nullptr") {
    // Save and restore state in case other tests set it
    auto* saved = GetMnemonicState();
    SetMnemonicState(nullptr);

    CHECK(GetMnemonicState() == nullptr);
    CHECK_FALSE(HasMnemonicState());

    SetMnemonicState(saved);
}

TEST_CASE("Set and get") {
    auto* saved = GetMnemonicState();

    MnemonicState state;
    SetMnemonicState(&state);
    CHECK(GetMnemonicState() == &state);
    CHECK(HasMnemonicState());

    SetMnemonicState(saved);
}

} // TEST_SUITE

// ============================================================================
// DrawMnemonicText smoke test
// ============================================================================

TEST_SUITE("MnemonicState::DrawMnemonicText") {

TEST_CASE("DrawMnemonicText does not crash") {
    // QPixmap requires QApplication — create one if not yet present
    int argc = 0;
    std::unique_ptr<QApplication> tempApp;
    if (QApplication::instance() == nullptr) {
        tempApp = std::make_unique<QApplication>(argc, nullptr);
    }

    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setFont(QFont("Arial", 12));
    painter.setPen(Qt::black);

    QRect rect(0, 0, 200, 30);

    // With underline
    MnemonicState::DrawMnemonicText(painter, rect,
                                     Qt::AlignLeft | Qt::AlignVCenter,
                                     "&File", true);

    // Without underline
    MnemonicState::DrawMnemonicText(painter, rect,
                                     Qt::AlignLeft | Qt::AlignVCenter,
                                     "&Edit", false);

    // No mnemonic text
    MnemonicState::DrawMnemonicText(painter, rect,
                                     Qt::AlignCenter,
                                     "Plain Text", true);

    // Escaped ampersand
    MnemonicState::DrawMnemonicText(painter, rect,
                                     Qt::AlignRight | Qt::AlignVCenter,
                                     "A && B", true);

    painter.end();
    CHECK(true); // If we got here, no crash
}

} // TEST_SUITE
