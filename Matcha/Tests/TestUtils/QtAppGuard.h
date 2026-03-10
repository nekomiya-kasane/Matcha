#pragma once

/**
 * @file QtAppGuard.h
 * @brief Ensures exactly one QApplication exists for the test process lifetime.
 *
 * Usage: call QtAppGuard::Ensure() at the top of any test case that creates
 * QWidgets. The QApplication is created lazily on first call and persists
 * until process exit. This avoids the Qt assertion
 * "there should be only one application object".
 */

#include <QApplication>

#include <memory>

namespace matcha::test {

struct QtAppGuard {
    static void Ensure()
    {
        if (QApplication::instance() == nullptr) {
            static int argc = 0;
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
            static auto app = std::make_unique<QApplication>(argc, nullptr);
        }
    }
};

} // namespace matcha::test
