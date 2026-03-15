#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include <Matcha/Core/Observable.h>
#include <Matcha/Core/PropertyBinding.h>

#include <string>

#include "doctest.h"

using namespace matcha::fw;

// ============================================================================
// Observable<T>
// ============================================================================

TEST_SUITE("Observable") {

TEST_CASE("default constructed value") {
    Observable<int> obs;
    CHECK(obs.Get() == 0);
}

TEST_CASE("initial value") {
    Observable<std::string> obs{"hello"};
    CHECK(obs.Get() == "hello");
}

TEST_CASE("implicit conversion") {
    Observable<int> obs{42};
    const int& ref = obs;
    CHECK(ref == 42);
}

TEST_CASE("Set fires observer with old and new") {
    Observable<int> obs{1};
    int capturedOld = 0;
    int capturedNew = 0;
    auto h = obs.Observe([&](const int& o, const int& n) {
        capturedOld = o;
        capturedNew = n;
    });
    obs.Set(2);
    CHECK(capturedOld == 1);
    CHECK(capturedNew == 2);
    obs.Unobserve(h);
}

TEST_CASE("Set with same value does not fire") {
    Observable<int> obs{5};
    int callCount = 0;
    auto h = obs.Observe([&](const int&, const int&) { ++callCount; });
    obs.Set(5);
    CHECK(callCount == 0);
    obs.Unobserve(h);
}

TEST_CASE("multiple observers") {
    Observable<int> obs{0};
    int a = 0;
    int b = 0;
    auto ha = obs.Observe([&](const int&, const int& n) { a = n; });
    auto hb = obs.Observe([&](const int&, const int& n) { b = n; });
    obs.Set(10);
    CHECK(a == 10);
    CHECK(b == 10);
    obs.Unobserve(ha);
    obs.Unobserve(hb);
}

TEST_CASE("Unobserve removes specific observer") {
    Observable<int> obs{0};
    int a = 0;
    int b = 0;
    auto ha = obs.Observe([&](const int&, const int& n) { a = n; });
    auto hb = obs.Observe([&](const int&, const int& n) { b = n; });
    obs.Unobserve(ha);
    obs.Set(7);
    CHECK(a == 0);
    CHECK(b == 7);
    obs.Unobserve(hb);
}

TEST_CASE("ObserverCount") {
    Observable<int> obs{0};
    CHECK(obs.ObserverCount() == 0);
    auto h1 = obs.Observe([](const int&, const int&) {});
    CHECK(obs.ObserverCount() == 1);
    auto h2 = obs.Observe([](const int&, const int&) {});
    CHECK(obs.ObserverCount() == 2);
    obs.Unobserve(h1);
    CHECK(obs.ObserverCount() == 1);
    obs.Unobserve(h2);
}

TEST_CASE("reentrancy guard — Set inside observer is ignored") {
    Observable<int> obs{0};
    int callCount = 0;
    auto h = obs.Observe([&](const int&, const int&) {
        ++callCount;
        obs.Set(999); // reentrant — should be silently ignored
    });
    obs.Set(1);
    CHECK(callCount == 1);
    CHECK(obs.Get() == 1); // not 999
    obs.Unobserve(h);
}

} // TEST_SUITE

// ============================================================================
// PropertyBinding<T>
// ============================================================================

TEST_SUITE("PropertyBinding") {

TEST_CASE("initial sync on construction") {
    Observable<std::string> src{"init"};
    std::string target;
    PropertyBinding<std::string> binding(src, [&](const std::string& v) { target = v; });
    CHECK(target == "init");
}

TEST_CASE("update propagation") {
    Observable<int> src{0};
    int target = -1;
    PropertyBinding<int> binding(src, [&](const int& v) { target = v; });
    src.Set(42);
    CHECK(target == 42);
}

TEST_CASE("Release stops propagation") {
    Observable<int> src{0};
    int target = 0;
    PropertyBinding<int> binding(src, [&](const int& v) { target = v; });
    binding.Release();
    src.Set(99);
    CHECK(target == 0);
}

TEST_CASE("IsActive") {
    Observable<int> src{0};
    PropertyBinding<int> binding(src, [](const int&) {});
    CHECK(binding.IsActive());
    binding.Release();
    CHECK_FALSE(binding.IsActive());
}

TEST_CASE("move semantics") {
    Observable<int> src{0};
    int target = 0;
    PropertyBinding<int> b1(src, [&](const int& v) { target = v; });
    PropertyBinding<int> b2 = std::move(b1);
    CHECK_FALSE(b1.IsActive()); // NOLINT(bugprone-use-after-move)
    CHECK(b2.IsActive());
    src.Set(5);
    CHECK(target == 5);
}

TEST_CASE("destructor auto-unsubscribes") {
    Observable<int> src{0};
    int target = 0;
    {
        PropertyBinding<int> binding(src, [&](const int& v) { target = v; });
        src.Set(1);
        CHECK(target == 1);
    }
    // binding destroyed
    src.Set(2);
    CHECK(target == 1); // should NOT have updated
    CHECK(src.ObserverCount() == 0);
}

} // TEST_SUITE
