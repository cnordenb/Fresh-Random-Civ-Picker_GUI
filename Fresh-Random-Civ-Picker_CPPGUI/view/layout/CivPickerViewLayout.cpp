#include "CivPickerView.h"

#include <cwchar>

#include "Assertions.h"
#include "CivPickerLayout.h"
#include "Resource.h"

namespace
{
    constexpr MenuItemSpec SEPARATOR = { nullptr, nullptr, 0, false };

    const MenuItemSpec FILE_ITEMS[] = {
        { L"Copy Share Link", nullptr, IDM_COPYLINK, false },    { L"Share ...", nullptr, IDM_SHARE, false },
        { L"Open Link ...", nullptr, IDM_OPENLINK, false },
        SEPARATOR,
        { L"Save Preset ...", nullptr, IDM_SAVELOG, false },     { L"Load Preset ...", nullptr, IDM_LOADLOG, false },
        SEPARATOR,
        { L"Quicksave", nullptr, IDM_QUICKSAVE, false },         { L"Quickload", nullptr, IDM_QUICKLOAD, false },
        SEPARATOR,
        { L"Options ...", nullptr, IDM_OPTIONS, false },
        SEPARATOR,
        { L"Exit", nullptr, IDM_EXIT, false },
    };
    const MenuItemSpec LOBBY_ITEMS[] = {
        { L"Join Lobby ...", nullptr, IDM_JOINLOBBY, false },
        { L"AoE2 Insights ...", nullptr, IDM_AOE2INSIGHTS, true },
        { L"Spectator Dashboard ...", nullptr, IDM_SPECTATORDASHBOARD, true },
    };
    const MenuItemSpec CIV_ITEMS[] = {
        { L"Undraw", nullptr, IDM_UNDRAW, false },
        { L"Redraw", nullptr, IDM_REDRAW, false },
        { L"Tech Tree ...", nullptr, IDM_TECHTREE, true },
        { L"Stats ...", nullptr, IDM_STATS, true },
    };
    constexpr MenuItemSpec SURVAPP_ITEM = { L"Sustainability App ...", nullptr, IDM_SURVAPP, true };
    const MenuItemSpec FRCP_ITEMS[] = {
        { L"GitHub Repository ...", nullptr, IDM_GITHUB, true },
        { L"Web Version ...", nullptr, IDM_WEBVERSION, true },
        { L"Discord Server ...", nullptr, IDM_DISCORD, true },
    };
    const MenuItemSpec HELP_ITEMS[] = {
        { L"Hotkeys ...", nullptr, IDM_HOTKEYS, false },
        { L"Donate ...", nullptr, IDM_DONATE, true },
        { L"Check for Updates ...", nullptr, IDM_UPDATE, true },
        { L"About ...", nullptr, IDM_ABOUT, false },
    };

    struct MenuShortcut
    {
        int command;
        HotkeyAction action;
    };

    const MenuShortcut MENU_SHORTCUTS[] = {
        { IDM_COPYLINK, HotkeyAction::copy_share_link }, { IDM_OPENLINK, HotkeyAction::open_share_link },
        { IDM_SAVELOG, HotkeyAction::save_preset },      { IDM_LOADLOG, HotkeyAction::load_preset },
        { IDM_QUICKSAVE, HotkeyAction::quicksave },      { IDM_QUICKLOAD, HotkeyAction::quickload },
        { IDM_OPTIONS, HotkeyAction::open_options },     { IDM_EXIT, HotkeyAction::quit },
        { IDM_JOINLOBBY, HotkeyAction::join_lobby },     { IDM_UNDRAW, HotkeyAction::undo },
        { IDM_REDRAW, HotkeyAction::redraw },            { IDM_TECHTREE, HotkeyAction::open_tech_tree },
        { IDM_SURVAPP, HotkeyAction::open_sustainability_app },
        { IDM_HOTKEYS, HotkeyAction::open_hotkeys },     { IDM_ABOUT, HotkeyAction::open_about },
    };

