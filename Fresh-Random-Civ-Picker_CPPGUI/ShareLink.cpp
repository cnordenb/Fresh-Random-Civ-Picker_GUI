#include "ShareLink.h"

#include <cwchar>
#include <cwctype>

#include "Assertions.h"

namespace
{
    const wchar_t* const WEB_ADDRESS = L"https://frcp.net/";
    const wchar_t* const CODE_MARK = L"#p=";
    constexpr char FORMAT_VERSION = '1';
    constexpr char DRAWS_MARK = '.';
    constexpr int BITS_PER_DIGIT = 6;
    constexpr int BITFIELD_START = 2; // after the version and the edition
    // base64url: the digit of each value from 0 to 63.
    const wchar_t* const DIGITS = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    constexpr int DIGIT_COUNT = 64;
    constexpr wchar_t HIGHEST_ASCII = 0x7F;
    static_assert(MAX_CIVS <= DIGIT_COUNT && MAX_EDITIONS <= DIGIT_COUNT, "a digit stands for each civ and edition");

    int DigitValue(char digit)
    {
        if (digit >= 'A' && digit <= 'Z') return digit - 'A';
        if (digit >= 'a' && digit <= 'z') return digit - 'a' + 26;
        if (digit >= '0' && digit <= '9') return digit - '0' + 52;
        if (digit == '-') return 62;
        if (digit == '_') return 63;
        return -1;
    }

    int HexValue(wchar_t digit)
    {
        if (digit >= L'0' && digit <= L'9') return digit - L'0';
        if (digit >= L'a' && digit <= L'f') return digit - L'a' + 10;
        if (digit >= L'A' && digit <= L'F') return digit - L'A' + 10;
        return -1;
    }

    // Opens a link (or only its code) as the web version's parseShare does: Open says whether the text is one.
    class LinkOpener final
    {
    public:
        explicit LinkOpener(const CivCatalog& civ_catalog) : catalog(civ_catalog) {}
        ShareLink::Opened Open(const wchar_t* text);

    private:
        bool ReadCode(const wchar_t* text);
        bool ReadParts();
        void ReadPool();
        bool ReadDraws();

        const CivCatalog& catalog;
        char code[ShareLink::MAX_INPUT_LENGTH] = {};
        int length = 0;
        int bits_end = 0;
        int draws_start = 0;
        SharedPool pool = {};
    };

    ShareLink::Opened LinkOpener::Open(const wchar_t* text)
    {
        if (!c_assert(text != nullptr)) return { false, {} };
        pool = {};
        if (!ReadCode(text) || !ReadParts()) return { false, {} };
        ReadPool();
        if (!ReadDraws()) return { false, {} };
        return { true, pool };
    }

    // The code: the text trimmed, from after its first "#", without "p=", and with its percent escapes decoded
    // (what JavaScript's trim and decodeURIComponent do). A character outside ASCII can't be part of a code.
    bool LinkOpener::ReadCode(const wchar_t* text)
    {
        if (!c_assert(text != nullptr)) return false;
        const int text_length = static_cast<int>(wcsnlen(text, ShareLink::MAX_INPUT_LENGTH + 1));
        if (text_length > ShareLink::MAX_INPUT_LENGTH) return false;
        int start = 0;
        int end = text_length;
        while (start < end && std::iswspace(text[start]) != 0) start++;
        while (end > start && std::iswspace(text[end - 1]) != 0) end--;
        const wchar_t* hash = wmemchr(text + start, L'#', static_cast<size_t>(end - start));
        if (hash != nullptr) start = static_cast<int>(hash - text) + 1;
        if (end - start >= 2 && text[start] == L'p' && text[start + 1] == L'=') start += 2;
        length = 0;
        for (int i = start; i < end; i++)
        {
            wchar_t character = text[i];
            if (character == L'%')
            {
                const int high = i + 2 < end ? HexValue(text[i + 1]) : -1;
                const int low = i + 2 < end ? HexValue(text[i + 2]) : -1;
                if (high < 0 || low < 0) return false;
                character = static_cast<wchar_t>(high * 16 + low);
                i += 2;
            }
            if (character == L'\0' || character > HIGHEST_ASCII) return false;
            code[length] = static_cast<char>(character);
            length++;
        }
        return true;
    }

