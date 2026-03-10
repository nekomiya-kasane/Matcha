#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Foundation/Types.h>

#include <memory>

using namespace matcha::fw;

TEST_CASE("observer_ptr construction from raw pointer") {
    int x = 42;
    observer_ptr<int> p{&x};
    CHECK(p.get() == &x);
    CHECK(static_cast<bool>(p));
}

TEST_CASE("observer_ptr default and nullptr construction") {
    observer_ptr<int> def;
    CHECK(def.get() == nullptr);
    CHECK_FALSE(static_cast<bool>(def));

    observer_ptr<int> null{nullptr};
    CHECK(null.get() == nullptr);
}

TEST_CASE("observer_ptr construction from unique_ptr") {
    auto uptr = std::make_unique<int>(99);
    observer_ptr<int> obs = uptr;
    CHECK(obs.get() == uptr.get());
    CHECK(*obs == 99);
}

TEST_CASE("observer_ptr accessors") {
    struct Widget {
        int id = 7;
    };

    Widget w;
    observer_ptr<Widget> p{&w};
    CHECK((*p).id == 7);
    CHECK(p->id == 7);
    CHECK(p.get() == &w);
}

TEST_CASE("observer_ptr reset and swap") {
    int a = 1;
    int b = 2;
    observer_ptr<int> pa{&a};
    observer_ptr<int> pb{&b};

    pa.swap(pb);
    CHECK(pa.get() == &b);
    CHECK(pb.get() == &a);

    pa.reset();
    CHECK(pa.get() == nullptr);

    pa.reset(&a);
    CHECK(pa.get() == &a);
}

TEST_CASE("observer_ptr comparison") {
    int x = 0;
    int y = 0;
    observer_ptr<int> px{&x};
    observer_ptr<int> py{&y};
    observer_ptr<int> px2{&x};

    CHECK(px == px2);
    CHECK(px != py);

    observer_ptr<int> null;
    CHECK(null == observer_ptr<int>{});
}

TEST_CASE("make_observer factory") {
    int val = 123;
    auto obs = make_observer(&val);
    CHECK(obs.get() == &val);
    CHECK(*obs == 123);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