    template <int Count> constexpr int CountOf(const MenuItemSpec (&)[Count]) { return Count; }

    HotkeyAction ShortcutOf(int command)
    {
        for (const MenuShortcut& shortcut : MENU_SHORTCUTS)
        {
            if (shortcut.command == command) return shortcut.action;
        }
        return HotkeyAction::none;
    }

    constexpr PixelRect TAB_STRIP_AREA = { 2, 0, 208, 25 };
    constexpr PixelRect OPTIONS_AREA = { 66, 54, 138, 126 };
    constexpr PixelRect HISTORY_AREA = { 385, 60, 468, 120 };
    constexpr PixelRect SURVAPP_AREA = { 90, 391, 150, 451 };
    constexpr PixelRect CIV_ICON_AREA = { 217, 165, 321, 269 };
    constexpr PixelRect CIV_NAME_AREA = { 169, 273, 369, 292 };
    constexpr PixelRect DRAW_AREA = { 217, 305, 317, 335 };
    constexpr PixelRect TECHTREE_AREA = { 384, 391, 444, 451 };
    constexpr PixelRect ROUND_COUNTER_AREA = { 456, 484, 516, 501 };
    constexpr PixelRect DRAWN_COUNT_AREA = { 10, 32, 105, 49 };
    constexpr PixelRect CLEAR_LOG_AREA = { 110, 25, 210, 55 };
    constexpr PixelRect DRAWN_LOG_AREA = { 10, 60, 208, 501 };
    constexpr PixelRect REMAINING_COUNT_AREA = { 327, 32, 452, 49 };
    constexpr PixelRect SHOW_REMAINING_AREA = { 457, 30, 517, 50 };
    constexpr PixelRect REMAINING_LOG_AREA = { 327, 60, 529, 501 };
    constexpr PixelRect SHARE_AREA = { 249, 385, 286, 422 };
    constexpr PixelRect POOL_SHARE_AREA = { 487, 466, 524, 503 };
    constexpr int SHARE_ICON_SIZE = 28;
    constexpr PixelRect EDITION_BANNER_AREA = { 200, 30, 328, 123 };
    constexpr PixelRect ENABLED_COUNT_AREA = { 210, 140, 330, 157 };
    constexpr PixelRect AUTO_TOGGLE_AREA = { 10, 180, 160, 200 };
    constexpr PixelRect ENABLE_ALL_AREA = { 10, 470, 110, 500 };
    constexpr PixelRect DISABLE_ALL_AREA = { 125, 470, 225, 500 };
    constexpr PixelRect AUTO_RESET_AREA = { 240, 480, 400, 500 };
    constexpr int RESET_LEFT_ON_DRAW_TAB = 10;
    constexpr int RESET_LEFT_ON_LOG_TAB = 217;
    constexpr int RESET_TOP = 471;
    constexpr int BUTTON_SIZE_X = 100;
    constexpr int BUTTON_SIZE_Y = 30;
    constexpr int SURVAPP_ICON_SIZE = 48;

    constexpr int EDITION_LEFT = 10;
    constexpr int EDITION_TOP = 36;
    constexpr int EDITION_STEP = 30;
    constexpr int EDITION_WIDTH = 188;
    constexpr int EDITION_HEIGHT = 28;
    static_assert(EDITION_TOP + EDITION_STEP * (MAX_EDITIONS - 1) + EDITION_HEIGHT <= ENABLED_COUNT_AREA.top,
                  "the edition rows fit above the count of enabled civs");

    constexpr int GRID_COLUMNS[] = { 10, 112, 214, 316, 418 };
    constexpr int GRID_COLUMN_COUNT = static_cast<int>(sizeof(GRID_COLUMNS) / sizeof(GRID_COLUMNS[0]));
    constexpr int GRID_ROW = 20;
    constexpr int GRID_ROWS = (MAX_CIVS + GRID_COLUMN_COUNT - 1) / GRID_COLUMN_COUNT;
    static_assert(CIV_GRID_TOP + GRID_ROWS * GRID_ROW <= ENABLE_ALL_AREA.top, "the civ grid fits above its buttons");
    constexpr int CIV_TOGGLE_WIDTH = 102;
    constexpr int CIV_ICON_SIZE = 18;
    constexpr int CIV_ICON_GAP = 2;

