#include "ParchmentDialog.h"

#include <cmath>
#include <commctrl.h>
#include <cwchar>
#include <windowsx.h>

#include "Assertions.h"

namespace
{
    // dialog.win { padding: 13px } with its 30 px title row (.dlg-title { padding-left: 10px }) and 46 px close
    // button, the panel nine-slice drawn 38 px wide (--slice) with its middle stretched under it, and the
    // drop shadow's room around the panel.
    constexpr int PANEL_PADDING = 13;
    constexpr int TITLE_HEIGHT = 30;
    constexpr int TITLE_LEFT = 10;
    constexpr float PANEL_BORDER = 38.0f;
    // .group { border: 1px } and its legend { padding: 0 3px }, 11 px in from the group's edge.
    constexpr int LEGEND_LEFT = 11;
    constexpr float LEGEND_PADDING = 3.0f;
    // kbd.key { border: 1px #b4b4b4; border-bottom-width: 2px; border-radius: 3px; padding: 0 4px }
    constexpr float KEY_RADIUS = 3.0f;
    constexpr float KEY_BORDER = 1.0f;
    constexpr float KEY_BOTTOM_BORDER = 2.0f;

    struct EmptyDialogTemplate
    {
        DLGTEMPLATE header;
        WORD menu;
        WORD window_class;
        WORD title;
        WORD padding;
    };

    // Every dialog starts empty; its layout is built when it opens.
    alignas(4) const EmptyDialogTemplate EMPTY_DIALOG = { { WS_POPUP | WS_CLIPCHILDREN, 0, 0, 0, 0, 100, 100 },
                                                         0, 0, 0, 0 };

    HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }

    int TextLength(const wchar_t* text) { return text != nullptr ? static_cast<int>(wcslen(text)) : 0; }

    ParchmentDialogs::Handled Unhandled() { return { false, FALSE }; }

    // The child's place in its parent's client area.
    PixelRect AreaInParent(HWND child, HWND parent)
    {
        if (!c_assert(child != NULL && parent != NULL)) return { 0, 0, 0, 0 };
        RECT client = {};
        POINT origin = { 0, 0 };
        if (!c_assert(GetClientRect(child, &client) != FALSE)) return { 0, 0, 0, 0 };
        // MapWindowPoints returns the offset it applied, which can rightly be zero.
        (void)MapWindowPoints(child, parent, &origin, 1);
        return { origin.x, origin.y, origin.x + client.right, origin.y + client.bottom };
    }

    bool IsTextField(HWND control)
    {
        if (!c_assert(control != NULL)) return false;
        wchar_t class_name[16] = L"";
        return GetClassNameW(control, class_name, 16) > 0 && _wcsicmp(class_name, WC_EDITW) == 0;
    }

    ParchmentDialogs::Handled HandledWith(HWND dialog, LRESULT result)
    {
        if (!c_assert(dialog != NULL)) return Unhandled();
        // A dialog procedure hands message results back through DWLP_MSGRESULT.
        (void)SetWindowLongPtr(dialog, DWLP_MSGRESULT, result);
        return { true, TRUE };
    }
}

bool ParchmentDialogs::Start(const Setup& setup)
{
    if (!c_assert(setup.instance != NULL && setup.main_shell != nullptr)) return false;
    if (!c_assert(setup.shell_workspace != nullptr)) return false;
    if (!c_assert(setup.context.renderer != nullptr))
        return false;
    instance = setup.instance;
    context = setup.context;
    main_shell = setup.main_shell;
    field_font = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Georgia");
    field_brush = CreateSolidBrush(RGB(FIELD_COLOUR.red, FIELD_COLOUR.green, FIELD_COLOUR.blue));
    list_brush = CreateSolidBrush(RGB(MENU_COLOUR.red, MENU_COLOUR.green, MENU_COLOUR.blue));
    if (!c_assert(field_font != NULL && field_brush != NULL && list_brush != NULL)) return false;
    for (Level& level : levels)
    {
        if (!level.shell.Create({ { MAX_DIALOG_WIDTH, MAX_DIALOG_HEIGHT }, SHADOW_MARGIN, setup.shell_workspace }))
            return false;
    }
    return true;
}

void ParchmentDialogs::Stop()
{
    for (Level& level : levels) level.shell.Destroy();
    // Freeing at shutdown: a failure leaves nothing to recover.
    if (field_font != NULL) (void)DeleteObject(field_font);
    if (field_brush != NULL) (void)DeleteObject(field_brush);
    if (list_brush != NULL) (void)DeleteObject(list_brush);
    field_font = NULL;
    field_brush = NULL;
    list_brush = NULL;
}

