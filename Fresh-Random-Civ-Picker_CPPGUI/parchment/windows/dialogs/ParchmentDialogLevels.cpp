#include "ParchmentDialog.h"

#include <commctrl.h>

#include "Assertions.h"

namespace
{
    struct EmptyDialogTemplate
    {
        DLGTEMPLATE header;
        WORD menu;
        WORD window_class;
        WORD title;
        WORD padding;
    };

    alignas(4) const EmptyDialogTemplate EMPTY_DIALOG = { { WS_POPUP | WS_CLIPCHILDREN, 0, 0, 0, 0, 100, 100 },
                                                          0, 0, 0, 0 };

    HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }
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
    if (field_font != NULL) (void)DeleteObject(field_font);
    if (field_brush != NULL) (void)DeleteObject(field_brush);
    if (list_brush != NULL) (void)DeleteObject(list_brush);
    field_font = NULL;
    field_brush = NULL;
    list_brush = NULL;
}

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
    level.key_button_count = 0;
    level.field_look = { context, field_font, nullptr };
    level.dropdown_look = { context, &level.shell.Backdrop() };
    if (!PlaceDialog(level) || !RenderPanel(level) || !level.shell.Attach(dialog, level.size)) return false;
    if (!Decorate(level) || !CreateParts(level) || !CreateLinks(level) || !CreateCloseButton(level)) return false;
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
        case PartKind::key_button: return CreateKeyButtonPart(level, part);
        case PartKind::slider:
        case PartKind::log:
        case PartKind::picture:
        case PartKind::dropdown:
        case PartKind::text_field:
        case PartKind::readonly_field: return CreateWindowPart(level, part);
        default: (void)c_assert(false); break;
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
    if (error) (void)ShowWindow(label.Window(), SW_HIDE);
    return true;
}

bool ParchmentDialogs::CreateKeyButtonPart(Level& level, const DialogPart& part)
{
    if (!c_assert(level.key_button_count < MAX_KEY_BUTTONS) || !c_assert(part.text != nullptr)) return false;
    KeyButton& key_button = level.key_buttons[level.key_button_count];
    level.key_button_count++;
    return key_button.Create(context, { level.dialog, part.id, part.bounds, &level.shell.Backdrop(), part.text });
}

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

void ParchmentDialogs::ReleaseLevel(Level& level)
{
    level.shell.Detach();
    level.close_button.Release();
    for (int i = 0; i < level.button_count && i < MAX_BUTTONS; i++) level.buttons[i].Release();
    for (int i = 0; i < level.checkbox_count && i < MAX_CHECKBOXES; i++) level.checkboxes[i].Release();
    for (int i = 0; i < level.label_count && i < MAX_LABELS; i++) level.labels[i].Release();
    for (int i = 0; i < level.link_count && i < MAX_LINKS; i++) level.links[i].Release();
    for (int i = 0; i < level.key_button_count && i < MAX_KEY_BUTTONS; i++) level.key_buttons[i].Release();
    level.slider.Release();
    level.log.Release();
    level.picture.Release();
    level.dialog = NULL;
}

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
            (void)SetWindowPos(child, NULL, area.left, area.top + growth.extra_height, 0, 0,
                               SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

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

void ParchmentDialogs::DimLevelsBelow(int open_depth)
{
    if (!c_assert(main_shell != nullptr) || !c_assert(open_depth >= 0 && open_depth <= MAX_LEVELS)) return;
    main_shell->Dim(open_depth);
    for (int i = 0; i < open_depth - 1 && i < MAX_LEVELS; i++) levels[i].shell.Dim(open_depth - 1 - i);
}