    constexpr float CLIENT_FRAME_BOTTOM = static_cast<float>(CLIENT_HEIGHT);
}

namespace
{
    PixelRect ResetArea(int left)
    {
        if (!c_assert(left >= 0 && left + BUTTON_SIZE_X <= CLIENT_WIDTH)) return { 0, 0, 0, 0 };
        return { left, RESET_TOP, left + BUTTON_SIZE_X, RESET_TOP + BUTTON_SIZE_Y };
    }
}

bool CivPickerView::BuildMainShell()
{
    if (!c_assert(main_window != NULL)) return false;
    PixelSurface& canvas = main_shell.Canvas();
    const PixelSize size = { WINDOW_WIDTH, WINDOW_HEIGHT };
    if (!renderer.BeginFrame({ canvas.Dc(), size, ParchmentRenderer::SurfaceAlpha::premultiplied })) return false;
    renderer.Clear();
    const PixelRect content = { WINDOW_PADDING, WINDOW_PADDING, WINDOW_WIDTH - WINDOW_PADDING,
                                CLIENT_TOP + CLIENT_HEIGHT };
    renderer.FillRect(ToFloatRect(content), CONTENT_UNDERLAY_COLOUR);
    const PixelRect visible_part = { BACKGROUND_VISIBLE_LEFT, BACKGROUND_VISIBLE_TOP, BACKGROUND_VISIBLE_RIGHT,
                                     BACKGROUND_VISIBLE_BOTTOM };
    renderer.DrawImage({ ImageId::background, visible_part, ToFloatRect({ 0, 0, size.width, size.height }), 1.0f,
                         ParchmentRenderer::Smoothing::smooth });
    return renderer.EndFrame() && main_shell.Attach(main_window, size);
}

bool CivPickerView::DecorateMainBackdrop()
{
    if (!c_assert(main_window != NULL) || !c_assert(title[0] != L'\0')) return false;
    PixelSurface& backdrop = main_shell.Backdrop();
    const PixelSize size = { WINDOW_WIDTH, WINDOW_HEIGHT };
    if (!renderer.BeginFrame({ backdrop.Dc(), size, ParchmentRenderer::SurfaceAlpha::opaque }))
        return false;
    const TextSpan title_text = { title, static_cast<int>(wcslen(title)), FontId::ui };
    const float baseline = renderer.BaselineCentred(FontId::ui, static_cast<float>(WINDOW_PADDING),
                                                    static_cast<float>(TITLE_BAR_HEIGHT));
    const PixelSize icon = renderer.ImageSize(ImageId::program_icon);
    const int icon_left = WINDOW_PADDING + TITLE_TEXT_LEFT;
    const int icon_top = WINDOW_PADDING + (TITLE_BAR_HEIGHT - TITLE_ICON_SIZE) / 2;
    renderer.DrawImage({ ImageId::program_icon, { 0, 0, icon.width, icon.height },
                         ToFloatRect({ icon_left, icon_top, icon_left + TITLE_ICON_SIZE, icon_top + TITLE_ICON_SIZE }),
                         1.0f, ParchmentRenderer::Smoothing::smooth });
    const int title_left = icon_left + TITLE_ICON_SIZE + TITLE_ICON_GAP;
    renderer.DrawText({ title_text, TEXT_COLOUR, static_cast<float>(title_left),
                        static_cast<float>(WINDOW_WIDTH - WINDOW_PADDING), baseline, TextAlign::left });
    const float left = static_cast<float>(CLIENT_LEFT);
    const float right = static_cast<float>(CLIENT_LEFT + CLIENT_WIDTH);
    const float top = static_cast<float>(CLIENT_TOP + CLIENT_FRAME_TOP);
    const float bottom = static_cast<float>(CLIENT_TOP) + CLIENT_FRAME_BOTTOM;
    renderer.FillRect({ left, top, right, top + 1.0f }, CLIENT_FRAME_COLOUR);
    renderer.FillRect({ left, bottom - 1.0f, right, bottom }, CLIENT_FRAME_COLOUR);
    renderer.FillRect({ left, top + 1.0f, left + 1.0f, bottom - 1.0f }, CLIENT_FRAME_COLOUR);
    renderer.FillRect({ right - 1.0f, top + 1.0f, right, bottom - 1.0f }, CLIENT_FRAME_COLOUR);
    return renderer.EndFrame();
}