// Opens a dialog over its owner and returns once it's closed, with what its procedure ended it with.
INT_PTR ParchmentDialogs::RunModal(const Run& run)
{
    if (!c_assert(depth < MAX_LEVELS) || !c_assert(run.layout != nullptr && run.procedure != nullptr)) return -1;
    Level& level = levels[depth];
    level.layout = run.layout;
    level.dialog = NULL;
    depth++;
    const INT_PTR result = DialogBoxIndirectParamW(instance, &EMPTY_DIALOG.header, run.owner, run.procedure,
                                                   run.parameter);
    depth--;
    DimLevelsBelow(depth);
    return result;
}

// Called by the dialog procedure on WM_INITDIALOG: sizes, shapes and fills the dialog from its layout.
bool ParchmentDialogs::Build(HWND dialog)
{
    if (!c_assert(depth > 0) || !c_assert(dialog != NULL)) return false;
    Level& level = levels[depth - 1];
    if (!c_assert(level.dialog == NULL && level.layout != nullptr)) return false;
    level.dialog = dialog;
    level.size = level.layout->size;
    level.extra_height = 0;
    level.button_count = 0;
    level.checkbox_count = 0;
    level.label_count = 0;
    level.link_count = 0;
    level.field_look = { context, field_font, nullptr };
    level.dropdown_look = { context, &level.shell.Backdrop() };
    if (!PlaceDialog(level) || !RenderPanel(level) || !level.shell.Attach(dialog, level.size)) return false;
    // The close button comes last in the tab order, so a dialog starts with its first part focused.
    if (!Decorate(level) || !CreateParts(level) || !CreateLinks(level) || !CreateCloseButton(level)) return false;
    // Posted rather than sent, so the dialog takes it after it has finished starting up.
    return c_assert(PostMessage(dialog, DM_SETDEFID, static_cast<WPARAM>(level.layout->default_button), 0) != FALSE);
}

ParchmentDialogs::Level* ParchmentDialogs::LevelOf(HWND dialog)
{
    if (!c_assert(depth >= 0 && depth <= MAX_LEVELS)) return nullptr;
    for (int i = 0; i < depth && i < MAX_LEVELS; i++)
    {
        if (levels[i].dialog == dialog) return &levels[i];
    }
    return nullptr;
}

// Centres the dialog over its owner, as a browser centres a modal dialog in the page.
bool ParchmentDialogs::PlaceDialog(Level& level)
{
    if (!c_assert(level.dialog != NULL)) return false;
    const PixelSize size = level.size;
    if (!c_assert(size.width > 0 && size.width <= MAX_DIALOG_WIDTH && size.height <= MAX_DIALOG_HEIGHT)) return false;
    RECT owner_rect = {};
    const HWND owner = GetWindow(level.dialog, GW_OWNER);
    if (owner == NULL || !GetWindowRect(owner, &owner_rect)) return false;
    const int left = (owner_rect.left + owner_rect.right - size.width) / 2;
    const int top = (owner_rect.top + owner_rect.bottom - size.height) / 2;
    return c_assert(SetWindowPos(level.dialog, NULL, left, top, size.width, size.height,
                                 SWP_NOZORDER | SWP_NOACTIVATE) != FALSE);
}

// The panel (dialog.win::before and ::after): the nine-slice's middle stretched over the inlay, then its corners and
// edges, tiles rounded to fit.
bool ParchmentDialogs::RenderPanel(Level& level)
{
    if (!c_assert(context.renderer != nullptr)) return false;
    if (!c_assert(level.size.width > 0 && level.size.height > 0)) return false;
    PixelSurface& canvas = level.shell.Canvas();
    const PixelPoint origin = level.shell.WindowOrigin();
    const PixelSize canvas_size = { level.size.width + 2 * origin.x, level.size.height + 2 * origin.y };
    ParchmentRenderer& renderer = *context.renderer;
    if (!renderer.BeginFrame({ canvas.Dc(), canvas_size, ParchmentRenderer::SurfaceAlpha::premultiplied }))
        return false;
    renderer.Clear();
    const FloatRect panel =
        ToFloatRect({ origin.x, origin.y, origin.x + level.size.width, origin.y + level.size.height });
    const float inlay = PANEL_BORDER / 2.0f;
    const PixelSize middle = renderer.ImageSize(ImageId::panel_inlay);
    renderer.DrawImage({ ImageId::panel_inlay, { 0, 0, middle.width, middle.height },
                         { panel.left + inlay, panel.top + inlay, panel.right - inlay, panel.bottom - inlay }, 1.0f,
                         ParchmentRenderer::Smoothing::smooth });
    renderer.DrawNineSlice({ NineSliceId::panel_frame, panel, PANEL_BORDER, TileRule::round, TileRule::round, false });
    return renderer.EndFrame();
}

