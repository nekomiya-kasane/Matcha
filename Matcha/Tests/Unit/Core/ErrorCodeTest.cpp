#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Core/ErrorCode.h>

#include <ostream>
#include <string>

using namespace matcha::fw;

// -- constexpr static_assert verification ---------------------------------- //

static_assert(ToString(ErrorCode::Ok) == "Ok");
static_assert(ToString(ErrorCode::NotFound) == "NotFound");
static_assert(ToString(ErrorCode::AlreadyExists) == "AlreadyExists");
static_assert(ToString(ErrorCode::InvalidArgument) == "InvalidArgument");
static_assert(ToString(ErrorCode::StaleHandle) == "StaleHandle");
static_assert(ToString(ErrorCode::PluginLoadFailed) == "PluginLoadFailed");
static_assert(ToString(ErrorCode::Timeout) == "Timeout");
static_assert(ToString(ErrorCode::AccessDenied) == "AccessDenied");

// -- runtime tests --------------------------------------------------------- //

TEST_CASE("Expected<int> construction with value") {
    Expected<int> result{42};
    CHECK(result.has_value());
    CHECK(result.value() == 42);
}

TEST_CASE("Expected<int> construction with error") {
    Expected<int> result{std::unexpected(ErrorCode::NotFound)};
    CHECK_FALSE(result.has_value());
    CHECK(result.error() == ErrorCode::NotFound);
}

TEST_CASE("Expected monadic chaining") {
    auto doubleIt = [](int v) -> Expected<int> { return v * 2; };
    auto stringify = [](int v) -> std::string { return std::to_string(v); };

    SUBCASE("and_then on value") {
        Expected<int> ok{10};
        auto result = ok.and_then(doubleIt);
        CHECK(result.has_value());
        CHECK(result.value() == 20);
    }

    SUBCASE("and_then on error propagates") {
        Expected<int> err{std::unexpected(ErrorCode::Timeout)};
        auto result = err.and_then(doubleIt);
        CHECK_FALSE(result.has_value());
        CHECK(result.error() == ErrorCode::Timeout);
    }

    SUBCASE("transform on value") {
        Expected<int> ok{7};
        auto result = ok.transform(stringify);
        CHECK(result.has_value());
        CHECK(result.value() == "7");
    }

    SUBCASE("or_else on error") {
        Expected<int> err{std::unexpected(ErrorCode::NotFound)};
        auto result = err.or_else([](ErrorCode) -> Expected<int> { return 0; });
        CHECK(result.has_value());
        CHECK(result.value() == 0);
    }
}

TEST_CASE("ToString covers all ErrorCode values") {
    CHECK(ToString(ErrorCode::Ok) == "Ok");
    CHECK(ToString(ErrorCode::NotFound) == "NotFound");
    CHECK(ToString(ErrorCode::AlreadyExists) == "AlreadyExists");
    CHECK(ToString(ErrorCode::InvalidArgument) == "InvalidArgument");
    CHECK(ToString(ErrorCode::StaleHandle) == "StaleHandle");
    CHECK(ToString(ErrorCode::PluginLoadFailed) == "PluginLoadFailed");
    CHECK(ToString(ErrorCode::Timeout) == "Timeout");
    CHECK(ToString(ErrorCode::AccessDenied) == "AccessDenied");
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
