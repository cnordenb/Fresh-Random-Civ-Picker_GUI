#include "JsonReader.h"

#include <cstring>

#include "Assertions.h"

namespace
{
    constexpr int BYTE_ORDER_MARK_LENGTH = 3;
    constexpr int HEX_DIGITS = 4; // \uXXXX
    constexpr int UNICODE_ESCAPE_LENGTH = 2 + HEX_DIGITS;
    constexpr int SIMPLE_ESCAPE_LENGTH = 2;
    constexpr int MAX_INTEGER_DIGITS = 9; // so an integer always fits an int
    constexpr int MAX_UTF8_LENGTH = 4;
    constexpr unsigned int MAX_CODE_POINT = 0x10FFFF;
    constexpr unsigned int FIRST_SURROGATE = 0xD800;
    constexpr unsigned int LAST_SURROGATE = 0xDFFF;
    constexpr unsigned int FIRST_SUPPLEMENTARY = 0x10000;

    // The escapes a JSON string may use after its backslash, and the characters they stand for.
    constexpr int SIMPLE_ESCAPE_COUNT = 8;
    constexpr char ESCAPE_KINDS[SIMPLE_ESCAPE_COUNT + 1] = "\"\\/bfnrt";
    constexpr wchar_t ESCAPE_MEANINGS[SIMPLE_ESCAPE_COUNT] = { L'"', L'\\', L'/', L'\b', L'\f', L'\n', L'\r', L'\t' };

    bool IsDigit(char c) { return c >= '0' && c <= '9'; }