// Draws the title and the layout's decorations onto the dialog's backdrop, under its controls.
bool ParchmentDialogs::Decorate(Level& level)
{
    if (!c_assert(context.renderer != nullptr) || !c_assert(level.layout != nullptr)) return false;
    ParchmentRenderer& renderer = *context.renderer;
    PixelSurface& backdrop = level.shell.Backdrop();
    if (!renderer.BeginFrame({ backdrop.Dc(), level.size, ParchmentRenderer::SurfaceAlpha::opaque })) return false;
    const DialogLayout& layout = *level.layout;
    const float title_top = PANEL_PADDING + (TITLE_HEIGHT - DIALOG_LINE_HEIGHT) / 2.0f;
    renderer.DrawText({ { layout.title, TextLength(layout.title), FontId::ui }, TEXT_COLOUR,
                        static_cast<float>(PANEL_PADDING + TITLE_LEFT), static_cast<float>(level.size.width),
                        renderer.BaselineInLine(FontId::ui, title_top, DIALOG_LINE_HEIGHT), TextAlign::left });
    for (int i = 0; i < layout.decoration_count; i++) DrawDecoration(level, i);
    for (int i = 0; i < layout.part_count; i++)
    {
        const PartKind kind = layout.parts[i].kind;
        if (kind == PartKind::text_field || kind == PartKind::readonly_field)
            PaintTextFieldFrame(renderer, layout.parts[i].bounds);
    }
    return renderer.EndFrame();
}

void ParchmentDialogs::DrawDecoration(Level& level, int index)
{
    if (!c_assert(level.layout != nullptr)) return;
    const DialogLayout& layout = *level.layout;
    if (!c_assert(index >= 0 && index < layout.decoration_count)) return;
    const DialogDecoration& decoration = layout.decorations[index];
    switch (decoration.kind)
    {
        case DecorationKind::text:
        case DecorationKind::bold_text:
        case DecorationKind::paragraph:
        case DecorationKind::hint: DrawParagraph(level, index); return;
        case DecorationKind::group: DrawGroup(decoration); return;
        case DecorationKind::key: DrawKey(decoration); return;
    }
}

// Wraps text to the decoration's width and draws it line by line, leaving out the words that are links: those
// become link parts placed where the words would be.
void ParchmentDialogs::DrawParagraph(Level& level, int index)
{
    if (!c_assert(level.layout != nullptr) || !c_assert(context.renderer != nullptr)) return;
    const DialogLayout& layout = *level.layout;
    if (!c_assert(index >= 0 && index < layout.decoration_count)) return;
    const DialogDecoration& decoration = layout.decorations[index];
    const FontId font = decoration.kind == DecorationKind::bold_text ? FontId::ui_bold : FontId::ui;
    const TextSpan span = { decoration.text, TextLength(decoration.text), font };
    const float* advances = context.renderer->MeasureAdvances(span);
    if (!wrapped.Wrap({ span.text, advances, span.length, static_cast<float>(RectWidth(decoration.bounds)) })) return;
    for (int line = 0; line < wrapped.Count() && line < MAX_WRAPPED_LINES; line++)
    {
        const float top = static_cast<float>(decoration.bounds.top) + static_cast<float>(line) * DIALOG_LINE_HEIGHT;
        DrawLine({ wrapped.Line(line), decoration, top, advances, &level });
    }
}

// Draws one wrapped line in pieces around the links it holds, and records where those links are.
void ParchmentDialogs::DrawLine(const LineDraw& draw)
{
    if (!c_assert(draw.level != nullptr) || !c_assert(draw.advances != nullptr)) return;
    Level& level = *draw.level;
    if (!c_assert(level.layout != nullptr)) return;
    const DialogLayout& layout = *level.layout;
    const DialogDecoration& decoration = draw.decoration;
    const FontId font = decoration.kind == DecorationKind::bold_text ? FontId::ui_bold : FontId::ui;
    const Colour colour = decoration.kind == DecorationKind::hint ? HINT_COLOUR : TEXT_COLOUR;
    const float baseline = context.renderer->BaselineInLine(font, draw.top, DIALOG_LINE_HEIGHT);
    int piece_start = draw.line.start;
    float x = static_cast<float>(decoration.bounds.left);
    float piece_x = x;
    const int line_end = draw.line.start + draw.line.length;
    for (int i = draw.line.start; i <= line_end && i <= ParchmentFonts::MAX_MEASURED_LENGTH; i++)
    {
        const int link = LinkStartingAt(layout, { &decoration, i });
        if (i < line_end && link < 0)
        {
            x += draw.advances[i];
            continue;
        }
        context.renderer->DrawText({ { decoration.text + piece_start, i - piece_start, font }, colour, piece_x,
                                     static_cast<float>(decoration.bounds.right), baseline, TextAlign::left });
        if (link < 0) break;
        const int link_length = TextLength(layout.links[link].text);
        float link_width = 0.0f;
        for (int j = i; j < i + link_length && j < line_end; j++) link_width += draw.advances[j];
        level.link_bounds[link] = { static_cast<int>(std::floor(x)), static_cast<int>(std::floor(draw.top)),
                                    static_cast<int>(std::ceil(x + link_width)),
                                    static_cast<int>(std::ceil(draw.top + DIALOG_LINE_HEIGHT)) };
        x += link_width;
        i += link_length - 1;
        piece_start = i + 1;
        piece_x = x;
    }
}

