#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file DtcgTokenModelTest.cpp
 * @brief Unit tests for DtcgTokenModel and DtcgSerializer (E1).
 */

#include "doctest.h"

#include <Matcha/Theming/Token/DtcgTokenModel.h>

using namespace matcha::fw;

TEST_SUITE("DtcgTokenModel") {

// ============================================================================
// Type string conversion
// ============================================================================

TEST_CASE("DtcgTypeToString round-trip") {
    CHECK(DtcgTypeToString(DtfmType::Color) == "color");
    CHECK(DtcgTypeToString(DtfmType::Dimension) == "dimension");
    CHECK(DtcgTypeToString(DtfmType::Transition) == "transition");
    CHECK(DtcgTypeToString(DtfmType::FontFamily) == "fontFamily");
    CHECK(DtcgTypeToString(DtfmType::FontWeight) == "fontWeight");
    CHECK(DtcgTypeToString(DtfmType::Number) == "number");
    CHECK(DtcgTypeToString(DtfmType::String) == "string");
}

TEST_CASE("DtcgTypeFromString valid values") {
    CHECK(DtcgTypeFromString("color") == DtfmType::Color);
    CHECK(DtcgTypeFromString("dimension") == DtfmType::Dimension);
    CHECK(DtcgTypeFromString("transition") == DtfmType::Transition);
    CHECK(DtcgTypeFromString("fontFamily") == DtfmType::FontFamily);
    CHECK(DtcgTypeFromString("fontWeight") == DtfmType::FontWeight);
    CHECK(DtcgTypeFromString("number") == DtfmType::Number);
    CHECK(DtcgTypeFromString("string") == DtfmType::String);
}

TEST_CASE("DtcgTypeFromString invalid returns nullopt") {
    CHECK_FALSE(DtcgTypeFromString("invalid").has_value());
    CHECK_FALSE(DtcgTypeFromString("").has_value());
}

// ============================================================================
// Export
// ============================================================================

TEST_CASE("Export simple color token") {
    DtfmTokenFile file;
    file.schema = "https://www.designtokens.org/schemas/2025.10/format.json";

    DtfmTokenGroup colorGroup;
    colorGroup.name = "color";
    colorGroup.tokens.push_back({
        .name = "primary",
        .type = DtfmType::Color,
        .value = std::string("#5b6abf"),
        .description = "Brand primary",
        .aliasRef = {},
    });
    file.groups.push_back(std::move(colorGroup));

    std::string json = DtcgSerializer::Export(file, 0);
    CHECK(json.find("\"$type\":\"color\"") != std::string::npos);
    CHECK(json.find("\"$value\":\"#5b6abf\"") != std::string::npos);
    CHECK(json.find("\"$description\":\"Brand primary\"") != std::string::npos);
}

TEST_CASE("Export dimension token") {
    DtfmTokenFile file;
    DtfmTokenGroup spacingGroup;
    spacingGroup.name = "spacing";
    spacingGroup.tokens.push_back({
        .name = "px4",
        .type = DtfmType::Dimension,
        .value = DtfmDimension{.value = 4.0, .unit = "px"},
        .description = {},
        .aliasRef = {},
    });
    file.groups.push_back(std::move(spacingGroup));

    std::string json = DtcgSerializer::Export(file, 0);
    CHECK(json.find("\"$type\":\"dimension\"") != std::string::npos);
    CHECK(json.find("\"value\":4") != std::string::npos);
    CHECK(json.find("\"unit\":\"px\"") != std::string::npos);
}

TEST_CASE("Export alias token") {
    DtfmTokenFile file;
    DtfmTokenGroup colorGroup;
    colorGroup.name = "color";
    colorGroup.tokens.push_back({
        .name = "surface",
        .type = DtfmType::Color,
        .value = std::monostate{},
        .description = "alias to neutral.50",
        .aliasRef = "color.neutral.50",
    });
    file.groups.push_back(std::move(colorGroup));

    std::string json = DtcgSerializer::Export(file, 0);
    CHECK(json.find("\"{color.neutral.50}\"") != std::string::npos);
}

// ============================================================================
// Import
// ============================================================================

TEST_CASE("Import simple color token") {
    const char* json = R"({
        "$schema": "https://www.designtokens.org/schemas/2025.10/format.json",
        "color": {
            "primary": {
                "$type": "color",
                "$value": "#5b6abf",
                "$description": "Brand primary"
            }
        }
    })";

    auto result = DtcgSerializer::Import(json);
    REQUIRE(result.has_value());
    CHECK(result->schema == "https://www.designtokens.org/schemas/2025.10/format.json");
    REQUIRE(result->groups.size() == 1);
    CHECK(result->groups[0].name == "color");
    REQUIRE(result->groups[0].tokens.size() == 1);

    const auto& token = result->groups[0].tokens[0];
    CHECK(token.name == "primary");
    CHECK(token.type == DtfmType::Color);
    CHECK(std::get<std::string>(token.value) == "#5b6abf");
    CHECK(token.description == "Brand primary");
}