bool CivPickerView::CreateMainControls()
{
    if (!c_assert(main_window != NULL)) return false;
    return CreateFrameControls() && CreateDrawTabControls() && CreateLogTabControls() && CreatePoolTabControls();
}

ControlPlacement CivPickerView::PlaceInClient(int id, const PixelRect& client_bounds, const wchar_t* text) const
{
    const ControlPlacement refused = { NULL, id, client_bounds, nullptr, L"" };
    if (!c_assert(HasArea(client_bounds)) || !c_assert(text != nullptr)) return refused;
    return { main_window, id, OffsetRect(client_bounds, { CLIENT_LEFT, CLIENT_TOP }), &main_shell.Backdrop(), text };
}

bool CivPickerView::CreateFrameControls()
{
    if (!c_assert(main_window != NULL)) return false;
    const ControlContext context = { &renderer, &back_buffer, text_menu };
    const PixelRect close_area = { WINDOW_WIDTH - WINDOW_PADDING - CLOSE_BUTTON_WIDTH, WINDOW_PADDING,
                                   WINDOW_WIDTH - WINDOW_PADDING, WINDOW_PADDING + TITLE_BAR_HEIGHT };
    if (!close_button.Create(context, { main_window, IDC_BUTTON_CLOSE, close_area, &main_shell.Backdrop(), L"" }))
        return false;
    const PixelRect menu_area = { WINDOW_PADDING, WINDOW_PADDING + TITLE_BAR_HEIGHT, WINDOW_WIDTH - WINDOW_PADDING,
                                  CLIENT_TOP };
    if (!BuildMenus()) return false;
    if (!menu_bar.Create(context, { main_window, IDC_MENU_BAR, menu_area, &main_shell.Backdrop(), L"" },
                         { menus, MENU_COUNT }))
        return false;
    const wchar_t* const tab_names[TabStrip::TAB_COUNT] = { L"Draw Civ", L"Log", L"Civ Pool" };
    tab_strip.SetNames(tab_names);
    return tab_strip.Create(context, PlaceInClient(IDC_TAB_STRIP, TAB_STRIP_AREA, L""));
}

bool CivPickerView::BuildMenus()
{
    static_assert(MAX_GAME_ITEMS <= ParchmentMenuBar::MAX_ITEMS, "the Game menu fits a menu");
    const CivCatalog& catalog = model.Catalog();
    const int link_count = catalog.LinkCount();
    if (!c_assert(link_count >= 0 && link_count < MAX_GAME_ITEMS)) return false;
    MenuItemSpec game_items[MAX_GAME_ITEMS] = {};
    for (int i = 0; i < link_count; i++)
    {
        (void)_snwprintf_s(game_item_labels[i], MAX_MENU_LABEL, _TRUNCATE, L"%ls ...", catalog.LinkAt(i).text.c_str());
        game_items[i] = { game_item_labels[i], nullptr, EDITION_LINK_BASE_ID + i, true };
    }
    game_items[link_count] = SURVAPP_ITEM;
    const MenuSpec all_menus[MENU_COUNT] = {
        { L"File", L'F', FILE_ITEMS, CountOf(FILE_ITEMS) },  { L"Lobbies", L'L', LOBBY_ITEMS, CountOf(LOBBY_ITEMS) },
        { L"Civs", L'C', CIV_ITEMS, CountOf(CIV_ITEMS) },     { L"Game", L'G', game_items, link_count + 1 },
        { L"FRCP", L'F', FRCP_ITEMS, CountOf(FRCP_ITEMS) },  { L"Help", L'H', HELP_ITEMS, CountOf(HELP_ITEMS) },
    };
    for (int i = 0; i < MENU_COUNT; i++)
    {
        if (!CopyMenu(i, all_menus[i])) return false;
    }
    return true;
}

