#pragma once

#include "CivCatalog.h"

// Part of the MODEL: the share links of the web version (frcp.net), which both versions write and open. A link is
// the web version's address, "#p=" and a code: the format version ("1"), the edition by its place in data.json, the
// civ pool as a bitfield of six civs per character (bit b of character c is the civ with ID 6c + b), and
// optionally "." and the civs drawn this round, one character each in draw order. The characters are base64url
// digits. Civs past the end of a link's bitfield count as enabled, so civs added to data.json after a link was
// made are on when it is opened. Plain C++, unit-tested.

// What a link carries: an edition, which civs are on, and the draws of the current round.
struct SharedPool
{
    int edition;
    bool enabled[MAX_CIVS];
    int draws[MAX_CIVS];
    int draw_count;
};

class ShareLink final
{
public:
    static constexpr int MAX_LINK_LENGTH = 128; // the longest link written has 98 characters
    static constexpr int MAX_INPUT_LENGTH = 1024; // the longest text a link is read from

    // A link as written: its characters, null-terminated.
    class Text final
    {
    public:
        const wchar_t* Characters() const;
        int Length() const;
        void Append(wchar_t character);
        void Append(const wchar_t* characters);

    private:
        wchar_t text[MAX_LINK_LENGTH] = L"";
        int length = 0;
    };

    struct Opened
    {
        bool ok;
        SharedPool pool;
    };

    static Text Write(const CivCatalog& catalog, const SharedPool& pool);
    static Opened Open(const CivCatalog& catalog, const wchar_t* text);
};
