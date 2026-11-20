#include "ParchmentControl.h"

#include <windowsx.h>
#include <cwchar>

#include "Assertions.h"

namespace
{
    const wchar_t* const CONTROL_CLASS = L"FrcpParchmentControl";
    ATOM control_class_atom = 0;

    HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }
}

bool ParchmentControl::RegisterWindowClass(HINSTANCE instance)
{
    if (!c_assert(instance != NULL)) return false;
    WNDCLASSEXW class_info = {};
    class_info.cbSize = sizeof(class_info);
    class_info.lpfnWndProc = WindowProc;
    class_info.hInstance = instance;
    class_info.lpszClassName = CONTROL_CLASS;
    control_class_atom = RegisterClassExW(&class_info);
    return c_assert(control_class_atom != 0);
}

bool ParchmentControl::Create(const ControlContext& control_context, const ControlPlacement& placement)
{
    if (!c_assert(window == NULL) || !c_assert(placement.parent != NULL)) return false;
    if (!c_assert(control_context.renderer != nullptr && control_context.back_buffer != nullptr)) return false;
    if (!c_assert(placement.backdrop != nullptr)) return false;
    context = control_context;
    backdrop = placement.backdrop;
    parent = placement.parent;
    id = placement.id;
    const wchar_t* initial_text = placement.text != nullptr ? placement.text : L"";
    text_length = wcsncpy_s(text, initial_text, _TRUNCATE) == 0 ? static_cast<int>(wcslen(text)) : 0;
    const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parent, GWLP_HINSTANCE));
    const PixelRect& bounds = placement.bounds;
    const DWORD style = static_cast<DWORD>(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | (IsClickable() ? WS_TABSTOP : 0));
    window = CreateWindowExW(0, CONTROL_CLASS, initial_text, style,
                             bounds.left, bounds.top, RectWidth(bounds), RectHeight(bounds), parent, ControlId(id),
                             instance, this);
    return c_assert(window != NULL);
}

void ParchmentControl::Release()
{
    window = NULL;
    parent = NULL;
    hot = false;
    pressed = false;
    checked = false;
    text[0] = L'\0';
    text_length = 0;
}

HWND ParchmentControl::Window() const { return window; }

bool ParchmentControl::IsChecked() const { return checked; }

void ParchmentControl::SetChecked(bool now_checked) { Check(now_checked); }

void ParchmentControl::Check(bool now_checked)
{
    if (checked == now_checked) return;
    checked = now_checked;
    Redraw();
}

void ParchmentControl::Redraw()
{
    if (window == NULL) return;
    (void)InvalidateRect(window, nullptr, FALSE);
}

bool ParchmentControl::IsClickable() const { return true; }

CursorId ParchmentControl::CursorShape() const { return IsClickable() ? CursorId::hand : CursorId::arrow; }

void ParchmentControl::OnClicked() {}

LRESULT ParchmentControl::DialogCode() const
{
    return IsClickable() ? DLGC_BUTTON | DLGC_UNDEFPUSHBUTTON : DLGC_STATIC;
}

bool ParchmentControl::ShowsFocus() const { return IsClickable(); }

void ParchmentControl::OnPointerMoved(PixelPoint) {}

void ParchmentControl::OnPointerPressed(PixelPoint) {}

void ParchmentControl::OnPointerLeft() {}

LRESULT ParchmentControl::OnOtherMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(window != NULL)) return 0;
    return DefWindowProc(window, message, wParam, lParam);
}

bool ParchmentControl::NotifiesClicks() const { return true; }

bool ParchmentControl::OnKeyPressed(WPARAM) { return false; }

void ParchmentControl::OnFocusLost() {}

bool ParchmentControl::IsPressed() const { return pressed; }

void ParchmentControl::NotifyParent(UINT message, WPARAM wParam) const
{
    if (!c_assert(parent != NULL) || !c_assert(window != NULL)) return;
    (void)SendMessage(parent, message, wParam, reinterpret_cast<LPARAM>(window));
}

void ParchmentControl::NotifyParentLater(UINT message, WPARAM wParam) const
{
    if (!c_assert(parent != NULL) || !c_assert(window != NULL)) return;
    (void)c_assert(PostMessage(parent, message, wParam, reinterpret_cast<LPARAM>(window)) != FALSE);
}

