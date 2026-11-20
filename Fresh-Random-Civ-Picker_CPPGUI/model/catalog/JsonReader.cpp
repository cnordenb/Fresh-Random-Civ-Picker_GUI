#include "JsonReader.h"

#include <algorithm>
#include <cstring>

#include "Assertions.h"

namespace
{
    constexpr int BYTE_ORDER_MARK_LENGTH = 3;
    constexpr int HEX_DIGITS = 4;
    constexpr int UNICODE_ESCAPE_LENGTH = 2 + HEX_DIGITS;
    constexpr int SIMPLE_ESCAPE_LENGTH = 2;

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
}

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
    for (int step = 0; step < length && position < length; step++)
    {
        const Scan scan = ScanAt(position);
        if (scan.status != JsonStatus::ok) return Fail(scan.status, position);
        if (!c_assert(scan.next > position)) return Fail(JsonStatus::syntax_error, position);
        position = scan.next;
    }
    if (expect != Expect::nothing) return Fail(JsonStatus::syntax_error, length);
    if (!c_assert(depth == 0 && !tokens.empty())) return Fail(JsonStatus::syntax_error, length);
    parsed = true;
    return JsonStatus::ok;
}

JsonStatus JsonDocument::Fail(JsonStatus status, int offset)
{
    (void)c_assert(status != JsonStatus::ok);
    error_offset = std::max(offset, 0);
    parsed = false;
    return status;
}

int JsonDocument::ErrorLine() const
{
    if (!c_assert(error_offset >= 0)) return 1;
    const int length = static_cast<int>(text.size());
    const int limit = std::min(error_offset, length);
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

int JsonDocument::DigitsEnd(int position, int end) const
{
    if (!c_assert(position >= 0 && end <= static_cast<int>(text.size()))) return position;
    int digits_end = position;
    for (int i = position; i < end && IsDigit(text[static_cast<size_t>(i)]); i++) digits_end = i + 1;
    return digits_end;
}

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
