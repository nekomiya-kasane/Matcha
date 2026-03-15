#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Core/StrongId.h>

#include <unordered_map>

using namespace matcha::fw;

// -- constexpr static_assert verification ---------------------------------- //

static_assert(DocumentId::From(42).value == 42);
static_assert(DocumentId::From(1) == DocumentId::From(1));
static_assert(DocumentId::From(1) != DocumentId::From(2));
static_assert(DocumentId::From(1) < DocumentId::From(2));

static_assert(GetType(MakeWidgetId(0xAB, 0x1234, 0x5678)) == 0xAB);
static_assert(GetGeneration(MakeWidgetId(0xAB, 0x1234, 0x5678)) == 0x1234);
static_assert(GetIndex(MakeWidgetId(0xAB, 0x1234, 0x5678)) == 0x5678);

// -- runtime tests --------------------------------------------------------- //

TEST_CASE("StrongId::From constructs with correct value") {
    auto id = DocumentId::From(100);
    CHECK(id.value == 100);
}

TEST_CASE("StrongId comparison operators") {
    auto a = ViewportId::From(10);
    auto b = ViewportId::From(20);
    auto c = ViewportId::From(10);

    CHECK(a == c);
    CHECK(a != b);
    CHECK(a < b);
    CHECK(b > a);
    CHECK(a <= c);
    CHECK(b >= a);
}

TEST_CASE("StrongId::Hash produces consistent results") {
    DocumentId::Hash hasher;
    auto id1 = DocumentId::From(42);
    auto id2 = DocumentId::From(42);
    auto id3 = DocumentId::From(99);

    CHECK(hasher(id1) == hasher(id2));
    CHECK(hasher(id1) != hasher(id3));

    std::unordered_map<DocumentId, int, DocumentId::Hash> map;
    map[id1] = 7;
    CHECK(map[id2] == 7);
}

TEST_CASE("StrongId cross-type safety") {
    // DocumentId and ViewportId are distinct types — this is a compile-time check.
    // The following must NOT compile:
    //   DocumentId d = ViewportId::From(1); // ❌
    // We verify the types are indeed different:
    CHECK_FALSE((std::is_same_v<DocumentId, ViewportId>));
    CHECK_FALSE((std::is_same_v<DocumentId, WidgetId>));
    CHECK_FALSE((std::is_same_v<ViewportId, WidgetId>));
}

TEST_CASE("WidgetId pack/unpack round-trip") {
    constexpr uint8_t type = 0xFF;
    constexpr uint16_t gen = 0xBEEF;
    constexpr uint64_t idx = 0xDEADCAFE;

    auto wid = MakeWidgetId(type, gen, idx);

    CHECK(GetType(wid) == type);
    CHECK(GetGeneration(wid) == gen);
    CHECK(GetIndex(wid) == idx);
}

TEST_CASE("WidgetId pack/unpack boundary values") {
    auto zero = MakeWidgetId(0, 0, 0);
    CHECK(GetType(zero) == 0);
    CHECK(GetGeneration(zero) == 0);
    CHECK(GetIndex(zero) == 0);

    auto max = MakeWidgetId(0xFF, 0xFFFF, kWidgetIdIndexMask);
    CHECK(GetType(max) == 0xFF);
    CHECK(GetGeneration(max) == 0xFFFF);
    CHECK(GetIndex(max) == kWidgetIdIndexMask);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