TEST_CASE("Import dimension token") {
    const char* json = R"({
        "spacing": {
            "px8": {
                "$type": "dimension",
                "$value": {"value": 8, "unit": "px"}
            }
        }
    })";

    auto result = DtcgSerializer::Import(json);
    REQUIRE(result.has_value());
    REQUIRE(result->groups.size() == 1);
    REQUIRE(result->groups[0].tokens.size() == 1);

    const auto& token = result->groups[0].tokens[0];
    CHECK(token.type == DtfmType::Dimension);
    auto dim = std::get<DtfmDimension>(token.value);
    CHECK(dim.value == doctest::Approx(8.0));
    CHECK(dim.unit == "px");
}

TEST_CASE("Import alias reference") {
    const char* json = R"({
        "color": {
            "surface": {
                "$type": "color",
                "$value": "{color.neutral.50}"
            }
        }
    })";

    auto result = DtcgSerializer::Import(json);
    REQUIRE(result.has_value());
    REQUIRE(result->groups[0].tokens.size() == 1);

    const auto& token = result->groups[0].tokens[0];
    CHECK(token.IsAlias());
    CHECK(token.aliasRef == "color.neutral.50");
}

TEST_CASE("Import transition token") {
    const char* json = R"({
        "transition": {
            "default": {
                "$type": "transition",
                "$value": {
                    "duration": {"value": 150, "unit": "ms"},
                    "delay": {"value": 0, "unit": "ms"},
                    "timingFunction": [0.33, 0, 0.67, 1]
                }
            }
        }
    })";

    auto result = DtcgSerializer::Import(json);
    REQUIRE(result.has_value());
    REQUIRE(result->groups[0].tokens.size() == 1);

    const auto& token = result->groups[0].tokens[0];
    CHECK(token.type == DtfmType::Transition);
    auto tr = std::get<DtfmTransition>(token.value);
    CHECK(tr.duration == doctest::Approx(150.0));
    CHECK(tr.delay == doctest::Approx(0.0));
    REQUIRE(tr.timingFunction.size() == 4);
    CHECK(tr.timingFunction[0] == doctest::Approx(0.33));
}

TEST_CASE("Import invalid JSON returns nullopt") {
    CHECK_FALSE(DtcgSerializer::Import("{invalid}").has_value());
    CHECK_FALSE(DtcgSerializer::Import("").has_value());
}

// ============================================================================
// Flatten
// ============================================================================

TEST_CASE("Flatten produces dot-separated paths") {
    DtfmTokenFile file;
    DtfmTokenGroup colorGroup;
    colorGroup.name = "color";
    colorGroup.tokens.push_back({
        .name = "primary",
        .type = DtfmType::Color,
        .value = std::string("#5b6abf"),
        .description = {},
        .aliasRef = {},
    });

    DtfmTokenGroup neutralSub;
    neutralSub.name = "neutral";
    neutralSub.tokens.push_back({
        .name = "50",
        .type = DtfmType::Color,
        .value = std::string("#fafafa"),
        .description = {},
        .aliasRef = {},
    });
    colorGroup.children.push_back(std::move(neutralSub));
    file.groups.push_back(std::move(colorGroup));

    auto flat = DtcgSerializer::Flatten(file);
    CHECK(flat.count("color.primary") == 1);
    CHECK(flat.count("color.neutral.50") == 1);
    CHECK(std::get<std::string>(flat["color.primary"].value) == "#5b6abf");
}

// ============================================================================
// Round-trip: Export -> Import
// ============================================================================

TEST_CASE("Export then Import round-trip preserves tokens") {
    DtfmTokenFile original;
    original.schema = "https://www.designtokens.org/schemas/2025.10/format.json";

    DtfmTokenGroup colorGroup;
    colorGroup.name = "color";
    colorGroup.tokens.push_back({
        .name = "primary",
        .type = DtfmType::Color,
        .value = std::string("#5b6abf"),
        .description = "Brand primary",
        .aliasRef = {},
    });
    original.groups.push_back(std::move(colorGroup));

    std::string json = DtcgSerializer::Export(original, 2);
    auto reimported = DtcgSerializer::Import(json);
    REQUIRE(reimported.has_value());
    CHECK(reimported->schema == original.schema);
    REQUIRE(reimported->groups.size() == 1);
    REQUIRE(reimported->groups[0].tokens.size() == 1);
    CHECK(reimported->groups[0].tokens[0].name == "primary");
    CHECK(std::get<std::string>(reimported->groups[0].tokens[0].value) == "#5b6abf");
}

} // TEST_SUITE
