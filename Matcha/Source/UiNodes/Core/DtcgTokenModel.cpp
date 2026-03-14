/**
 * @file DtcgTokenModel.cpp
 * @brief Implementation of DTCG token model and serializer.
 */

#include <Matcha/Foundation/DtcgTokenModel.h>

#include <charconv>
#include <format>
#include <sstream>

namespace matcha::fw {

// ============================================================================
// DtcgType string conversion
// ============================================================================

auto DtcgTypeToString(DtcgType t) -> std::string
{
    switch (t) {
    case DtcgType::Color:      return "color";
    case DtcgType::Dimension:  return "dimension";
    case DtcgType::Transition: return "transition";
    case DtcgType::FontFamily: return "fontFamily";
    case DtcgType::FontWeight: return "fontWeight";
    case DtcgType::Number:     return "number";
    case DtcgType::String:     return "string";
    }
    return "string";
}

auto DtcgTypeFromString(std::string_view s) -> std::optional<DtcgType>
{
    if (s == "color")      return DtcgType::Color;
    if (s == "dimension")  return DtcgType::Dimension;
    if (s == "transition") return DtcgType::Transition;
    if (s == "fontFamily") return DtcgType::FontFamily;
    if (s == "fontWeight") return DtcgType::FontWeight;
    if (s == "number")     return DtcgType::Number;
    if (s == "string")     return DtcgType::String;
    return std::nullopt;
}

// ============================================================================
// JSON Export helpers
// ============================================================================

namespace {

void WriteIndent(std::ostringstream& os, int level, int indent)
{
    if (indent <= 0) { return; }
    for (int i = 0; i < level * indent; ++i) {
        os << ' ';
    }
}

void WriteString(std::ostringstream& os, std::string_view s)
{
    os << '"';
    for (char c : s) {
        switch (c) {
        case '"':  os << "\\\""; break;
        case '\\': os << "\\\\"; break;
        case '\n': os << "\\n";  break;
        case '\r': os << "\\r";  break;
        case '\t': os << "\\t";  break;
        default:   os << c;      break;
        }
    }
    os << '"';
}

void WriteNumber(std::ostringstream& os, double v)
{
    if (v == static_cast<int>(v) && v >= -1e9 && v <= 1e9) {
        os << static_cast<int>(v);
    } else {
        os << std::format("{:.6g}", v);
    }
}

auto Nl(int indent) -> std::string_view
{
    return (indent > 0) ? "\n" : "";
}

auto Sep(int indent) -> std::string_view
{
    return (indent > 0) ? " " : "";
}

void WriteValue(std::ostringstream& os, const DtcgToken& token, int level, int indent);

void WriteDimensionValue(std::ostringstream& os, const DtcgDimension& dim,
                         int level, int indent)
{
    os << '{' << Nl(indent);
    WriteIndent(os, level + 1, indent);
    os << "\"value\":" << Sep(indent);
    WriteNumber(os, dim.value);
    os << ',' << Nl(indent);
    WriteIndent(os, level + 1, indent);
    os << "\"unit\":" << Sep(indent);
    WriteString(os, dim.unit);
    os << Nl(indent);
    WriteIndent(os, level, indent);
    os << '}';
}

void WriteTransitionValue(std::ostringstream& os, const DtcgTransition& tr,
                          int level, int indent)
{
    os << '{' << Nl(indent);
    WriteIndent(os, level + 1, indent);
    os << "\"duration\":" << Sep(indent) << '{';
    os << "\"value\":" << Sep(indent);
    WriteNumber(os, tr.duration);
    os << ',' << Sep(indent) << "\"unit\":" << Sep(indent) << "\"ms\"}";
    os << ',' << Nl(indent);

    WriteIndent(os, level + 1, indent);
    os << "\"delay\":" << Sep(indent) << '{';
    os << "\"value\":" << Sep(indent);
    WriteNumber(os, tr.delay);
    os << ',' << Sep(indent) << "\"unit\":" << Sep(indent) << "\"ms\"}";
    os << ',' << Nl(indent);

    WriteIndent(os, level + 1, indent);
    os << "\"timingFunction\":" << Sep(indent) << '[';
    for (size_t i = 0; i < tr.timingFunction.size(); ++i) {
        if (i > 0) { os << ',' << Sep(indent); }
        WriteNumber(os, tr.timingFunction[i]);
    }
    os << ']' << Nl(indent);
    WriteIndent(os, level, indent);
    os << '}';
}

void WriteValue(std::ostringstream& os, const DtcgToken& token, int level, int indent)
{
    if (token.IsAlias()) {
        WriteString(os, std::format("{{{}}}", token.aliasRef));
        return;
    }

    if (std::holds_alternative<std::string>(token.value)) {
        WriteString(os, std::get<std::string>(token.value));
    } else if (std::holds_alternative<DtcgDimension>(token.value)) {
        WriteDimensionValue(os, std::get<DtcgDimension>(token.value), level, indent);
    } else if (std::holds_alternative<DtcgTransition>(token.value)) {
        WriteTransitionValue(os, std::get<DtcgTransition>(token.value), level, indent);
    } else if (std::holds_alternative<double>(token.value)) {
        WriteNumber(os, std::get<double>(token.value));
    } else {
        os << "null";
    }
}

void WriteToken(std::ostringstream& os, const DtcgToken& token,
                int level, int indent)
{
    WriteIndent(os, level, indent);
    WriteString(os, token.name);
    os << ':' << Sep(indent) << '{' << Nl(indent);

    WriteIndent(os, level + 1, indent);
    os << "\"$type\":" << Sep(indent);
    WriteString(os, DtcgTypeToString(token.type));
    os << ',' << Nl(indent);

    WriteIndent(os, level + 1, indent);
    os << "\"$value\":" << Sep(indent);
    WriteValue(os, token, level + 1, indent);

    if (!token.description.empty()) {
        os << ',' << Nl(indent);
        WriteIndent(os, level + 1, indent);
        os << "\"$description\":" << Sep(indent);
        WriteString(os, token.description);
    }

    os << Nl(indent);
    WriteIndent(os, level, indent);
    os << '}';
}

void WriteGroup(std::ostringstream& os, const DtcgTokenGroup& group,
                int level, int indent)
{
    WriteIndent(os, level, indent);
    WriteString(os, group.name);
    os << ':' << Sep(indent) << '{' << Nl(indent);

    bool first = true;
    for (const auto& token : group.tokens) {
        if (!first) { os << ',' << Nl(indent); }
        first = false;
        WriteToken(os, token, level + 1, indent);
    }
    for (const auto& child : group.children) {
        if (!first) { os << ',' << Nl(indent); }
        first = false;
        WriteGroup(os, child, level + 1, indent);
    }

    os << Nl(indent);
    WriteIndent(os, level, indent);
    os << '}';
}

// ============================================================================
// Minimal JSON parser (recursive descent)
// ============================================================================

struct JsonParser {
    std::string_view src;
    size_t pos = 0;

