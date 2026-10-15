#pragma once

#include "framework.h"
#include "ParchmentControl.h"
#include "ParchmentGeometry.h"

// A read-only text log of the parchment theme (View): the web version's sunken brown box (.log) drawn from the log
// nine-slice, with the game's scrollbar over its right side while the text overflows. It takes the place of a
// read-only multi-line EDIT control: its text is the window text (SetWindowText, GetWindowText), long lines wrap,
// the wheel, the keys and the scrollbar scroll it, and text can be selected and copied (Ctrl+C, Ctrl+A or the
// context menu). A log may lead each line by an icon, picked by the label the line starts with (.log-row).
class ParchmentLog final
{
public:
    static constexpr int MAX_TEXT_LENGTH = ParchmentFonts::MAX_MEASURED_LENGTH;

    struct Icons
    {
        const IconLabels* labels;
        ImageId atlas;
        int columns;
        int cell_size;
    };

    static bool RegisterWindowClass(HINSTANCE instance);

    bool Create(const ControlContext& context, const ControlPlacement& placement);
    bool ShowIcons(const Icons& line_icons);
    void Release();
    HWND Window() const;

private:
    enum class Drag { none, selecting, knob, scrolling };

    struct Layout
    {
        PixelRect text_area;
        bool has_scrollbar;
        int content_height;
    };

    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
    void LayOutText();
    bool WrapText(int text_width);
    void OnPaint();
    void PaintLog(ParchmentRenderer& renderer, PixelSize size);
    void PaintLines(ParchmentRenderer& renderer);
    void PaintLine(ParchmentRenderer& renderer, int line_index, float top);
    void PaintIcon(ParchmentRenderer& renderer, int line_index, float top);
    void PaintSelection(ParchmentRenderer& renderer, int line_index, float top);
    void PaintScrollbar(ParchmentRenderer& renderer);

    void OnButtonDown(LPARAM lParam);
    void OnMouseMove(WPARAM wParam, LPARAM lParam);
    void OnButtonUp();
    void OnCaptureLost();
    void OnTimer();
    void OnWheel(WPARAM wParam);
    void OnKey(WPARAM key);
    void OnContextMenu(LPARAM lParam);
    LRESULT OnSetCursor(WPARAM wParam, LPARAM lParam);

    void PressScrollbar(PixelPoint point);
    bool StepScrollbar();
    bool ScrollBy(int distance);
    int ScrollStep(ScrollbarPart part) const;
    int LineStep() const;
    int IconColumn() const;
    int TrackLength() const;
    int MaxScroll() const;
    int CharacterAt(PixelPoint point) const;
    float CharacterX(int line_index, int character) const;
    void SelectAll();
    void CopySelection() const;
    void Redraw();

    HWND window = NULL;
    HWND parent = NULL;
    ControlContext context = { nullptr, nullptr, NULL };
    PixelSize size = { 0, 0 };
    Icons icons = { nullptr, ImageId::civ_atlas, 0, 0 };
    float line_height = UI_LINE_HEIGHT;
    Layout layout = { { 0, 0, 0, 0 }, false, 0 };
    int scroll_top = 0;
    int selection_anchor = 0;
    int selection_caret = 0;
    Drag drag = Drag::none;
    ScrollbarPart pressed_part = ScrollbarPart::none;
    int pressed_y = 0;
    int drag_start_y = 0;
    int drag_start_scroll = 0;
    WrappedLines lines;
    wchar_t text[MAX_TEXT_LENGTH] = L"";
    float advances[MAX_TEXT_LENGTH] = {};
    int text_length = 0;
};
