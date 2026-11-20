#pragma once

#include "CivCatalog.h"

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
    static constexpr int MAX_LINK_LENGTH = 128;
    static constexpr int MAX_INPUT_LENGTH = 1024;

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
