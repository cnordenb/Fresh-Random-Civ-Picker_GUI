#pragma once

#include "framework.h"
#include "ParchmentGeometry.h"
#include "ParchmentRenderer.h"
#include "PixelSurface.h"

struct ControlContext
{
    ParchmentRenderer* renderer;
    PixelSurface* back_buffer;
    HMENU text_menu;
};

struct ControlPlacement
{
    HWND parent;
    int id;
    PixelRect bounds;
    const PixelSurface* backdrop;
    const wchar_t* text;
};

class ParchmentControl
{
public:
    static constexpr int MAX_TEXT_LENGTH = 128;

    static bool RegisterWindowClass(HINSTANCE instance);

    bool Create(const ControlContext& context, const ControlPlacement& placement);
    void Release();
    HWND Window() const;
    bool IsChecked() const;
    void SetChecked(bool checked);
    void Redraw();

protected:
    ParchmentControl() = default;
    virtual ~ParchmentControl() = default;
    ParchmentControl(const ParchmentControl&) = delete;
    ParchmentControl& operator=(const ParchmentControl&) = delete;

    struct Look
    {
        bool hot;
        bool pressed;
        bool focused;
        bool enabled;
        bool checked;
    };

    virtual void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) = 0;
    virtual bool IsClickable() const;
    virtual CursorId CursorShape() const;
    virtual void OnClicked();
    virtual LRESULT DialogCode() const;
    virtual bool ShowsFocus() const;
    virtual void OnPointerMoved(PixelPoint point);
    virtual void OnPointerPressed(PixelPoint point);
    virtual void OnPointerLeft();
    virtual LRESULT OnOtherMessage(UINT message, WPARAM wParam, LPARAM lParam);
    virtual bool NotifiesClicks() const;
    virtual bool OnKeyPressed(WPARAM key);
    virtual void OnFocusLost();

    Look CurrentLook() const;
    TextSpan Text(FontId font) const;
    void Check(bool checked);
    void NotifyParent(UINT message, WPARAM wParam) const;
    void NotifyParentLater(UINT message, WPARAM wParam) const;
    bool IsPressed() const;

private:
    static constexpr int MAX_BATCH = 128;
    static constexpr int MAX_SIBLINGS = 512;

    struct Batch
    {
        struct Entry
        {
            ParchmentControl* control;
            PixelRect area;
        };

        Entry entries[MAX_BATCH];
        int count;
        PixelRect bounds;
        PixelSize limit;

        bool Add(ParchmentControl* control);
        bool Overlaps(const PixelRect& area) const;
    };

    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    void OnPaint();
    Batch WaitingBatch();
    ParchmentControl* WaitingSibling(HWND sibling) const;
    PixelRect AreaInParent() const;
    bool RenderBatch(const Batch& batch) const;
    bool CopyBackdropTo(PixelSurface& buffer, const PixelRect& area) const;
    void PaintInFrame(ParchmentRenderer& renderer, const PixelRect& area, PixelPoint frame_origin);
    void ShowBatch(HDC target, const Batch& batch) const;
    void ShowWaitingPaint(const PixelRect& area) const;
    void ShowArea(HDC target, const PixelRect& area) const;
    void OnMouseMove(WPARAM wParam, LPARAM lParam);
    void OnMouseLeave();
    void OnButtonDown(LPARAM lParam);
    void OnButtonUp(LPARAM lParam);
    void OnCaptureLost();
    LRESULT OnSetCursor(WPARAM wParam, LPARAM lParam);
    LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
    void OnKillFocus();
    void OnKey(UINT message, WPARAM key);
    void Click();
    void ForwardMouseMove(WPARAM wParam, LPARAM lParam) const;

    HWND window = NULL;
    HWND parent = NULL;
    int id = 0;
    ControlContext context = { nullptr, nullptr, NULL };
    const PixelSurface* backdrop = nullptr;
    bool hot = false;
    bool pressed = false;
    bool checked = false;
    wchar_t text[MAX_TEXT_LENGTH] = L"";
    int text_length = 0;
};