ParchmentControl::Look ParchmentControl::CurrentLook() const
{
    if (!c_assert(window != NULL)) return { false, false, false, false, false };
    const LRESULT ui_state = parent != NULL ? SendMessage(parent, WM_QUERYUISTATE, 0, 0) : 0;
    const bool focus_cues = (ui_state & UISF_HIDEFOCUS) == 0;
    const bool focused = ShowsFocus() && focus_cues && GetFocus() == window;
    return { hot, pressed, focused, IsWindowEnabled(window) == TRUE, checked };
}

TextSpan ParchmentControl::Text(FontId font) const
{
    if (!c_assert(static_cast<int>(font) < FONT_COUNT)) return { text, 0, FontId::ui };
    if (!c_assert(text_length >= 0 && text_length < MAX_TEXT_LENGTH)) return { text, 0, font };
    return { text, text_length, font };
}

LRESULT CALLBACK ParchmentControl::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        const CREATESTRUCT* create_struct = reinterpret_cast<const CREATESTRUCT*>(lParam);
        if (create_struct == nullptr) return FALSE;
        (void)SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
    }
    ParchmentControl* control = reinterpret_cast<ParchmentControl*>(GetWindowLongPtr(window, GWLP_USERDATA));
    if (control == nullptr) return DefWindowProc(window, message, wParam, lParam);
    if (message == WM_NCCREATE) control->window = window;
    if (control->window != window) return DefWindowProc(window, message, wParam, lParam);
    return control->HandleMessage(message, wParam, lParam);
}

LRESULT ParchmentControl::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(window != NULL)) return 0;
    switch (message)
    {
        case WM_PAINT: OnPaint(); return 0;
        case WM_ERASEBKGND: return 1;
        case WM_MOUSEMOVE: OnMouseMove(wParam, lParam); return 0;
        case WM_MOUSELEAVE: OnMouseLeave(); return 0;
        case WM_LBUTTONDOWN: OnButtonDown(lParam); return 0;
        case WM_LBUTTONUP: OnButtonUp(lParam); return 0;
        case WM_CAPTURECHANGED: OnCaptureLost(); return 0;
        case WM_SETCURSOR: return OnSetCursor(wParam, lParam);
        case WM_SETTEXT: return OnSetText(wParam, lParam);
        case WM_KEYDOWN:
        case WM_KEYUP: OnKey(message, wParam); return 0;
        case WM_KILLFOCUS: OnKillFocus(); return 0;
        case WM_SETFOCUS:
        case WM_ENABLE: Redraw(); return 0;
        case WM_GETDLGCODE: return DialogCode();
        case BM_GETCHECK: return checked ? BST_CHECKED : BST_UNCHECKED;
        case BM_SETCHECK: Check(wParam == BST_CHECKED); return 0;
        case BM_GETSTATE: return (checked ? BST_CHECKED : 0) | (pressed ? BST_PUSHED : 0);
        case BM_CLICK: Click(); return 0;
        default: return OnOtherMessage(message, wParam, lParam);
    }
}

void ParchmentControl::OnPaint()
{
    if (!c_assert(window != NULL)) return;
    PAINTSTRUCT paint = {};
    const HDC target = BeginPaint(window, &paint);
    if (target != NULL)
    {
        const Batch batch = WaitingBatch();
        if (RenderBatch(batch)) ShowBatch(target, batch);
    }
    (void)EndPaint(window, &paint);
}

ParchmentControl::Batch ParchmentControl::WaitingBatch()
{
    Batch batch = {};
    batch.limit = context.back_buffer != nullptr ? context.back_buffer->Size() : PixelSize{ 0, 0 };
    if (!c_assert(batch.Add(this))) return batch;
    HWND sibling = GetWindow(parent, GW_CHILD);
    for (int i = 0; i < MAX_SIBLINGS && sibling != NULL; i++)
    {
        ParchmentControl* control = WaitingSibling(sibling);
        if (control != nullptr) (void)batch.Add(control);
        sibling = GetWindow(sibling, GW_HWNDNEXT);
    }
    (void)c_assert(sibling == NULL);
    return batch;
}