    void SkipWs()
    {
        while (pos < src.size() && (src[pos] == ' ' || src[pos] == '\t' ||
               src[pos] == '\n' || src[pos] == '\r')) {
            ++pos;
        }
    }

    auto Peek() -> char
    {
        SkipWs();
        return (pos < src.size()) ? src[pos] : '\0';
    }

    auto Consume(char c) -> bool
    {
        SkipWs();
        if (pos < src.size() && src[pos] == c) {
            ++pos;
            return true;
        }
        return false;
    }

    auto ParseString() -> std::optional<std::string>
    {
        SkipWs();
        if (pos >= src.size() || src[pos] != '"') { return std::nullopt; }
        ++pos;
        std::string result;
        while (pos < src.size() && src[pos] != '"') {
            if (src[pos] == '\\') {
                ++pos;
                if (pos >= src.size()) { return std::nullopt; }
                switch (src[pos]) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                default:   result += src[pos]; break;
                }
            } else {
                result += src[pos];
            }
            ++pos;
        }
        if (pos >= src.size()) { return std::nullopt; }
        ++pos; // closing quote
        return result;
    }

    auto ParseNumber() -> std::optional<double>
    {
        SkipWs();
        const auto start = pos;
        // Optional leading minus
        if (pos < src.size() && src[pos] == '-') { ++pos; }
        // Integer part (at least one digit required)
        bool hasDigit = false;
        while (pos < src.size() && src[pos] >= '0' && src[pos] <= '9') {
            ++pos;
            hasDigit = true;
        }
        // Fractional part
        if (pos < src.size() && src[pos] == '.') {
            ++pos;
            while (pos < src.size() && src[pos] >= '0' && src[pos] <= '9') {
                ++pos;
                hasDigit = true;
            }
        }
        // Exponent part
        if (pos < src.size() && (src[pos] == 'e' || src[pos] == 'E')) {
            ++pos;
            if (pos < src.size() && (src[pos] == '+' || src[pos] == '-')) { ++pos; }
            while (pos < src.size() && src[pos] >= '0' && src[pos] <= '9') { ++pos; }
        }
        if (!hasDigit) { pos = start; return std::nullopt; }
        double val = 0.0;
        auto [ptr, ec] = std::from_chars(src.data() + start, src.data() + pos, val);
        if (ec != std::errc{}) { pos = start; return std::nullopt; }
        pos = static_cast<size_t>(ptr - src.data());
        return val;
    }