// The index of the layout link that starts at this character of this decoration, or -1.
int ParchmentDialogs::LinkStartingAt(const DialogLayout& layout, const TextPosition& position) const
{
    if (!c_assert(position.decoration != nullptr) || !c_assert(position.index >= 0)) return -1;
    for (int i = 0; i < layout.link_count && i < MAX_LINKS; i++)
    {
        const DialogLink& link = layout.links[i];
        if (&layout.decorations[link.decoration] != position.decoration) continue;
        const wchar_t* found = wcsstr(position.decoration->text, link.text);
        if (found != nullptr && found - position.decoration->text == position.index) return i;
    }
    return -1;
}

// A fieldset: a 1 px frame whose top runs through the middle of its legend, broken where the legend is.
void ParchmentDialogs::DrawGroup(const DialogDecoration& decoration)
{
    if (!c_assert(context.renderer != nullptr) || !c_assert(HasArea(decoration.bounds))) return;
    ParchmentRenderer& renderer = *context.renderer;
    const FloatRect frame = ToFloatRect(decoration.bounds);
    const TextSpan legend = { decoration.text, TextLength(decoration.text), FontId::ui };
    const float legend_left = frame.left + LEGEND_LEFT;
    const float legend_right = legend_left + renderer.MeasureText(legend) + 2.0f * LEGEND_PADDING;
    const float line = frame.top + std::floor(DIALOG_LINE_HEIGHT / 2.0f);
    renderer.FillRect({ frame.left, line, legend_left, line + 1.0f }, GROUP_FRAME_COLOUR);
    renderer.FillRect({ legend_right, line, frame.right, line + 1.0f }, GROUP_FRAME_COLOUR);
    renderer.FillRect({ frame.left, line + 1.0f, frame.left + 1.0f, frame.bottom }, GROUP_FRAME_COLOUR);
    renderer.FillRect({ frame.right - 1.0f, line + 1.0f, frame.right, frame.bottom }, GROUP_FRAME_COLOUR);
    renderer.FillRect({ frame.left + 1.0f, frame.bottom - 1.0f, frame.right - 1.0f, frame.bottom }, GROUP_FRAME_COLOUR);
    const float baseline = renderer.BaselineInLine(FontId::ui, frame.top, DIALOG_LINE_HEIGHT);
    renderer.DrawText({ legend, TEXT_COLOUR, legend_left + LEGEND_PADDING, legend_right, baseline, TextAlign::left });
}

// A key of the Hotkeys dialog (kbd.key): a small rounded tan box with the key's name.
void ParchmentDialogs::DrawKey(const DialogDecoration& decoration)
{
    if (!c_assert(context.renderer != nullptr) || !c_assert(HasArea(decoration.bounds))) return;
    ParchmentRenderer& renderer = *context.renderer;
    const FloatRect box = ToFloatRect(decoration.bounds);
    renderer.FillRoundedRect(box, KEY_RADIUS, KEY_BORDER_COLOUR);
    renderer.FillRoundedRect({ box.left + KEY_BORDER, box.top + KEY_BORDER, box.right - KEY_BORDER,
                               box.bottom - KEY_BOTTOM_BORDER },
                             KEY_RADIUS - KEY_BORDER, KEY_COLOUR);
    const float baseline = renderer.BaselineInLine(FontId::key, box.top + KEY_BORDER, KEY_LINE_HEIGHT);
    renderer.DrawText({ { decoration.text, TextLength(decoration.text), FontId::key }, TOOLTIP_TEXT_COLOUR, box.left,
                        box.right, baseline, TextAlign::centre });
}