    // The version, the edition, at least one digit of bitfield, and optionally "." and at least one draw: nothing
    // else (the web version's /^1([A-Za-z0-9_-])([A-Za-z0-9_-]+)(?:\.([A-Za-z0-9_-]+))?$/).
    bool LinkOpener::ReadParts()
    {
        if (!c_assert(length >= 0 && length <= ShareLink::MAX_INPUT_LENGTH)) return false;
        if (length <= BITFIELD_START || code[0] != FORMAT_VERSION) return false;
        pool.edition = DigitValue(code[1]);
        if (pool.edition < 0 || pool.edition >= catalog.EditionCount()) return false;
        bits_end = BITFIELD_START;
        while (bits_end < length && code[bits_end] != DRAWS_MARK)
        {
            if (DigitValue(code[bits_end]) < 0) return false;
            bits_end++;
        }
        if (bits_end == BITFIELD_START) return false;
        draws_start = bits_end < length ? bits_end + 1 : length;
        if (bits_end < length && draws_start == length) return false;
        for (int i = draws_start; i < length; i++)
        {
            if (DigitValue(code[i]) < 0) return false;
        }
        return true;
    }

    // A civ is on if it belongs to the edition and its bit is set, or lies past the end of the bitfield.
    void LinkOpener::ReadPool()
    {
        if (!c_assert(bits_end > BITFIELD_START && bits_end <= length)) return;
        const int digits = bits_end - BITFIELD_START;
        for (int civ = 0; civ < catalog.CivCount() && civ < MAX_CIVS; civ++)
        {
            const int digit = civ / BITS_PER_DIGIT;
            const int value = digit < digits ? DigitValue(code[BITFIELD_START + digit]) : DIGIT_COUNT - 1;
            const bool bit_set = ((value >> (civ % BITS_PER_DIGIT)) & 1) == 1;
            pool.enabled[civ] = bit_set && catalog.InEdition(civ, pool.edition);
        }
    }

    // Each draw is a civ in the pool, drawn once.
    bool LinkOpener::ReadDraws()
    {
        if (!c_assert(draws_start >= bits_end && draws_start <= length)) return false;
        bool drawn[MAX_CIVS] = {};
        pool.draw_count = 0;
        for (int i = draws_start; i < length; i++)
        {
            const int civ = DigitValue(code[i]);
            if (civ < 0 || civ >= catalog.CivCount() || !pool.enabled[civ] || drawn[civ]) return false;
            drawn[civ] = true;
            pool.draws[pool.draw_count] = civ;
            pool.draw_count++;
        }
        return true;
    }
}

const wchar_t* ShareLink::Text::Characters() const { return text; }

int ShareLink::Text::Length() const { return length; }

void ShareLink::Text::Append(wchar_t character)
{
    if (!c_assert(length >= 0 && length + 1 < MAX_LINK_LENGTH)) return;
    text[length] = character;
    length++;
    text[length] = L'\0';
}

void ShareLink::Text::Append(const wchar_t* characters)
{
    if (!c_assert(characters != nullptr)) return;
    for (int i = 0; i < MAX_LINK_LENGTH && characters[i] != L'\0'; i++) Append(characters[i]);
}

// The link the web version writes for the same pool (its shareCode), with the draws if the pool has any.
ShareLink::Text ShareLink::Write(const CivCatalog& catalog, const SharedPool& pool)
{
    const int civ_count = catalog.CivCount();
    if (!c_assert(pool.edition >= 0 && pool.edition < catalog.EditionCount())) return Text();
    if (!c_assert(pool.draw_count >= 0 && pool.draw_count <= civ_count)) return Text();
    Text link;
    link.Append(WEB_ADDRESS);
    link.Append(CODE_MARK);
    link.Append(static_cast<wchar_t>(FORMAT_VERSION));
    link.Append(DIGITS[pool.edition]);
    const int digits = (civ_count + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
    for (int digit = 0; digit < digits; digit++)
    {
        int value = 0;
        for (int bit = 0; bit < BITS_PER_DIGIT; bit++)
        {
            const int civ = digit * BITS_PER_DIGIT + bit;
            if (civ >= civ_count || pool.enabled[civ]) value |= 1 << bit;
        }
        link.Append(DIGITS[value]);
    }
    if (pool.draw_count > 0) link.Append(static_cast<wchar_t>(DRAWS_MARK));
    for (int i = 0; i < pool.draw_count && i < MAX_CIVS; i++)
    {
        if (!c_assert(pool.draws[i] >= 0 && pool.draws[i] < civ_count)) return Text();
        link.Append(DIGITS[pool.draws[i]]);
    }
    return link;
}

ShareLink::Opened ShareLink::Open(const CivCatalog& catalog, const wchar_t* text)
{
    if (!c_assert(text != nullptr) || !c_assert(catalog.CivCount() > 0)) return { false, {} };
    LinkOpener opener(catalog);
    return opener.Open(text);
}