ParchmentControl* ParchmentControl::WaitingSibling(HWND sibling) const
{
    if (!c_assert(sibling != NULL) || sibling == window) return nullptr;
    const bool is_control = static_cast<ATOM>(GetClassLongPtrW(sibling, GCW_ATOM)) == control_class_atom;
    if (!is_control || !IsWindowVisible(sibling) || !GetUpdateRect(sibling, nullptr, FALSE)) return nullptr;
    ParchmentControl* control = reinterpret_cast<ParchmentControl*>(GetWindowLongPtr(sibling, GWLP_USERDATA));
    if (control == nullptr || control->context.back_buffer != context.back_buffer) return nullptr;
    return control->context.renderer == context.renderer ? control : nullptr;
}

PixelRect ParchmentControl::AreaInParent() const
{
    if (!c_assert(window != NULL && parent != NULL)) return { 0, 0, 0, 0 };
    RECT client = {};
    POINT origin = { 0, 0 };
    if (!c_assert(GetClientRect(window, &client) != FALSE)) return { 0, 0, 0, 0 };
    (void)MapWindowPoints(window, parent, &origin, 1);
    return { origin.x, origin.y, origin.x + client.right, origin.y + client.bottom };
}

bool ParchmentControl::Batch::Add(ParchmentControl* control)
{
    if (!c_assert(control != nullptr) || count >= MAX_BATCH) return false;
    const PixelRect area = control->AreaInParent();
    const bool inside = area.left >= 0 && area.top >= 0 && area.right <= limit.width && area.bottom <= limit.height;
    if (RectWidth(area) <= 0 || RectHeight(area) <= 0 || !inside || Overlaps(area)) return false;
    entries[count] = { control, area };
    bounds = count == 0 ? area : BoundingRect(bounds, area);
    count++;
    return true;
}

bool ParchmentControl::Batch::Overlaps(const PixelRect& area) const
{
    for (int i = 0; i < count && i < MAX_BATCH; i++)
    {
        if (RectsOverlap(entries[i].area, area)) return true;
    }
    return false;
}

bool ParchmentControl::RenderBatch(const Batch& batch) const
{
    PixelSurface* buffer = context.back_buffer;
    ParchmentRenderer* renderer = context.renderer;
    if (!c_assert(buffer != nullptr && renderer != nullptr) || !c_assert(batch.count > 0)) return false;
    for (int i = 0; i < batch.count && i < MAX_BATCH; i++)
    {
        if (!batch.entries[i].control->CopyBackdropTo(*buffer, batch.entries[i].area)) return false;
    }
    const PixelPoint origin = { batch.bounds.left, batch.bounds.top };
    const PixelSize size = { RectWidth(batch.bounds), RectHeight(batch.bounds) };
    if (!renderer->BeginFrame({ buffer->Dc(), size, ParchmentRenderer::SurfaceAlpha::opaque, origin })) return false;
    for (int i = 0; i < batch.count && i < MAX_BATCH; i++)
    {
        batch.entries[i].control->PaintInFrame(*renderer, batch.entries[i].area, origin);
    }
    return renderer->EndFrame();
}

bool ParchmentControl::CopyBackdropTo(PixelSurface& buffer, const PixelRect& area) const
{
    if (!c_assert(backdrop != nullptr) || !c_assert(buffer.Contains(area))) return false;
    return c_assert(BitBlt(buffer.Dc(), area.left, area.top, RectWidth(area), RectHeight(area), backdrop->Dc(),
                           area.left, area.top, SRCCOPY) != FALSE);
}

void ParchmentControl::PaintInFrame(ParchmentRenderer& renderer, const PixelRect& area, PixelPoint frame_origin)
{
    if (!c_assert(RectWidth(area) > 0 && RectHeight(area) > 0)) return;
    const FloatRect bounds = { 0.0f, 0.0f, static_cast<float>(RectWidth(area)), static_cast<float>(RectHeight(area)) };
    renderer.SetOrigin({ area.left - frame_origin.x, area.top - frame_origin.y });
    renderer.PushClip(bounds);
    Paint(renderer, bounds);
    renderer.PopClip();
}

void ParchmentControl::ShowBatch(HDC target, const Batch& batch) const
{
    if (!c_assert(target != NULL) || !c_assert(batch.count > 0)) return;
    ShowArea(target, batch.entries[0].area);
    for (int i = 1; i < batch.count && i < MAX_BATCH; i++)
    {
        batch.entries[i].control->ShowWaitingPaint(batch.entries[i].area);
    }
}

