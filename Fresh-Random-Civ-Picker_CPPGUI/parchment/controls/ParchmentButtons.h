#pragma once

#include "ParchmentControl.h"

// The clickable controls of the parchment theme (View), each drawn as its web version counterpart (style.css).

// A wooden button with a blackletter face (.btn): brighter text on hover, the pressed image while held.
class TextButton final : public ParchmentControl
{
public:
    TextButton() = default;

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
};

// An image button whose looks sit side by side in one image (#optionsBig, #techBtn): normal first, pressed last,
// and a hover look between them where the image has one.
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

// A small wooden button drawn from the button nine-slice with an icon over it (the web version's share buttons),
// pressed while held. Pixel art icons are enlarged without smoothing.
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

// A clickable picture: part of an image drawn over the whole control (the civ icon, the edition banner, the
// History book).
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

// The wooden x of a title bar (.dlg-x): its normal or pressed look in the middle of the 46 x 30 button.
class CloseButton final : public ParchmentControl
{
public:
    CloseButton() = default;

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    CursorId CursorShape() const override;
};

// An underlined link in a dialog's text.
class LinkButton final : public ParchmentControl
{
public:
    LinkButton() = default;

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    bool ShowsFocus() const override;
};