bool ParchmentDialogs::CreateCloseButton(Level& level)
{
    if (!c_assert(level.dialog != NULL)) return false;
    if (!c_assert(level.size.width > PANEL_PADDING + CLOSE_BUTTON_WIDTH)) return false;
    const ControlPlacement close = { level.dialog, IDCANCEL,
                                     { level.size.width - PANEL_PADDING - CLOSE_BUTTON_WIDTH, PANEL_PADDING,
                                       level.size.width - PANEL_PADDING, PANEL_PADDING + TITLE_HEIGHT },
                                     &level.shell.Backdrop(), L"" };
    return level.close_button.Create(context, close);
}

bool ParchmentDialogs::CreateParts(Level& level)
{
    if (!c_assert(level.layout != nullptr)) return false;
    const DialogLayout& layout = *level.layout;
    if (!c_assert(layout.part_count >= 0 && (layout.part_count == 0 || layout.parts != nullptr))) return false;
    for (int i = 0; i < layout.part_count; i++)
    {
        if (!CreatePart(level, layout.parts[i])) return false;
    }
    return true;
}

bool ParchmentDialogs::CreatePart(Level& level, const DialogPart& part)
{
    if (!c_assert(level.dialog != NULL) || !c_assert(HasArea(part.bounds))) return false;
    switch (part.kind)
    {
        case PartKind::button:
        case PartKind::default_button: return CreateButtonPart(level, part);
        case PartKind::checkbox:
        case PartKind::radio: return CreateCheckboxPart(level, part);
        case PartKind::label:
        case PartKind::error_label: return CreateLabelPart(level, part);
        case PartKind::slider:
        case PartKind::log:
        case PartKind::picture:
        case PartKind::dropdown:
        case PartKind::text_field:
        case PartKind::readonly_field: return CreateWindowPart(level, part);
    }
    return false;
}

bool ParchmentDialogs::CreateButtonPart(Level& level, const DialogPart& part)
{
    if (!c_assert(level.button_count < MAX_BUTTONS) || !c_assert(part.text != nullptr)) return false;
    TextButton& button = level.buttons[level.button_count];
    level.button_count++;
    return button.Create(context, { level.dialog, part.id, part.bounds, &level.shell.Backdrop(), part.text });
}

bool ParchmentDialogs::CreateCheckboxPart(Level& level, const DialogPart& part)
{
    if (!c_assert(level.checkbox_count < MAX_CHECKBOXES) || !c_assert(part.text != nullptr)) return false;
    Checkbox& checkbox = level.checkboxes[level.checkbox_count];
    level.checkbox_count++;
    // .row { gap: 6px } in a dialog, whose text has line-height 1.4.
    checkbox.SetStyle({ 6, part.kind == PartKind::radio, DIALOG_LINE_HEIGHT });
    return checkbox.Create(context, { level.dialog, part.id, part.bounds, &level.shell.Backdrop(), part.text });
}

bool ParchmentDialogs::CreateLabelPart(Level& level, const DialogPart& part)
{
    if (!c_assert(level.label_count < MAX_LABELS)) return false;
    if (!c_assert(part.kind == PartKind::label || part.kind == PartKind::error_label)) return false;
    Label& label = level.labels[level.label_count];
    level.label_count++;
    const bool error = part.kind == PartKind::error_label;
    label.SetStyle({ FontId::ui, error ? ERROR_COLOUR : TEXT_COLOUR, TextAlign::left, DIALOG_LINE_HEIGHT });
    const ControlPlacement placement = { level.dialog, part.id, part.bounds, &level.shell.Backdrop(), part.text };
    if (!label.Create(context, placement)) return false;
    // An error message waits hidden until the dialog has an error to show.
    if (error) (void)ShowWindow(label.Window(), SW_HIDE);
    return true;
}

// The parts that are one of a kind in a dialog, or standard controls in the parchment look.
bool ParchmentDialogs::CreateWindowPart(Level& level, const DialogPart& part)
{
    if (!c_assert(level.dialog != NULL) || !c_assert(HasArea(part.bounds))) return false;
    const ControlPlacement placement = { level.dialog, part.id, part.bounds, &level.shell.Backdrop(), part.text };
    if (part.kind == PartKind::slider) return level.slider.Create(context, placement);
    if (part.kind == PartKind::log) return level.log.Create(context, placement);
    if (part.kind == PartKind::picture) return level.picture.Create(context, placement);
    const bool dropdown = part.kind == PartKind::dropdown;
    const PixelRect area = dropdown ? part.bounds : TextFieldEditArea(part.bounds);
    const LONG field_style = part.kind == PartKind::readonly_field ? (ES_AUTOHSCROLL | ES_READONLY) : ES_AUTOHSCROLL;
    const LONG kind_style =
        dropdown ? (WS_VSCROLL | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS) : field_style;
    const DWORD style = static_cast<DWORD>(WS_CHILD | WS_VISIBLE | WS_TABSTOP | kind_style);
    // A dropdown's height includes its open list.
    const int height = dropdown ? RectHeight(area) * 12 : RectHeight(area);
    const HWND control = CreateWindowExW(0, dropdown ? WC_COMBOBOXW : WC_EDITW, L"", style, area.left, area.top,
                                         RectWidth(area), height, level.dialog, ControlId(part.id), instance, nullptr);
    if (!c_assert(control != NULL)) return false;
    if (dropdown) return StyleDropdown(control, level.dropdown_look);
    level.field_look.placeholder = part.text;
    return StyleTextField(control, level.field_look);
}

