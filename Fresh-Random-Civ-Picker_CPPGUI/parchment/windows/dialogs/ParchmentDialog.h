#pragma once

#include "framework.h"
#include "ParchmentButtons.h"
#include "ParchmentDialogControls.h"
#include "ParchmentLog.h"
#include "ParchmentShell.h"
#include "ParchmentWidgets.h"

// --- What a dialog consists of: its parts (controls, with command IDs) and its decorations (drawn once) ---

enum class PartKind
{
    checkbox, radio, button, default_button, label, error_label, slider, dropdown, text_field, readonly_field, log,
    picture, key_button
};

struct DialogPart
{
    PartKind kind;
    int id;
    PixelRect bounds;
    const wchar_t* text;
};

// Text decorations give the top of their first line; paragraphs wrap to their width.
enum class DecorationKind { text, bold_text, paragraph, hint, group, key };

struct DialogDecoration
{
    DecorationKind kind;
    PixelRect bounds;
    const wchar_t* text;
};

// A link inside a paragraph decoration: the characters it covers become a link part with this command ID.
struct DialogLink
{
    int decoration;
    const wchar_t* text;
    int id;
};

struct DialogLayout
{
    const wchar_t* title;
    PixelSize size;
    const DialogPart* parts;
    int part_count;
    const DialogDecoration* decorations;
    int decoration_count;
    const DialogLink* links;
    int link_count;
    int default_button;
};

// Runs the dialogs of the parchment theme (View): the web version's panels (dialog.win) with their title, wooden
// close button and drop shadow, over everything they dim. A dialog is an ordinary modal dialog whose procedure
// keeps its job; this class builds the dialog from its layout when the procedure gets WM_INITDIALOG, and handles
// the messages that draw and shape it. Dialogs can open over each other up to MAX_LEVELS deep; each level has its
// controls and surfaces from startup.
class ParchmentDialogs final
{
public:
    static constexpr int MAX_LEVELS = 2;
    // The largest dialogs are Hotkeys (882 wide) and History (509 tall); their layouts check they fit.
    static constexpr int MAX_DIALOG_WIDTH = 900;
    static constexpr int MAX_DIALOG_HEIGHT = 560;
    static constexpr int MAX_KEY_BUTTONS = 24; // the Customise Hotkeys dialog's, for the 24 global hotkeys
    static constexpr int SHADOW_MARGIN = 48; // room for the panel's drop shadow around a dialog

    struct Setup
    {
        HINSTANCE instance;
        ControlContext context;
        ParchmentShell* main_shell;
        ShellWorkspace* shell_workspace;
    };

    // The dialog procedure is the function pointer Win32 runs the dialog with (CLAUDE2.md I2).
    struct Run
    {
        HWND owner;
        INT_PTR(CALLBACK* procedure)(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
        LPARAM parameter;
        const DialogLayout* layout;
    };

    struct Handled
    {
        bool handled;
        INT_PTR result;
    };

    // Grows a dialog downwards: the parts from a height on (but one) move down to make room.
    struct Growth
    {
        int extra_height;
        int from_y;
        int staying_id;
    };

    bool Start(const Setup& setup);
    void Stop();
    INT_PTR RunModal(const Run& run);
    bool Build(HWND dialog);
    Handled HandleMessage(const MSG& message);
    bool Grow(HWND dialog, const Growth& growth);
    void ShowPicture(HWND dialog, const PictureButton::Picture& picture);

private:
    static constexpr int MAX_BUTTONS = 6;
    static constexpr int MAX_CHECKBOXES = 12;
    static constexpr int MAX_LABELS = 32;
    static constexpr int MAX_LINKS = 4;

    struct Level
    {
        ParchmentShell shell;
        HWND dialog;
        const DialogLayout* layout;
        PixelSize size;
        int extra_height;
        FieldLook field_look;
        DropdownLook dropdown_look;
        CloseButton close_button;
        TextButton buttons[MAX_BUTTONS];
        Checkbox checkboxes[MAX_CHECKBOXES];
        Label labels[MAX_LABELS];
        LinkButton links[MAX_LINKS];
        KeyButton key_buttons[MAX_KEY_BUTTONS];
        SliderControl slider;
        ParchmentLog log;
        PictureButton picture;
        PixelRect link_bounds[MAX_LINKS];
        int button_count;
        int checkbox_count;
        int label_count;
        int link_count;
        int key_button_count;
    };

    struct LineDraw
    {
        TextLine line;
        const DialogDecoration& decoration;
        float top;
        const float* advances;
        Level* level;
    };

    struct TextPosition
    {
        const DialogDecoration* decoration;
        int index;
    };

    Level* LevelOf(HWND dialog);
    bool PlaceDialog(Level& level);
    bool RenderPanel(Level& level);
    bool Decorate(Level& level);
    void DrawDecoration(Level& level, int index);
    void DrawParagraph(Level& level, int index);
    void DrawLine(const LineDraw& draw);
    int LinkStartingAt(const DialogLayout& layout, const TextPosition& position) const;
    void DrawGroup(const DialogDecoration& decoration);
    void DrawKey(const DialogDecoration& decoration);
    bool CreateCloseButton(Level& level);
    bool CreateParts(Level& level);
    bool CreatePart(Level& level, const DialogPart& part);
    bool CreateButtonPart(Level& level, const DialogPart& part);
    bool CreateCheckboxPart(Level& level, const DialogPart& part);
    bool CreateLabelPart(Level& level, const DialogPart& part);
    bool CreateKeyButtonPart(Level& level, const DialogPart& part);
    bool CreateWindowPart(Level& level, const DialogPart& part);
    bool CreateLinks(Level& level);
    void ReleaseLevel(Level& level);
    bool ReshapeLevel(Level& level);
    void MovePartsDown(const Level& level, const Growth& growth);
    void DimLevelsBelow(int depth);
    Handled OnWindowPosChanged(Level& level, LPARAM lParam);
    Handled OnDestroy(Level& level);
    Handled OnPaint(Level& level);
    void PaintFieldFocus(const Level& level, HDC target) const;
    Handled OnCommand(const Level& level, WPARAM wParam, LPARAM lParam) const;
    Handled OnHitTest(HWND dialog, const Level& level, LPARAM lParam);
    Handled OnSetCursor(HWND dialog, WPARAM wParam, LPARAM lParam);
    Handled OnFieldColour(WPARAM wParam) const;
    Handled OnReadOnlyFieldColour(WPARAM wParam, LPARAM lParam) const;
    Handled OnMeasureItem(HWND dialog, LPARAM lParam) const;
    Handled OnDrawItem(LPARAM lParam) const;

    HINSTANCE instance = NULL;
    ControlContext context = { nullptr, nullptr, NULL };
    ParchmentShell* main_shell = nullptr;
    HFONT field_font = NULL;
    HBRUSH field_brush = NULL;
    HBRUSH list_brush = NULL;
    Level levels[MAX_LEVELS] = {};
    WrappedLines wrapped;
    int depth = 0;
};