bool CivPickerView::CopyMenu(int menu_index, const MenuSpec& menu)
{
    if (!c_assert(menu_index >= 0 && menu_index < MENU_COUNT) || !c_assert(menu.items != nullptr)) return false;
    if (!c_assert(menu.item_count > 0 && menu.item_count <= ParchmentMenuBar::MAX_ITEMS)) return false;
    for (int i = 0; i < menu.item_count; i++)
    {
        MenuItemSpec item = menu.items[i];
        const HotkeyAction shortcut = ShortcutOf(item.command);
        if (shortcut != HotkeyAction::none) item.shortcut = hotkey_keys[static_cast<int>(shortcut)].characters;
        menu_items[menu_index][i] = item;
    }
    menus[menu_index] = { menu.title, menu.mnemonic, menu_items[menu_index], menu.item_count };
    return true;
}

bool CivPickerView::CreateDrawTabControls()
{
    if (!c_assert(main_window != NULL)) return false;
    const ControlContext context = { &renderer, &back_buffer, text_menu };
    button_options.SetLooks({ ImageId::options_button, OPTIONS_BUTTON_CELL, true });
    button_techtree.SetLooks({ ImageId::techtree_button, TECHTREE_BUTTON_CELL, false });
    button_survapp.SetIcon({ ImageId::aoe2vils_icon, SURVAPP_ICON_SIZE, ParchmentRenderer::Smoothing::pixelated });
    const PixelSize book = renderer.ImageSize(ImageId::history_book);
    button_history.ShowPicture({ ImageId::history_book, { 0, 0, book.width, book.height } });
    const bool created = button_options.Create(context, PlaceInClient(IDC_BUTTON_OPTIONS, OPTIONS_AREA, L"")) &&
                         button_history.Create(context, PlaceInClient(IDC_BUTTON_HISTORY, HISTORY_AREA, L"History")) &&
                         button_survapp.Create(context, PlaceInClient(IDC_BUTTON_SURVAPP, SURVAPP_AREA, L"")) &&
                         civ_icon.Create(context, PlaceInClient(IDC_ICON_CIV, CIV_ICON_AREA, L"")) &&
                         button_draw.Create(context, PlaceInClient(IDC_BUTTON_DRAW, DRAW_AREA, L"Draw")) &&
                         button_reset.Create(context, PlaceInClient(IDC_BUTTON_RESET, ResetArea(RESET_LEFT_ON_DRAW_TAB),
                                                                    L"Reset")) &&
                         button_techtree.Create(context, PlaceInClient(IDC_BUTTON_TECHTREE, TECHTREE_AREA, L""));
    if (!created) return false;
    ShowCivIconFor(L"Random");
    return CreateLabel(label_centre, PlaceInClient(IDC_LABEL_CENTRE, CIV_NAME_AREA, L"?"),
                       { FontId::civ_name, TEXT_COLOUR, TextAlign::centre, CIV_NAME_LINE_HEIGHT }) &&
           CreateLabel(label_corner, PlaceInClient(IDC_LABEL_CORNER, ROUND_COUNTER_AREA, L""),
                       { FontId::ui_bold, TEXT_COLOUR, TextAlign::right, UI_LINE_HEIGHT });
}

