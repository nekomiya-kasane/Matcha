#pragma once

/**
 * @file WidgetTestFixture.h
 * @brief Headless QApplication bootstrap and widget factory for GUI tests.
 */

#include <QApplication>
#include <QTest>
#include <QWidget>

#include <memory>
#include <vector>

namespace matcha::test {

/**
 * @brief RAII fixture that boots a headless QApplication for widget tests.
 *
 * Usage: instantiate once in your test main() before running tests.
 * Automatically sets QT_QPA_PLATFORM=offscreen and suppresses animations.
 */
class WidgetTestFixture {
public:
    WidgetTestFixture() {
        // Force offscreen rendering for CI headless
        qputenv("QT_QPA_PLATFORM", "offscreen");

        // QApplication requires argc to remain valid for its lifetime
        static int argc = 1;
        static char appName[] = "MatchaWidgetTests";
        static char* argv[] = {appName, nullptr};

        _app = std::make_unique<QApplication>(argc, argv);

        // Suppress all animations for deterministic test timing
        _app->setProperty("animationDuration", 0);
    }

    ~WidgetTestFixture() {
        _trackedWidgets.clear();
        _app.reset();
    }

    WidgetTestFixture(const WidgetTestFixture&) = delete;
    auto operator=(const WidgetTestFixture&) -> WidgetTestFixture& = delete;
    WidgetTestFixture(WidgetTestFixture&&) = delete;
    auto operator=(WidgetTestFixture&&) -> WidgetTestFixture& = delete;

    /**
     * @brief Create a widget of type T with automatic cleanup.
     * @tparam T Widget type (must derive from QWidget).
     * @tparam Args Constructor argument types.
     * @param args Forwarded to T's constructor.
     * @return Raw pointer to the created widget (owned by this fixture).
     */
    template <typename T = QWidget, typename... Args>
    auto CreateWidget(Args&&... args) -> T* {
        auto widget = std::make_unique<T>(std::forward<Args>(args)...);
        auto* ptr = widget.get();
        _trackedWidgets.push_back(std::move(widget));
        return ptr;
    }

    /** @brief Access the QApplication instance. */
    [[nodiscard]] auto App() const -> QApplication* { return _app.get(); }

private:
    std::unique_ptr<QApplication> _app;
    std::vector<std::unique_ptr<QWidget>> _trackedWidgets;
};

} // namespace matcha::test