bool ParchmentDialogs::CreateLinks(Level& level)
{
    if (!c_assert(level.layout != nullptr) || !c_assert(level.dialog != NULL)) return false;
    const DialogLayout& layout = *level.layout;
    for (int i = 0; i < layout.link_count && i < MAX_LINKS; i++)
    {
        const ControlPlacement placement = { level.dialog, layout.links[i].id, level.link_bounds[i],
                                             &level.shell.Backdrop(), layout.links[i].text };
        if (!level.links[i].Create(context, placement)) return false;
        level.link_count++;
    }
    return true;
}

// The dialog's windows are gone; its controls forget them so they can serve the next dialog.
void ParchmentDialogs::ReleaseLevel(Level& level)
{
    level.shell.Detach();
    level.close_button.Release();
    for (int i = 0; i < level.button_count && i < MAX_BUTTONS; i++) level.buttons[i].Release();
    for (int i = 0; i < level.checkbox_count && i < MAX_CHECKBOXES; i++) level.checkboxes[i].Release();
    for (int i = 0; i < level.label_count && i < MAX_LABELS; i++) level.labels[i].Release();
    for (int i = 0; i < level.link_count && i < MAX_LINKS; i++) level.links[i].Release();
    level.slider.Release();
    level.log.Release();
    level.picture.Release();
    level.dialog = NULL;
}

// Makes room in an open dialog (for an error line): moves the parts below a height down and reshapes the dialog.
bool ParchmentDialogs::Grow(HWND dialog, const Growth& growth)
{
    Level* level = LevelOf(dialog);
    if (!c_assert(level != nullptr) || !c_assert(growth.extra_height > 0)) return false;
    if (!c_assert(level->size.height + growth.extra_height <= MAX_DIALOG_HEIGHT)) return false;
    MovePartsDown(*level, growth);
    level->size.height += growth.extra_height;
    level->extra_height += growth.extra_height;
    return ReshapeLevel(*level);
}