    auto ParseArray() -> std::optional<std::vector<double>>
    {
        if (!Consume('[')) { return std::nullopt; }
        std::vector<double> result;
        if (Peek() == ']') { ++pos; return result; }
        while (true) {
            auto num = ParseNumber();
            if (!num) { return std::nullopt; }
            result.push_back(*num);
            if (!Consume(',')) { break; }
        }
        if (!Consume(']')) { return std::nullopt; }
        return result;
    }

    // Parse a JSON object as key-value pairs (values are strings, numbers, objects, or arrays)
    struct JsonValue;
    struct JsonObject {
        std::vector<std::pair<std::string, JsonValue>> members;

        auto Find(std::string_view key) const -> const JsonValue* {
            for (const auto& [k, v] : members) {
                if (k == key) { return &v; }
            }
            return nullptr;
        }
    };

    struct JsonValue {
        std::variant<std::string, double, JsonObject, std::vector<double>, std::monostate> data;

        [[nodiscard]] auto AsString() const -> const std::string* {
            return std::get_if<std::string>(&data);
        }
        [[nodiscard]] auto AsNumber() const -> const double* {
            return std::get_if<double>(&data);
        }
        [[nodiscard]] auto AsObject() const -> const JsonObject* {
            return std::get_if<JsonObject>(&data);
        }
        [[nodiscard]] auto AsArray() const -> const std::vector<double>* {
            return std::get_if<std::vector<double>>(&data);
        }
    };

    auto ParseValue() -> std::optional<JsonValue>
    {
        SkipWs();
        if (pos >= src.size()) { return std::nullopt; }
        char c = src[pos];

        if (c == '"') {
            auto s = ParseString();
            if (!s) { return std::nullopt; }
            return JsonValue{*s};
        }
        if (c == '{') {
            auto obj = ParseObject();
            if (!obj) { return std::nullopt; }
            return JsonValue{*obj};
        }
        if (c == '[') {
            auto arr = ParseArray();
            if (!arr) { return std::nullopt; }
            return JsonValue{*arr};
        }
        if (c == 'n') { // null
            if (pos + 4 <= src.size() && src.substr(pos, 4) == "null") {
                pos += 4;
                return JsonValue{std::monostate{}};
            }
            return std::nullopt;
        }
        // Try number
        auto num = ParseNumber();
        if (num) { return JsonValue{*num}; }
        return std::nullopt;
    }