bool CivPickerView::CreateLogTabControls()
{
    if (!c_assert(main_window != NULL)) return false;
    const ControlContext context = { &renderer, &back_buffer, text_menu };
    const Label::Style count_style = { FontId::ui_bold, TEXT_COLOUR, TextAlign::left, UI_LINE_HEIGHT };
    checkbox_showremainlog.SetChecked(model.GetSettings().remainlog_enabled);
    button_share.SetIcon({ ImageId::share_icon, SHARE_ICON_SIZE, ParchmentRenderer::Smoothing::smooth });
    return CreateLabel(label_drawncount, PlaceInClient(IDC_LABEL_DRAWNCOUNT, DRAWN_COUNT_AREA, L""), count_style) &&
           button_clearlog.Create(context, PlaceInClient(IDC_BUTTON_CLEARLOG, CLEAR_LOG_AREA, L"Clear Log")) &&
           drawn_log.Create(context, PlaceInClient(IDC_LOG_DRAWN, DRAWN_LOG_AREA, L"")) &&
           CreateLabel(label_remainingcount, PlaceInClient(IDC_LABEL_REMAININGCOUNT, REMAINING_COUNT_AREA, L""),
                       count_style) &&
           checkbox_showremainlog.Create(context,
                                         PlaceInClient(IDC_CHECKBOX_REMAINLOG, SHOW_REMAINING_AREA, L"Show")) &&
           remaining_log.Create(context, PlaceInClient(IDC_LOG_REMAINING, REMAINING_LOG_AREA, L"")) &&
           ShowCivIconsInLogs() && button_share.Create(context, PlaceInClient(IDC_BUTTON_SHARE, SHARE_AREA, L"")) &&
           RaiseLogsOverCivName();
}

bool CivPickerView::ShowCivIconsInLogs()
{
    static_assert(MAX_CIVS <= MAX_ICON_LABELS, "every civ's name can lead a log line");
    if (!c_assert(drawn_log.Window() != NULL && remaining_log.Window() != NULL)) return false;
    const CivCatalog& catalog = model.Catalog();
    for (int civ = 0; civ < catalog.CivCount() && civ < MAX_CIVS; civ++)
    {
        if (!civ_labels.Add({ catalog.CivAt(civ).name.c_str(), civ })) return false;
    }
    const ParchmentLog::Icons icons = { &civ_labels, ImageId::civ_atlas, CIV_ATLAS_COLUMNS, CIV_ATLAS_CELL };
    return drawn_log.ShowIcons(icons) && remaining_log.ShowIcons(icons);
}

bool CivPickerView::RaiseLogsOverCivName()
{
    if (!c_assert(drawn_log.Window() != NULL && remaining_log.Window() != NULL)) return false;
    const UINT flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;
    return c_assert(SetWindowPos(drawn_log.Window(), HWND_TOP, 0, 0, 0, 0, flags) != FALSE) &&
           c_assert(SetWindowPos(remaining_log.Window(), HWND_TOP, 0, 0, 0, 0, flags) != FALSE);
}

bool CivPickerView::CreatePoolTabControls()
{
    if (!c_assert(main_window != NULL)) return false;
    const ControlContext context = { &renderer, &back_buffer, text_menu };
    const Settings& settings = model.GetSettings();
    checkbox_autotoggle.SetChecked(settings.autotoggle_enabled);
    checkbox_autoreset.SetChecked(settings.autoreset_enabled);
    button_poolshare.SetIcon({ ImageId::share_icon, SHARE_ICON_SIZE, ParchmentRenderer::Smoothing::smooth });
    const std::wstring enabled_text = L"Enabled: " + std::to_wstring(model.GetCustomMaxCivs());
    return CreateEditionControls() &&
           CreateLabel(label_enabledcount,
                       PlaceInClient(IDC_LABEL_ENABLEDCOUNT, ENABLED_COUNT_AREA, enabled_text.c_str()),
                       { FontId::ui_bold, TEXT_COLOUR, TextAlign::left, UI_LINE_HEIGHT }) &&
           checkbox_autotoggle.Create(context, PlaceInClient(IDC_CHECKBOX_AUTOTOGGLE, AUTO_TOGGLE_AREA,
                                                             L"Auto-toggle older civs")) &&
           CreateCivToggles() && CreateDlcToggles() &&
           button_enableall.Create(context, PlaceInClient(IDC_BUTTON_ENABLEALL, ENABLE_ALL_AREA, L"Enable All")) &&
           button_disableall.Create(context, PlaceInClient(IDC_BUTTON_DISABLEALL, DISABLE_ALL_AREA, L"Disable All")) &&
           checkbox_autoreset.Create(context, PlaceInClient(IDC_CHECKBOX_AUTORESET, AUTO_RESET_AREA,
                                                            L"Auto-reset upon change")) &&
           button_poolshare.Create(context, PlaceInClient(IDC_BUTTON_POOLSHARE, POOL_SHARE_AREA, L""));
}

