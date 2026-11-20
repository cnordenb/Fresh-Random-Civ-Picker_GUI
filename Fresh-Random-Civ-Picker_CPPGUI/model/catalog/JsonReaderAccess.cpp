#include "JsonReader.h"

#include <cstring>

#include "Assertions.h"

namespace
{
    constexpr int MAX_INTEGER_DIGITS = 9;
    constexpr int MAX_UTF8_LENGTH = 4;
    constexpr unsigned int MAX_CODE_POINT = 0x10FFFF;
    constexpr unsigned int FIRST_SURROGATE = 0xD800;
    constexpr unsigned int LAST_SURROGATE = 0xDFFF;
    constexpr unsigned int FIRST_SUPPLEMENTARY = 0x10000;

    int Utf8Length(unsigned char lead)
    {
        if (lead < 0x80) return 1;
        if (lead >= 0xC2 && lead <= 0xDF) return 2;
        if (lead >= 0xE0 && lead <= 0xEF) return 3;
        if (lead >= 0xF0 && lead <= 0xF4) return 4;
        return 0;
    }
}

int JsonDocument::Root() const { return parsed && !tokens.empty() ? 0 : NONE; }

bool JsonDocument::IsValue(int value) const { return value >= 0 && value < static_cast<int>(tokens.size()); }

bool JsonDocument::Is(int value, JsonType type) const
{
    return IsValue(value) && tokens[static_cast<size_t>(value)].type == type;
}

int JsonDocument::Count(int container) const
{
    if (!Is(container, JsonType::object) && !Is(container, JsonType::array)) return 0;
    return tokens[static_cast<size_t>(container)].count;
}

int JsonDocument::Member(int object, const char* key) const
{
    if (!c_assert(key != nullptr) || !Is(object, JsonType::object)) return NONE;
    const int count = tokens[static_cast<size_t>(object)].count;
    int key_token = object + 1;
    for (int i = 0; i < count && i < MAX_VALUES; i++)
    {
        if (!c_assert(IsValue(key_token + 1))) return NONE;
        if (KeyEquals(key_token, key)) return key_token + 1;
        key_token = tokens[static_cast<size_t>(key_token + 1)].after;
    }
    return NONE;
}

int JsonDocument::MemberKey(int object, int position) const
{
    if (!Is(object, JsonType::object) || position < 0 || position >= Count(object)) return NONE;
    int key_token = object + 1;
    for (int i = 0; i < position && i < MAX_VALUES; i++)
    {
        if (!c_assert(IsValue(key_token + 1))) return NONE;
        key_token = tokens[static_cast<size_t>(key_token + 1)].after;
    }
    return c_assert(Is(key_token, JsonType::string)) ? key_token : NONE;
}

int JsonDocument::Element(int array, int position) const
{
    if (!Is(array, JsonType::array) || position < 0 || position >= Count(array)) return NONE;
    int element = array + 1;
    for (int i = 0; i < position && i < MAX_VALUES; i++)
    {
        if (!c_assert(IsValue(element))) return NONE;
        element = tokens[static_cast<size_t>(element)].after;
    }
    return c_assert(IsValue(element)) ? element : NONE;
}

bool JsonDocument::KeyEquals(int key, const char* name) const
{
    if (!c_assert(Is(key, JsonType::string)) || !c_assert(name != nullptr)) return false;
    const Token& token = tokens[static_cast<size_t>(key)];
    const size_t length = std::strlen(name);
    return static_cast<size_t>(token.end - token.start) == length &&
           text.compare(static_cast<size_t>(token.start), length, name) == 0;
}

JsonDocument::TextResult JsonDocument::Text(int string_value) const
{
    if (!Is(string_value, JsonType::string)) return { false, L"" };
    const Token& token = tokens[static_cast<size_t>(string_value)];
    if (!c_assert(token.start <= token.end)) return { false, L"" };
    std::wstring decoded;
    decoded.reserve(static_cast<size_t>(token.end - token.start));
    int position = token.start;
    for (int step = 0; step < token.end - token.start && position < token.end; step++)
    {
        const bool escape = text[static_cast<size_t>(position)] == '\\';
        const CodeUnits next = escape ? DecodeEscape(position) : DecodeUtf8(position, token.end);
        if (next.consumed <= 0) return { false, L"" };
        for (int i = 0; i < next.count && i < 2; i++) decoded.push_back(next.units[i]);
        position += next.consumed;
    }
    return { true, decoded };
}

JsonDocument::CodeUnits JsonDocument::DecodeUtf8(int position, int end) const
{
    const CodeUnits malformed = { 0, 0, { 0, 0 } };
    if (!c_assert(position >= 0 && position < end && end <= static_cast<int>(text.size()))) return malformed;
    const int length = Utf8Length(static_cast<unsigned char>(text[static_cast<size_t>(position)]));
    if (length == 0 || position + length > end) return malformed;
    const unsigned int lead_bits[MAX_UTF8_LENGTH + 1] = { 0, 0x7F, 0x1F, 0x0F, 0x07 };
    const unsigned int shortest[MAX_UTF8_LENGTH + 1] = { 0, 0, 0x80, 0x800, FIRST_SUPPLEMENTARY };
    unsigned int code_point = static_cast<unsigned char>(text[static_cast<size_t>(position)]) & lead_bits[length];
    for (int i = 1; i < length; i++)
    {
        const unsigned char next = static_cast<unsigned char>(text[static_cast<size_t>(position + i)]);
        if ((next & 0xC0u) != 0x80u) return malformed;
        code_point = (code_point << 6) | (next & 0x3Fu);
    }
    const bool surrogate = code_point >= FIRST_SURROGATE && code_point <= LAST_SURROGATE;
    if (code_point < shortest[length] || code_point > MAX_CODE_POINT || surrogate) return malformed;
    if (code_point < FIRST_SUPPLEMENTARY) return { length, 1, { static_cast<wchar_t>(code_point), 0 } };
    const unsigned int offset = code_point - FIRST_SUPPLEMENTARY;
    return { length, 2, { static_cast<wchar_t>(FIRST_SURROGATE + (offset >> 10)),
                          static_cast<wchar_t>(0xDC00u + (offset & 0x3FFu)) } };
}

JsonDocument::IntegerResult JsonDocument::Integer(int number_value) const
{
    if (!Is(number_value, JsonType::number)) return { false, 0 };
    const Token& token = tokens[static_cast<size_t>(number_value)];
    if (!c_assert(token.start < token.end)) return { false, 0 };
    const bool negative = text[static_cast<size_t>(token.start)] == '-';
    const int first_digit = negative ? token.start + 1 : token.start;
    const int digit_count = token.end - first_digit;
    if (digit_count < 1 || digit_count > MAX_INTEGER_DIGITS) return { false, 0 };
    if (DigitsEnd(first_digit, token.end) != token.end) return { false, 0 };
    int value = 0;
    for (int i = first_digit; i < token.end; i++) value = value * 10 + (text[static_cast<size_t>(i)] - '0');
    return { true, negative ? -value : value };
}

bool JsonDocument::IsTrue(int value) const
{
    return Is(value, JsonType::literal) && text.compare(static_cast<size_t>(tokens[static_cast<size_t>(value)].start),
                                                        4, "true") == 0;
}

bool JsonDocument::IsNull(int value) const
{
    return Is(value, JsonType::literal) && text.compare(static_cast<size_t>(tokens[static_cast<size_t>(value)].start),
                                                        4, "null") == 0;
}
