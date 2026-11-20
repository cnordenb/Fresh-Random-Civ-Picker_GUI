#pragma once

#include "ParchmentControl.h"

class TextButton final : public ParchmentControl
{
public:
    TextButton() = default;

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
};

class AtlasButton final : public ParchmentControl
{
public:
    struct Looks
    {
        ImageId image;
        int cell_size;
        bool has_hover_look;
    };

    void SetLooks(const Looks& button_looks);

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;

    Looks looks = { ImageId::options_button, OPTIONS_BUTTON_CELL, true };
};

class FramedIconButton final : public ParchmentControl
{
public:
    struct Icon
    {
        ImageId image;
        int size;
        ParchmentRenderer::Smoothing smoothing;
    };

    void SetIcon(const Icon& button_icon);

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;

    Icon icon = { ImageId::aoe2vils_icon, 0, ParchmentRenderer::Smoothing::pixelated };
};

class PictureButton final : public ParchmentControl
{
public:
    struct Picture
    {
        ImageId image;
        PixelRect source;
    };

    void ShowPicture(const Picture& shown_picture);

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;

    Picture picture = { ImageId::civ_atlas, { 0, 0, 0, 0 } };
};

class CloseButton final : public ParchmentControl
{
public:
    CloseButton() = default;

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    CursorId CursorShape() const override;
};

class LinkButton final : public ParchmentControl
{
public:
    LinkButton() = default;

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    bool ShowsFocus() const override;
};

struct KeyCap
{
    FloatRect box;
    TextSpan name;
    Colour face;
};

void PaintKeyCap(ParchmentRenderer& renderer, const KeyCap& cap);

class KeyButton final : public ParchmentControl
{
public:
    static constexpr WORD KEY_LISTENING = 0x4B01;
    static constexpr WORD KEY_CHOSEN = 0x4B02;
    static constexpr WORD KEY_KEPT = 0x4B03;
    static constexpr UINT GET_KEYSTROKE = WM_USER + 0x100;
    static constexpr UINT STOP_LISTENING = WM_USER + 0x101;

    KeyButton() = default;

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    void OnClicked() override;
    bool NotifiesClicks() const override;
    LRESULT DialogCode() const override;
    bool OnKeyPressed(WPARAM key) override;
    LRESULT OnOtherMessage(UINT message, WPARAM wParam, LPARAM lParam) override;
    void OnFocusLost() override;
    void Choose(WPARAM key);
    void Notify(WORD notification) const;

    bool listening = false;
    LRESULT chosen = 0;
};