bool CivPickerView::CreateEditionControls()
{
    if (!c_assert(main_window != NULL)) return false;
    const ControlContext context = { &renderer, &back_buffer, text_menu };
    const CivCatalog& catalog = model.Catalog();
    for (int i = 0; i < catalog.EditionCount() && i < MAX_EDITIONS; i++)
    {
        const EditionInfo& edition = catalog.EditionAt(i);
        const int top = EDITION_TOP + EDITION_STEP * i;
        const PixelRect area = { EDITION_LEFT, top, EDITION_LEFT + EDITION_WIDTH, top + EDITION_HEIGHT };
        const std::wstring text = edition.short_name + L" (" + std::to_wstring(edition.year) + L")";
        edition_rows[i].SetEditionIndex(i);
        if (!edition_rows[i].Create(context, PlaceInClient(EDITION_ROW_BASE_ID + i, area, text.c_str()))) return false;
    }
    edition_rows[0].SetChecked(true);
    return edition_icon.Create(context, PlaceInClient(IDC_ICON_EDITION, EDITION_BANNER_AREA, L""));
}

bool CivPickerView::CreateCivToggles()
{
    if (!c_assert(main_window != NULL)) return false;
    const ControlContext context = { &renderer, &back_buffer, text_menu };
    const CivCatalog& catalog = model.Catalog();
    for (int position = 0; position < catalog.CivCount() && position < MAX_CIVS; position++)
    {
        const int civ = catalog.DisplayCiv(position);
        const int left = GRID_COLUMNS[position % GRID_COLUMN_COUNT];
        const int top = CIV_GRID_TOP + GRID_ROW * (position / GRID_COLUMN_COUNT);
        civ_toggles[civ].SetIcon({ ImageId::civ_atlas, ImageId::civ_atlas_grey, CIV_ATLAS_COLUMNS, CIV_ATLAS_CELL, civ,
                                   CIV_ICON_SIZE, CIV_ICON_GAP, true });
        civ_toggles[civ].SetChecked(true);
        const PixelRect area = { left, top, left + CIV_TOGGLE_WIDTH, top + GRID_ROW };
        const ControlPlacement placement =
            PlaceInClient(civ + CIV_CHECKBOX_BASE_ID, area, catalog.CivAt(civ).name.c_str());
        if (!civ_toggles[civ].Create(context, placement)) return false;
    }
    return true;
}

bool CivPickerView::CreateLabel(Label& label, const ControlPlacement& placement, const Label::Style& style)
{
    if (!c_assert(placement.parent != NULL) || !c_assert(style.line_height > 0.0f)) return false;
    label.SetStyle(style);
    return label.Create({ &renderer, &back_buffer, text_menu }, placement);
}

void CivPickerView::MoveResetButton(int tab_index)
{
    if (!c_assert(tab_index == DRAW_TAB || tab_index == LOG_TAB)) return;
    if (!c_assert(button_reset.Window() != NULL)) return;
    const int left = tab_index == LOG_TAB ? RESET_LEFT_ON_LOG_TAB : RESET_LEFT_ON_DRAW_TAB;
    const PixelRect area = OffsetRect(ResetArea(left), { CLIENT_LEFT, CLIENT_TOP });
    const UINT flags = SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE;
    (void)SetWindowPos(button_reset.Window(), NULL, area.left, area.top, 0, 0, flags);
}