    auto ParseObject() -> std::optional<JsonObject>
    {
        if (!Consume('{')) { return std::nullopt; }
        JsonObject obj;
        if (Peek() == '}') { ++pos; return obj; }
        while (true) {
            auto key = ParseString();
            if (!key) { return std::nullopt; }
            if (!Consume(':')) { return std::nullopt; }
            auto val = ParseValue();
            if (!val) { return std::nullopt; }
            obj.members.emplace_back(*key, *val);
            if (!Consume(',')) { break; }
        }
        if (!Consume('}')) { return std::nullopt; }
        return obj;
    }
};

// Convert parsed JSON object to DtcgToken (if it has $type and $value)
auto ParseDtcgToken(const std::string& name,
                    const JsonParser::JsonObject& obj) -> std::optional<DtcgToken>
{
    const auto* typeVal = obj.Find("$type");
    const auto* valueVal = obj.Find("$value");
    if (typeVal == nullptr || valueVal == nullptr) {
        return std::nullopt;
    }

    const auto* typeStr = typeVal->AsString();
    if (typeStr == nullptr) { return std::nullopt; }

    auto dtcgType = DtcgTypeFromString(*typeStr);
    if (!dtcgType) { return std::nullopt; }

    DtcgToken token;
    token.name = name;
    token.type = *dtcgType;

    // Check for alias reference (string starting with '{' and ending with '}')
    if (const auto* strVal = valueVal->AsString()) {
        if (strVal->size() >= 2 && strVal->front() == '{' && strVal->back() == '}') {
            token.aliasRef = strVal->substr(1, strVal->size() - 2);
            token.value = std::monostate{};
        } else {
            token.value = *strVal;
        }
    } else if (const auto* numVal = valueVal->AsNumber()) {
        token.value = *numVal;
    } else if (const auto* objVal = valueVal->AsObject()) {
        // Could be dimension or transition
        if (*dtcgType == DtcgType::Dimension) {
            DtcgDimension dim;
            if (const auto* v = objVal->Find("value")) {
                if (const auto* n = v->AsNumber()) { dim.value = *n; }
            }
            if (const auto* u = objVal->Find("unit")) {
                if (const auto* s = u->AsString()) { dim.unit = *s; }
            }
            token.value = dim;
        } else if (*dtcgType == DtcgType::Transition) {
            DtcgTransition tr;
            if (const auto* d = objVal->Find("duration")) {
                if (const auto* dObj = d->AsObject()) {
                    if (const auto* v = dObj->Find("value")) {
                        if (const auto* n = v->AsNumber()) { tr.duration = *n; }
                    }
                }
            }
            if (const auto* d2 = objVal->Find("delay")) {
                if (const auto* dObj = d2->AsObject()) {
                    if (const auto* v = dObj->Find("value")) {
                        if (const auto* n = v->AsNumber()) { tr.delay = *n; }
                    }
                }
            }
            if (const auto* tf = objVal->Find("timingFunction")) {
                if (const auto* arr = tf->AsArray()) {
                    tr.timingFunction = *arr;
                }
            }
            token.value = tr;
        }
    }

    // Description
    if (const auto* descVal = obj.Find("$description")) {
        if (const auto* s = descVal->AsString()) {
            token.description = *s;
        }
    }

    return token;
}

// Recursively convert a JSON object to DtcgTokenGroup
auto ParseGroup(const std::string& name,
                const JsonParser::JsonObject& obj) -> DtcgTokenGroup
{
    DtcgTokenGroup group;
    group.name = name;

    for (const auto& [key, val] : obj.members) {
        if (key.starts_with('$')) { continue; } // skip $schema etc.
        const auto* childObj = val.AsObject();
        if (childObj == nullptr) { continue; }

        // If it has $type and $value, it's a token
        if (auto token = ParseDtcgToken(key, *childObj)) {
            group.tokens.push_back(std::move(*token));
        } else {
            // Otherwise, it's a sub-group
            group.children.push_back(ParseGroup(key, *childObj));
        }
    }

    return group;
}

void FlattenGroup(const DtcgTokenGroup& group, const std::string& prefix,
                  std::unordered_map<std::string, DtcgToken>& out)
{
    const std::string groupPath = prefix.empty() ? group.name : (prefix + "." + group.name);
    for (const auto& token : group.tokens) {
        const std::string tokenPath = groupPath + "." + token.name;
        out[tokenPath] = token;
    }
    for (const auto& child : group.children) {
        FlattenGroup(child, groupPath, out);
    }
}

} // anonymous namespace

// ============================================================================
// DtcgSerializer
// ============================================================================

auto DtcgSerializer::Export(const DtcgTokenFile& file, int indent) -> std::string
{
    std::ostringstream os;
    os << '{' << Nl(indent);

    bool first = true;
    if (!file.schema.empty()) {
        WriteIndent(os, 1, indent);
        os << "\"$schema\":" << Sep(indent);
        WriteString(os, file.schema);
        first = false;
    }

    for (const auto& group : file.groups) {
        if (!first) { os << ',' << Nl(indent); }
        first = false;
        WriteGroup(os, group, 1, indent);
    }

    os << Nl(indent);
    os << '}';
    return os.str();
}

auto DtcgSerializer::Import(std::string_view json) -> std::optional<DtcgTokenFile>
{
    JsonParser parser;
    parser.src = json;

    auto root = parser.ParseObject();
    if (!root) { return std::nullopt; }

    DtcgTokenFile file;

    for (const auto& [key, val] : root->members) {
        if (key == "$schema") {
            if (const auto* s = val.AsString()) {
                file.schema = *s;
            }
            continue;
        }
        if (key.starts_with('$')) { continue; }

        const auto* childObj = val.AsObject();
        if (childObj == nullptr) { continue; }

        file.groups.push_back(ParseGroup(key, *childObj));
    }

    return file;
}

auto DtcgSerializer::Flatten(const DtcgTokenFile& file)
    -> std::unordered_map<std::string, DtcgToken>
{
    std::unordered_map<std::string, DtcgToken> result;
    for (const auto& group : file.groups) {
        FlattenGroup(group, "", result);
    }
    return result;
}

} // namespace matcha::fw
