#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file VariantNameRegistryTest.cpp
 * @brief Unit tests for VariantNameRegistry (E2).
 */

#include "doctest.h"

#include <Matcha/Foundation/VariantNameRegistry.h>

using namespace matcha::fw;

TEST_SUITE("VariantNameRegistry") {

TEST_CASE("RegisterKind and IndexOf") {
    VariantNameRegistry reg;
    reg.RegisterKind("PushButton", {"primary", "secondary", "ghost", "danger"});

    CHECK(reg.IndexOf("PushButton", "primary") == 0);
    CHECK(reg.IndexOf("PushButton", "secondary") == 1);
    CHECK(reg.IndexOf("PushButton", "ghost") == 2);
    CHECK(reg.IndexOf("PushButton", "danger") == 3);
}

TEST_CASE("IndexOf returns nullopt for unknown variant") {
    VariantNameRegistry reg;
    reg.RegisterKind("PushButton", {"primary", "secondary"});
    CHECK_FALSE(reg.IndexOf("PushButton", "nonexistent").has_value());
}

TEST_CASE("IndexOf returns nullopt for unknown kind") {
    VariantNameRegistry reg;
    CHECK_FALSE(reg.IndexOf("Unknown", "primary").has_value());
}

TEST_CASE("NameOf returns variant name by index") {
    VariantNameRegistry reg;
    reg.RegisterKind("LineEdit", {"default", "search", "password"});

    CHECK(reg.NameOf("LineEdit", 0) == "default");
    CHECK(reg.NameOf("LineEdit", 1) == "search");
    CHECK(reg.NameOf("LineEdit", 2) == "password");
}

TEST_CASE("NameOf returns nullopt for out-of-range index") {
    VariantNameRegistry reg;
    reg.RegisterKind("LineEdit", {"default"});

    CHECK_FALSE(reg.NameOf("LineEdit", 1).has_value());
    CHECK_FALSE(reg.NameOf("LineEdit", -1).has_value());
}

TEST_CASE("NameOf returns nullopt for unknown kind") {
    VariantNameRegistry reg;
    CHECK_FALSE(reg.NameOf("Unknown", 0).has_value());
}

TEST_CASE("VariantNames returns pointer to name list") {
    VariantNameRegistry reg;
    reg.RegisterKind("ComboBox", {"outlined", "filled"});

    const auto* names = reg.VariantNames("ComboBox");
    REQUIRE(names != nullptr);
    REQUIRE(names->size() == 2);
    CHECK((*names)[0] == "outlined");
    CHECK((*names)[1] == "filled");
}

TEST_CASE("VariantNames returns nullptr for unknown kind") {
    VariantNameRegistry reg;
    CHECK(reg.VariantNames("Unknown") == nullptr);
}

TEST_CASE("VariantCount") {
    VariantNameRegistry reg;
    reg.RegisterKind("PushButton", {"primary", "secondary", "ghost", "danger"});

    CHECK(reg.VariantCount("PushButton") == 4);
    CHECK(reg.VariantCount("Unknown") == 0);
}

TEST_CASE("HasKind") {
    VariantNameRegistry reg;
    reg.RegisterKind("PushButton", {"primary"});

    CHECK(reg.HasKind("PushButton"));
    CHECK_FALSE(reg.HasKind("Unknown"));
}

TEST_CASE("RegisteredKinds") {
    VariantNameRegistry reg;
    reg.RegisterKind("PushButton", {"primary"});
    reg.RegisterKind("LineEdit", {"default"});

    auto kinds = reg.RegisteredKinds();
    CHECK(kinds.size() == 2);
}

TEST_CASE("Re-register replaces previous") {
    VariantNameRegistry reg;
    reg.RegisterKind("PushButton", {"old1", "old2"});
    CHECK(reg.VariantCount("PushButton") == 2);

    reg.RegisterKind("PushButton", {"new1", "new2", "new3"});
    CHECK(reg.VariantCount("PushButton") == 3);
    CHECK(reg.IndexOf("PushButton", "new1") == 0);
    CHECK_FALSE(reg.IndexOf("PushButton", "old1").has_value());
}

TEST_CASE("UnregisterKind") {
    VariantNameRegistry reg;
    reg.RegisterKind("PushButton", {"primary"});

    CHECK(reg.UnregisterKind("PushButton"));
    CHECK_FALSE(reg.HasKind("PushButton"));
    CHECK_FALSE(reg.UnregisterKind("PushButton")); // already removed
}

TEST_CASE("Clear removes all") {
    VariantNameRegistry reg;
    reg.RegisterKind("PushButton", {"primary"});
    reg.RegisterKind("LineEdit", {"default"});

    reg.Clear();
    CHECK_FALSE(reg.HasKind("PushButton"));
    CHECK_FALSE(reg.HasKind("LineEdit"));
    CHECK(reg.RegisteredKinds().empty());
}

} // TEST_SUITE
