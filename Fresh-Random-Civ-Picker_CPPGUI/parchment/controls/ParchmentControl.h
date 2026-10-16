#pragma once

#include "framework.h"
#include "ParchmentGeometry.h"
#include "ParchmentRenderer.h"
#include "PixelSurface.h"

// What every parchment control draws with: the renderer, the back buffer each paint goes through, and the
// Copy / Select All menu of the text logs.
struct ControlContext
{
    ParchmentRenderer* renderer;
    PixelSurface* back_buffer;
    HMENU text_menu;
};

// Where a control goes: its parent, command ID, place in the parent's client area, and the parent's backdrop,
// the image under the control (the parent's client area at the backdrop's top-left corner).
struct ControlPlacement
{
    HWND parent;
    int id;
    PixelRect bounds;
    const PixelSurface* backdrop;
    const wchar_t* text;
};

// A control of the parchment theme (View): a child window that paints itself over its parent's backdrop, the
// way the web version's elements sit on the parchment. It behaves like a Win32 button towards its parent and the
// controller: a click sends WM_COMMAND/BN_CLICKED, and BM_GETCHECK, BM_SETCHECK and BM_CLICK work as on a button.
// Each kind of control is a final subclass that draws its look (CLAUDE2.md I1: a shallow, closed hierarchy).
// Controls live in fixed pools created at startup; their windows are created once, at startup or when their
// dialog opens.
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

    Look CurrentLook() const;
    TextSpan Text(FontId font) const;
    void Check(bool checked);
    void NotifyParent(UINT message, WPARAM wParam) const;
    bool IsPressed() const;

private:
    static constexpr int MAX_BATCH = 128;
    static constexpr int MAX_SIBLINGS = 512;

    // The controls drawn in one frame, each at its place in the parent's client area. A frame costs about as
    // much as drawing a control, so every control waiting for its paint is drawn in the frame of the first one.
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
