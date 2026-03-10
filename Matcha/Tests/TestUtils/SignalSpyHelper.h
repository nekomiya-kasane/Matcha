#pragma once

/**
 * @file SignalSpyHelper.h
 * @brief QSignalSpy convenience wrappers for widget tests.
 */

#include <QSignalSpy>
#include <QTest>

namespace matcha::test {

/**
 * @brief Wait for a signal to be emitted within a timeout.
 * @param spy QSignalSpy watching the signal.
 * @param timeoutMs Maximum wait time in milliseconds.
 * @return true if the signal was emitted before timeout.
 */
[[nodiscard]] inline auto WaitForSignal(QSignalSpy& spy, int timeoutMs = 5000) -> bool {
    if (spy.count() > 0) {
        return true;
    }
    return QTest::qWaitFor([&spy]() { return spy.count() > 0; }, timeoutMs);
}

/**
 * @brief Assert that a signal was emitted exactly `expectedCount` times.
 * @param spy QSignalSpy watching the signal.
 * @param expectedCount Expected number of emissions.
 * @return true if count matches.
 */
[[nodiscard]] inline auto AssertSignalCount(const QSignalSpy& spy, int expectedCount) -> bool {
    return spy.count() == expectedCount;
}

/**
 * @brief Assert that no signal was emitted (after a short wait).
 * @param spy QSignalSpy watching the signal.
 * @param waitMs Time to wait before checking (default 100ms).
 * @return true if no signal was emitted.
 */
[[nodiscard]] inline auto AssertNoSignal(QSignalSpy& spy, int waitMs = 100) -> bool {
    (void)QTest::qWaitFor([&spy]() { return spy.count() > 0; }, waitMs);
    return spy.count() == 0;
}

} // namespace matcha::test
