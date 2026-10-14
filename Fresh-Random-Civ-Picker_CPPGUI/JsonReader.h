#pragma once

#include <string>
#include <vector>

// A reader for the program's JSON data file (data.json), in plain C++ so it can be unit-tested.
// Parse() splits the text into one token per value, in the order the values appear; an object's members are its
// key and value tokens in turn, so a container's contents follow it and end where its "after" index points.
// The tokenizer is one loop over the text with an explicit stack of open containers: no recursion (P10-1), and
// every loop is bounded by the text's length or a fixed capacity (P10-2). It runs at startup (P10-3).

enum class JsonType { object, array, string, number, literal };

enum class JsonStatus { ok, too_long, too_many_values, too_deep, syntax_error };

class JsonDocument final
{
public:
    static constexpr int MAX_TEXT_LENGTH = 1024 * 1024;
    static constexpr int MAX_VALUES = 16384;
    static constexpr int MAX_DEPTH = 16;
    static constexpr int NONE = -1; // the index of a value that isn't there

    struct TextResult
    {
        bool ok;
        std::wstring text;
    };

    struct IntegerResult
    {
        bool ok;
        int value;
    };

    JsonStatus Parse(const std::string& utf8_text);
    int ErrorLine() const;

    int Root() const;
    bool Is(int value, JsonType type) const;
    int Count(int container) const;
    int Member(int object, const char* key) const;
    int MemberKey(int object, int position) const;
    int Element(int array, int position) const;
    TextResult Text(int string_value) const;
    IntegerResult Integer(int number_value) const;
    bool IsTrue(int value) const;
    bool IsNull(int value) const;

private:
    struct Token
    {
        JsonType type;
        int start; // the first character (inside the quotes, for a string)
        int end; // one past the last character
        int count; // an object's members or an array's elements
        int after; // the index of the first token after this value and its contents
    };

    // What the next character may be, after what came before it.
    enum class Expect { value, value_or_close, key, key_or_close, colon, comma_or_close, nothing };

    struct Scan
    {
        JsonStatus status;
        int next;
    };

    // The UTF-16 code units one escape or one UTF-8 character of a string stands for; consumed is 0 if it is
    // malformed.
    struct CodeUnits
    {
        int consumed;
        int count;
        wchar_t units[2];
    };

    struct HexResult
    {
        bool ok;
        unsigned int value;
    };

    JsonStatus Fail(JsonStatus status, int offset);
    Scan ScanAt(int position);
    Scan Open(JsonType type, int position);
    Scan Close(JsonType type, int position);
    Scan Comma(int position);
    Scan Colon(int position);
    Scan String(int position);
    Scan Number(int position);
    Scan Literal(int position);
    JsonStatus BeginValue(JsonType type, int start);
    JsonStatus AddKey(int start, int end);
    void EndValue();
    int StringEnd(int position) const;
    int EscapeLength(int position) const;
    int NumberEnd(int position) const;
    bool IsNumber(int start, int end) const;
    int DigitsEnd(int position, int end) const;
    int TopContainer() const;
    bool IsValue(int value) const;
    bool KeyEquals(int key, const char* name) const;
    CodeUnits DecodeEscape(int position) const;
    CodeUnits DecodeUtf8(int position, int end) const;
    HexResult Hex4(int position) const;

    std::string text;
    std::vector<Token> tokens;
    int open[MAX_DEPTH] = {};
    int depth = 0;
    Expect expect = Expect::value;
    int error_offset = 0;
    bool parsed = false;
};