void ParchmentDialogs::MovePartsDown(const Level& level, const Growth& growth)
{
    if (!c_assert(level.dialog != NULL) || !c_assert(growth.extra_height > 0)) return;
    constexpr int MAX_CHILDREN = 64;
    HWND child = GetWindow(level.dialog, GW_CHILD);
    for (int i = 0; i < MAX_CHILDREN && child != NULL; i++)
    {
        RECT area = {};
        const bool placed =
            GetWindowRect(child, &area) && MapWindowPoints(NULL, level.dialog, reinterpret_cast<POINT*>(&area), 2) != 0;
        if (placed && area.top >= growth.from_y && GetDlgCtrlID(child) != growth.staying_id)
        {
            // A part that couldn't move stays usable where it is.
            (void)SetWindowPos(child, NULL, area.left, area.top + growth.extra_height, 0, 0,
                               SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

// Draws the dialog again at its new size: panel, shape, backdrop and every part.
bool ParchmentDialogs::ReshapeLevel(Level& level)
{
    if (!c_assert(level.dialog != NULL)) return false;
    level.shell.Detach();
    if (!SetWindowPos(level.dialog, NULL, 0, 0, level.size.width, level.size.height,
                      SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE))
        return false;
    if (!RenderPanel(level) || !level.shell.Attach(level.dialog, level.size) || !Decorate(level)) return false;
    if (!c_assert(RedrawWindow(level.dialog, nullptr, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN) != FALSE))
        return false;
    level.shell.Reveal();
    DimLevelsBelow(depth);
    return true;
}

void ParchmentDialogs::ShowPicture(HWND dialog, const PictureButton::Picture& picture)
{
    Level* level = LevelOf(dialog);
    if (!c_assert(level != nullptr)) return;
    level->picture.ShowPicture(picture);
}

// Everything under the open dialogs is dimmed once per dialog above it.
void ParchmentDialogs::DimLevelsBelow(int open_depth)
{
    if (!c_assert(main_shell != nullptr) || !c_assert(open_depth >= 0 && open_depth <= MAX_LEVELS)) return;
    main_shell->Dim(open_depth);
    for (int i = 0; i < open_depth - 1 && i < MAX_LEVELS; i++) levels[i].shell.Dim(open_depth - 1 - i);
}

// The messages that draw and shape a dialog; the dialog procedure passes them here first.
ParchmentDialogs::Handled ParchmentDialogs::HandleMessage(const MSG& message)
{
    Level* level = LevelOf(message.hwnd);
    if (level == nullptr || level->dialog == NULL) return Unhandled();
    switch (message.message)
    {
        case WM_PAINT: return OnPaint(*level);
        case WM_ERASEBKGND: return HandledWith(message.hwnd, 1);
        case WM_NCHITTEST: return OnHitTest(message.hwnd, *level, message.lParam);
        case WM_SETCURSOR: return OnSetCursor(message.hwnd, message.wParam, message.lParam);
        case WM_CTLCOLOREDIT: return OnFieldColour(message.wParam);
        case WM_CTLCOLORSTATIC: return OnReadOnlyFieldColour(message.wParam, message.lParam);
        case WM_CTLCOLORLISTBOX: return { true, reinterpret_cast<INT_PTR>(list_brush) };
        case WM_MEASUREITEM: return OnMeasureItem(message.hwnd, message.lParam);
        case WM_DRAWITEM: return OnDrawItem(message.lParam);
        case WM_WINDOWPOSCHANGED: return OnWindowPosChanged(*level, message.lParam);
        case WM_DESTROY: return OnDestroy(*level);
        case WM_COMMAND: return OnCommand(*level, message.wParam, message.lParam);
        default: return Unhandled();
    }
}

// The shape follows the dialog; once the dialog has just shown, it and the dimming under it are revealed.
ParchmentDialogs::Handled ParchmentDialogs::OnWindowPosChanged(Level& level, LPARAM lParam)
{
    if (!c_assert(level.dialog != NULL)) return Unhandled();
    const WINDOWPOS* position = reinterpret_cast<const WINDOWPOS*>(lParam);
    level.shell.Follow();
    if (position != nullptr && (position->flags & SWP_SHOWWINDOW) != 0)
    {
        level.shell.Reveal();
        DimLevelsBelow(depth);
    }
    return Unhandled();
}

ParchmentDialogs::Handled ParchmentDialogs::OnDestroy(Level& level)
{
    ReleaseLevel(level);
    return Unhandled();
}

ParchmentDialogs::Handled ParchmentDialogs::OnPaint(Level& level)
{
    if (!c_assert(level.dialog != NULL)) return Unhandled();
    level.shell.PaintWaitingChildren();
    PAINTSTRUCT paint = {};
    const HDC target = BeginPaint(level.dialog, &paint);
    if (target != NULL) level.shell.ShowBackdrop(target);
    if (target != NULL) PaintFieldFocus(level, target);
    (void)EndPaint(level.dialog, &paint);
    return { true, TRUE };
}

// The focused text field shows a dotted outline around its box, as the web version's focused field does.
void ParchmentDialogs::PaintFieldFocus(const Level& level, HDC target) const
{
    if (!c_assert(target != NULL) || !c_assert(level.dialog != NULL)) return;
    const HWND focus = GetFocus();
    if (focus == NULL || GetParent(focus) != level.dialog || !IsTextField(focus)) return;
    const PixelRect frame = TextFieldFocusFrame(AreaInParent(focus, level.dialog));
    PixelSurface* buffer = context.back_buffer;
    ParchmentRenderer* renderer = context.renderer;
    if (!c_assert(buffer != nullptr && renderer != nullptr) || !c_assert(buffer->Contains(frame))) return;
    const PixelSize size = { RectWidth(frame), RectHeight(frame) };
    const HDC buffer_dc = buffer->Dc();
    if (!BitBlt(buffer_dc, frame.left, frame.top, size.width, size.height, level.shell.Backdrop().Dc(), frame.left,
                frame.top, SRCCOPY))
        return;
    if (!renderer->BeginFrame({ buffer_dc, size, ParchmentRenderer::SurfaceAlpha::opaque, { frame.left, frame.top } }))
        return;
    renderer->DrawDottedFrame(ToFloatRect({ 0, 0, size.width, size.height }), FOCUS_COLOUR);
    if (!renderer->EndFrame()) return;
    // A failed copy leaves the outline as it was until the next paint.
    (void)BitBlt(target, frame.left, frame.top, size.width, size.height, buffer_dc, frame.left, frame.top, SRCCOPY);
}

// A text field gaining or losing the focus redraws the outline around it; the dialog procedure still gets the
// notification.
ParchmentDialogs::Handled ParchmentDialogs::OnCommand(const Level& level, WPARAM wParam, LPARAM lParam) const
{
    if (!c_assert(level.dialog != NULL)) return Unhandled();
    const WORD notification = HIWORD(wParam);
    const HWND control = reinterpret_cast<HWND>(lParam);
    if ((notification != EN_SETFOCUS && notification != EN_KILLFOCUS) || control == NULL) return Unhandled();
    const PixelRect frame = TextFieldFocusFrame(AreaInParent(control, level.dialog));
    const RECT area = { frame.left, frame.top, frame.right, frame.bottom };
    // Invalidating a live window can't fail in a way that needs handling; at worst the next paint shows it.
    (void)InvalidateRect(level.dialog, &area, FALSE);
    return Unhandled();
}

// The title row moves the dialog, as its title bar did.
ParchmentDialogs::Handled ParchmentDialogs::OnHitTest(HWND dialog, const Level& level, LPARAM lParam)
{
    if (!c_assert(dialog != NULL && dialog == level.dialog)) return Unhandled();
    POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    if (!ScreenToClient(dialog, &point)) return Unhandled();
    const bool in_title = point.y >= PANEL_PADDING && point.y < PANEL_PADDING + TITLE_HEIGHT &&
                          point.x >= PANEL_PADDING && point.x < level.size.width - PANEL_PADDING - CLOSE_BUTTON_WIDTH;
    if (!in_title) return Unhandled();
    return HandledWith(dialog, HTCAPTION);
}

ParchmentDialogs::Handled ParchmentDialogs::OnSetCursor(HWND dialog, WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(dialog != NULL) || !c_assert(context.renderer != nullptr)) return Unhandled();
    const bool over_dialog = reinterpret_cast<HWND>(wParam) == dialog;
    const UINT hit = LOWORD(lParam);
    if (!over_dialog || (hit != HTCLIENT && hit != HTCAPTION)) return Unhandled();
    (void)SetCursor(context.renderer->Cursor(CursorId::arrow));
    return HandledWith(dialog, TRUE);
}

// Text fields: white text on the field's grey, which the frame around them matches.
ParchmentDialogs::Handled ParchmentDialogs::OnFieldColour(WPARAM wParam) const
{
    const HDC dc = reinterpret_cast<HDC>(wParam);
    if (!c_assert(dc != NULL) || !c_assert(field_brush != NULL)) return Unhandled();
    (void)SetTextColor(dc, RGB(FIELD_TEXT_COLOUR.red, FIELD_TEXT_COLOUR.green, FIELD_TEXT_COLOUR.blue));
    (void)SetBkColor(dc, RGB(FIELD_COLOUR.red, FIELD_COLOUR.green, FIELD_COLOUR.blue));
    return { true, reinterpret_cast<INT_PTR>(field_brush) };
}

// A read-only field asks for its colours as static text does; it looks like the other fields.
ParchmentDialogs::Handled ParchmentDialogs::OnReadOnlyFieldColour(WPARAM wParam, LPARAM lParam) const
{
    const HWND control = reinterpret_cast<HWND>(lParam);
    if (control == NULL || !IsTextField(control)) return Unhandled();
    return OnFieldColour(wParam);
}

ParchmentDialogs::Handled ParchmentDialogs::OnMeasureItem(HWND dialog, LPARAM lParam) const
{
    if (!c_assert(dialog != NULL)) return Unhandled();
    MEASUREITEMSTRUCT* measure = reinterpret_cast<MEASUREITEMSTRUCT*>(lParam);
    if (measure == nullptr || measure->CtlType != ODT_COMBOBOX) return Unhandled();
    const bool selection_field = measure->itemID == static_cast<UINT>(-1);
    measure->itemHeight = static_cast<UINT>(selection_field ? DROPDOWN_SELECTION_HEIGHT : DROPDOWN_ITEM_HEIGHT);
    return HandledWith(dialog, TRUE);
}

ParchmentDialogs::Handled ParchmentDialogs::OnDrawItem(LPARAM lParam) const
{
    const DRAWITEMSTRUCT* item = reinterpret_cast<const DRAWITEMSTRUCT*>(lParam);
    if (item == nullptr || item->CtlType != ODT_COMBOBOX) return Unhandled();
    if (!c_assert(context.renderer != nullptr && context.back_buffer != nullptr)) return Unhandled();
    DrawDropdownItem(*context.renderer, *context.back_buffer, *item);
    return { true, TRUE };
}