    bool IsNumberCharacter(char c) { return IsDigit(c) || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E'; }

    int HexDigit(char c)
    {
        if (IsDigit(c)) return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    // The length of the UTF-8 sequence a byte starts, or 0 if it can't start one.
    int Utf8Length(unsigned char lead)
    {
        if (lead < 0x80) return 1;
        if (lead >= 0xC2 && lead <= 0xDF) return 2;
        if (lead >= 0xE0 && lead <= 0xEF) return 3;
        if (lead >= 0xF0 && lead <= 0xF4) return 4;
        return 0;
    }
}

// --- Tokenizing ---

JsonStatus JsonDocument::Parse(const std::string& utf8_text)
{
    parsed = false;
    text.clear();
    tokens.clear();
    depth = 0;
    expect = Expect::value;
    if (utf8_text.size() > static_cast<size_t>(MAX_TEXT_LENGTH)) return Fail(JsonStatus::too_long, 0);
    text = utf8_text;
    tokens.reserve(MAX_VALUES);
    const int length = static_cast<int>(text.size());
    int position = text.compare(0, BYTE_ORDER_MARK_LENGTH, "\xEF\xBB\xBF") == 0 ? BYTE_ORDER_MARK_LENGTH : 0;
    // Every scan consumes at least one character, so the text's length bounds the loop.
    for (int step = 0; step < length && position < length; step++)
    {
        const Scan scan = ScanAt(position);
        if (scan.status != JsonStatus::ok) return Fail(scan.status, position);
        if (!c_assert(scan.next > position)) return Fail(JsonStatus::syntax_error, position);
        position = scan.next;
    }
    // The text ended inside a value, or held none.
    if (expect != Expect::nothing) return Fail(JsonStatus::syntax_error, length);
    // A complete value has closed every container it opened.
    if (!c_assert(depth == 0 && !tokens.empty())) return Fail(JsonStatus::syntax_error, length);
    parsed = true;
    return JsonStatus::ok;
}

JsonStatus JsonDocument::Fail(JsonStatus status, int offset)
{
    (void)c_assert(status != JsonStatus::ok);
    error_offset = offset >= 0 ? offset : 0;
    parsed = false;
    return status;
}

// The line of the text where parsing stopped, counted from 1.
int JsonDocument::ErrorLine() const
{
    if (!c_assert(error_offset >= 0)) return 1;
    const int length = static_cast<int>(text.size());
    const int limit = error_offset < length ? error_offset : length;
    int line = 1;
    for (int i = 0; i < limit; i++)
    {
        if (text[static_cast<size_t>(i)] == '\n') line++;
    }
    return line;
}

JsonDocument::Scan JsonDocument::ScanAt(int position)
{
    if (!c_assert(position >= 0 && position < static_cast<int>(text.size()))) return { JsonStatus::syntax_error, 0 };
    const char c = text[static_cast<size_t>(position)];
    switch (c)
    {
        case ' ':
        case '\t':
        case '\r':
        case '\n': return { JsonStatus::ok, position + 1 };
        case '{': return Open(JsonType::object, position);
        case '[': return Open(JsonType::array, position);
        case '}': return Close(JsonType::object, position);
        case ']': return Close(JsonType::array, position);
        case ',': return Comma(position);
        case ':': return Colon(position);
        case '"': return String(position);
        default: return c == '-' || IsDigit(c) ? Number(position) : Literal(position);
    }
}

JsonDocument::Scan JsonDocument::Open(JsonType type, int position)
{
    if (!c_assert(type == JsonType::object || type == JsonType::array)) return { JsonStatus::syntax_error, position };
    if (depth >= MAX_DEPTH) return { JsonStatus::too_deep, position };
    const JsonStatus begun = BeginValue(type, position);
    if (begun != JsonStatus::ok) return { begun, position };
    open[depth] = static_cast<int>(tokens.size()) - 1;
    depth++;
    expect = type == JsonType::object ? Expect::key_or_close : Expect::value_or_close;
    return { JsonStatus::ok, position + 1 };
}

JsonDocument::Scan JsonDocument::Close(JsonType type, int position)
{
    const int container = TopContainer();
    if (container == NONE || !c_assert(IsValue(container)) || tokens[static_cast<size_t>(container)].type != type)
        return { JsonStatus::syntax_error, position };
    // A container closes after one of its values, or right after it opened.
    const Expect empty = type == JsonType::object ? Expect::key_or_close : Expect::value_or_close;
    if (expect != Expect::comma_or_close && expect != empty) return { JsonStatus::syntax_error, position };
    Token& token = tokens[static_cast<size_t>(container)];
    token.end = position + 1;
    token.after = static_cast<int>(tokens.size());
    depth--;
    EndValue();
    return { JsonStatus::ok, position + 1 };
}

JsonDocument::Scan JsonDocument::Comma(int position)
{
    if (!c_assert(position >= 0)) return { JsonStatus::syntax_error, 0 };
    const int container = TopContainer();
    if (expect != Expect::comma_or_close || container == NONE) return { JsonStatus::syntax_error, position };
    expect = tokens[static_cast<size_t>(container)].type == JsonType::object ? Expect::key : Expect::value;
    return { JsonStatus::ok, position + 1 };
}

JsonDocument::Scan JsonDocument::Colon(int position)
{
    if (!c_assert(position >= 0)) return { JsonStatus::syntax_error, 0 };
    if (expect != Expect::colon) return { JsonStatus::syntax_error, position };
    expect = Expect::value;
    return { JsonStatus::ok, position + 1 };
}

// A string is a member's key where the object expects one, and a value everywhere else.
JsonDocument::Scan JsonDocument::String(int position)
{
    const int closing = StringEnd(position);
    if (closing == NONE || !c_assert(closing > position)) return { JsonStatus::syntax_error, position };
    if (expect == Expect::key || expect == Expect::key_or_close) return { AddKey(position + 1, closing), closing + 1 };
    const JsonStatus begun = BeginValue(JsonType::string, position + 1);
    if (begun != JsonStatus::ok) return { begun, position };
    tokens.back().end = closing;
    EndValue();
    return { JsonStatus::ok, closing + 1 };
}

JsonDocument::Scan JsonDocument::Number(int position)
{
    if (!c_assert(position >= 0 && position < static_cast<int>(text.size()))) return { JsonStatus::syntax_error, 0 };
    const int end = NumberEnd(position);
    if (!IsNumber(position, end)) return { JsonStatus::syntax_error, position };
    const JsonStatus begun = BeginValue(JsonType::number, position);
    if (begun != JsonStatus::ok) return { begun, position };
    tokens.back().end = end;
    EndValue();
    return { JsonStatus::ok, end };
}

// true, false or null.
JsonDocument::Scan JsonDocument::Literal(int position)
{
    if (!c_assert(position >= 0 && position < static_cast<int>(text.size()))) return { JsonStatus::syntax_error, 0 };
    const char* const literals[] = { "true", "false", "null" };
    for (const char* literal : literals)
    {
        const size_t length = std::strlen(literal);
        if (text.compare(static_cast<size_t>(position), length, literal) != 0) continue;
        const JsonStatus begun = BeginValue(JsonType::literal, position);
        if (begun != JsonStatus::ok) return { begun, position };
        const int end = position + static_cast<int>(length);
        tokens.back().end = end;
        EndValue();
        return { JsonStatus::ok, end };
    }
    return { JsonStatus::syntax_error, position };
}

JsonStatus JsonDocument::BeginValue(JsonType type, int start)
{
    if (!c_assert(start >= 0)) return JsonStatus::syntax_error;
    if (expect != Expect::value && expect != Expect::value_or_close) return JsonStatus::syntax_error;
    if (static_cast<int>(tokens.size()) >= MAX_VALUES) return JsonStatus::too_many_values;
    // An array counts its elements as they begin; an object counts its members by their keys.
    const int container = TopContainer();
    if (container != NONE && tokens[static_cast<size_t>(container)].type == JsonType::array)
        tokens[static_cast<size_t>(container)].count++;
    const int index = static_cast<int>(tokens.size());
    tokens.push_back({ type, start, start, 0, index + 1 });
    return JsonStatus::ok;
}

JsonStatus JsonDocument::AddKey(int start, int end)
{
    const int container = TopContainer();
    if (!c_assert(container != NONE) || !c_assert(start <= end)) return JsonStatus::syntax_error;
    if (static_cast<int>(tokens.size()) >= MAX_VALUES) return JsonStatus::too_many_values;
    tokens[static_cast<size_t>(container)].count++;
    const int index = static_cast<int>(tokens.size());
    tokens.push_back({ JsonType::string, start, end, 0, index + 1 });
    expect = Expect::colon;
    return JsonStatus::ok;
}

void JsonDocument::EndValue()
{
    (void)c_assert(depth >= 0 && depth <= MAX_DEPTH);
    expect = depth > 0 ? Expect::comma_or_close : Expect::nothing;
}

int JsonDocument::TopContainer() const
{
    if (!c_assert(depth >= 0 && depth <= MAX_DEPTH)) return NONE;
    return depth > 0 ? open[depth - 1] : NONE;
}

// The index of the quote that closes the string opening at position, or NONE if the string is malformed.
int JsonDocument::StringEnd(int position) const
{
    const int length = static_cast<int>(text.size());
    if (!c_assert(position >= 0 && position < length)) return NONE;
    int i = position + 1;
    for (int step = 0; step < length && i < length; step++)
    {
        const unsigned char c = static_cast<unsigned char>(text[static_cast<size_t>(i)]);
        if (c == '"') return i;
        if (c < 0x20) return NONE;
        const int skipped = c == '\\' ? EscapeLength(i) : 1;
        if (skipped == 0) return NONE;
        i += skipped;
    }
    return NONE;
}

// The length of the escape at position (from its backslash), or 0 if it is malformed.
int JsonDocument::EscapeLength(int position) const
{
    const int length = static_cast<int>(text.size());
    if (!c_assert(position >= 0 && position < length)) return 0;
    if (position + 1 >= length) return 0;
    const char kind = text[static_cast<size_t>(position + 1)];
    if (kind == 'u') return Hex4(position + 2).ok ? UNICODE_ESCAPE_LENGTH : 0;
    for (int i = 0; i < SIMPLE_ESCAPE_COUNT; i++)
    {
        if (ESCAPE_KINDS[i] == kind) return SIMPLE_ESCAPE_LENGTH;
    }
    return 0;
}

JsonDocument::HexResult JsonDocument::Hex4(int position) const
{
    if (!c_assert(position >= 0) || position + HEX_DIGITS > static_cast<int>(text.size())) return { false, 0 };
    unsigned int value = 0;
    for (int i = 0; i < HEX_DIGITS; i++)
    {
        const int digit = HexDigit(text[static_cast<size_t>(position + i)]);
        if (digit < 0) return { false, 0 };
        value = value * 16 + static_cast<unsigned int>(digit);
    }
    return { true, value };
}

int JsonDocument::NumberEnd(int position) const
{
    const int length = static_cast<int>(text.size());
    if (!c_assert(position >= 0 && position < length)) return position;
    int end = position;
    for (int i = position; i < length && IsNumberCharacter(text[static_cast<size_t>(i)]); i++) end = i + 1;
    return end;
}

// JSON's number grammar: an optional minus, an integer without leading zeros, then optionally a fraction and an
// exponent.
bool JsonDocument::IsNumber(int start, int end) const
{
    if (!c_assert(start >= 0 && start < end && end <= static_cast<int>(text.size()))) return false;
    const int integer_start = text[static_cast<size_t>(start)] == '-' ? start + 1 : start;
    int position = DigitsEnd(integer_start, end);
    if (position == integer_start) return false;
    if (text[static_cast<size_t>(integer_start)] == '0' && position > integer_start + 1) return false;
    if (position < end && text[static_cast<size_t>(position)] == '.')
    {
        const int fraction_end = DigitsEnd(position + 1, end);
        if (fraction_end == position + 1) return false;
        position = fraction_end;
    }
    if (position < end && (text[static_cast<size_t>(position)] == 'e' || text[static_cast<size_t>(position)] == 'E'))
    {
        int exponent = position + 1;
        const bool signed_exponent = exponent < end && (text[static_cast<size_t>(exponent)] == '+' ||
                                                        text[static_cast<size_t>(exponent)] == '-');
        if (signed_exponent) exponent++;
        const int exponent_end = DigitsEnd(exponent, end);
        if (exponent_end == exponent) return false;
        position = exponent_end;
    }
    return position == end;
}

// The first position from position on (up to end) that doesn't hold a digit.
int JsonDocument::DigitsEnd(int position, int end) const
{
    if (!c_assert(position >= 0 && end <= static_cast<int>(text.size()))) return position;
    int digits_end = position;
    for (int i = position; i < end && IsDigit(text[static_cast<size_t>(i)]); i++) digits_end = i + 1;
    return digits_end;
}

// --- Reading the values ---

int JsonDocument::Root() const { return parsed && !tokens.empty() ? 0 : NONE; }

bool JsonDocument::IsValue(int value) const { return value >= 0 && value < static_cast<int>(tokens.size()); }

bool JsonDocument::Is(int value, JsonType type) const
{
    return IsValue(value) && tokens[static_cast<size_t>(value)].type == type;
}

// The members of an object or the elements of an array; 0 for anything else, including NONE.
int JsonDocument::Count(int container) const
{
    if (!Is(container, JsonType::object) && !Is(container, JsonType::array)) return 0;
    return tokens[static_cast<size_t>(container)].count;
}

// The value of an object's member, or NONE if the object (or what stands for it) has no member of that name.
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

// The key of an object's member at position; its value is the token after it.
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

// Keys are compared as written: a key that uses escapes doesn't match.
bool JsonDocument::KeyEquals(int key, const char* name) const
{
    if (!c_assert(Is(key, JsonType::string)) || !c_assert(name != nullptr)) return false;
    const Token& token = tokens[static_cast<size_t>(key)];
    const size_t length = std::strlen(name);
    return static_cast<size_t>(token.end - token.start) == length &&
           text.compare(static_cast<size_t>(token.start), length, name) == 0;
}

// A string value (or a key) in UTF-16, its escapes and UTF-8 characters decoded.
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

// An escape that StringEnd accepted; a \u escape gives one UTF-16 code unit, as JSON writes surrogate pairs as
// two escapes.
JsonDocument::CodeUnits JsonDocument::DecodeEscape(int position) const
{
    const CodeUnits malformed = { 0, 0, { 0, 0 } };
    const int length = EscapeLength(position);
    if (!c_assert(length > 0)) return malformed;
    const char kind = text[static_cast<size_t>(position + 1)];
    if (kind == 'u') return { length, 1, { static_cast<wchar_t>(Hex4(position + 2).value), 0 } };
    for (int i = 0; i < SIMPLE_ESCAPE_COUNT; i++)
    {
        if (ESCAPE_KINDS[i] == kind) return { length, 1, { ESCAPE_MEANINGS[i], 0 } };
    }
    return malformed;
}

// One UTF-8 character as one UTF-16 code unit, or two (a surrogate pair) beyond the Basic Multilingual Plane.
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

// A number value without a fraction or exponent, of at most MAX_INTEGER_DIGITS digits.
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