void ParchmentControl::ShowWaitingPaint(const PixelRect& area) const
{
    if (!c_assert(window != NULL) || !c_assert(HasArea(area))) return;
    const HDC target = GetDC(window);
    if (!c_assert(target != NULL)) return;
    ShowArea(target, area);
    (void)ReleaseDC(window, target);
    (void)ValidateRect(window, nullptr);
}

void ParchmentControl::ShowArea(HDC target, const PixelRect& area) const
{
    const PixelSurface* buffer = context.back_buffer;
    if (!c_assert(buffer != nullptr) || !c_assert(target != NULL)) return;
    if (!c_assert(HasArea(area) && buffer->Contains(area))) return;
    (void)BitBlt(target, 0, 0, RectWidth(area), RectHeight(area), buffer->Dc(), area.left, area.top, SRCCOPY);
}

void ParchmentControl::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(window != NULL)) return;
    OnPointerMoved({ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
    if (!hot)
    {
        hot = true;
        TRACKMOUSEEVENT tracking = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, window, 0 };
        (void)TrackMouseEvent(&tracking);
        Redraw();
    }
    ForwardMouseMove(wParam, lParam);
}

void ParchmentControl::ForwardMouseMove(WPARAM wParam, LPARAM lParam) const
{
    if (!c_assert(parent != NULL) || !c_assert(window != NULL)) return;
    POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    (void)MapWindowPoints(window, parent, &point, 1);
    (void)SendMessage(parent, WM_MOUSEMOVE, wParam, MAKELPARAM(point.x, point.y));
}

void ParchmentControl::OnMouseLeave()
{
    if (!c_assert(window != NULL)) return;
    hot = false;
    OnPointerLeft();
    Redraw();
}

void ParchmentControl::OnButtonDown(LPARAM lParam)
{
    if (!IsClickable() || !c_assert(window != NULL)) return;
    OnPointerPressed({ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
    pressed = true;
    (void)SetCapture(window);
    Redraw();
}

void ParchmentControl::OnButtonUp(LPARAM lParam)
{
    if (!c_assert(window != NULL)) return;
    if (!pressed || GetCapture() != window) return;
    const POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    RECT client = {};
    const bool inside = GetClientRect(window, &client) && PtInRect(&client, point);
    (void)ReleaseCapture();
    if (inside) Click();
}

void ParchmentControl::OnCaptureLost()
{
    if (!c_assert(window != NULL)) return;
    if (!pressed) return;
    pressed = false;
    Redraw();
}

LRESULT ParchmentControl::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(window != NULL)) return FALSE;
    const HCURSOR cursor = context.renderer != nullptr ? context.renderer->Cursor(CursorShape()) : NULL;
    if (LOWORD(lParam) != HTCLIENT || cursor == NULL) return DefWindowProc(window, WM_SETCURSOR, wParam, lParam);
    (void)SetCursor(cursor);
    return TRUE;
}

LRESULT ParchmentControl::OnSetText(WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(window != NULL)) return FALSE;
    const LRESULT result = DefWindowProc(window, WM_SETTEXT, wParam, lParam);
    const wchar_t* new_text = reinterpret_cast<const wchar_t*>(lParam);
    if (!c_assert(new_text != nullptr)) return result;
    text_length = wcsncpy_s(text, new_text, _TRUNCATE) == 0 ? static_cast<int>(wcslen(text)) : 0;
    Redraw();
    return result;
}

void ParchmentControl::OnKillFocus()
{
    if (!c_assert(window != NULL)) return;
    OnFocusLost();
    Redraw();
}

void ParchmentControl::OnKey(UINT message, WPARAM key)
{
    if (!c_assert(message == WM_KEYDOWN || message == WM_KEYUP)) return;
    if (message == WM_KEYDOWN && OnKeyPressed(key)) return;
    if (key != VK_SPACE || !IsClickable()) return;
    if (message == WM_KEYDOWN)
    {
        pressed = true;
        Redraw();
        return;
    }
    if (!pressed) return;
    pressed = false;
    Click();
}

void ParchmentControl::Click()
{
    if (!c_assert(window != NULL) || !IsClickable()) return;
    OnClicked();
    Redraw();
    if (NotifiesClicks()) NotifyParent(WM_COMMAND, MAKEWPARAM(id, BN_CLICKED));
}
