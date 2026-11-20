#include "CivPickerController.h"

#include <shellapi.h>
#include <iomanip>
#include <sstream>

#include "Resource.h"

namespace
{
    void OpenUrl(const wchar_t* url) { ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL); }
}

void CivPickerController::LoadCachedLog()
{
    model.SetCacheLoading(true);
    LoadLog(SaveType::automatic);
    model.SetCacheAvailable(false);
    model.SetCacheLoading(false);
}

void CivPickerController::SaveLog(SaveType type)
{
    model.DiscardRedoHistory();
    std::wstring save_file_path = log_file_path;
    if (type == SaveType::quick) save_file_path = QuicksavePath();
    else if (type == SaveType::manual)
    {
        const CivPickerView::FileChoice choice = ChooseManualSaveFile();
        if (!choice.chosen) return;
        save_file_path = choice.path;
    }
    (void)model.WriteLogFile(save_file_path);
}

CivPickerView::FileChoice CivPickerController::ChooseManualSaveFile()
{
    view.PlayAudio(SoundType::button);
    SYSTEMTIME now = {};
    GetLocalTime(&now);

    std::wstringstream file_name;
    file_name << L"FRCP Preset v" << VERSION << L" @"
              << std::setw(4) << std::setfill(L'0') << now.wYear << L"."
              << std::setw(2) << std::setfill(L'0') << now.wMonth << L"."
              << std::setw(2) << std::setfill(L'0') << now.wDay << L" "
              << std::setw(2) << std::setfill(L'0') << now.wHour
              << std::setw(2) << std::setfill(L'0') << now.wMinute
              << std::setw(2) << std::setfill(L'0') << now.wSecond << L".txt";

    CivPickerView::FileChoice choice = view.AskSaveFilePath(file_name.str(), SavesFolder());
    view.PlayAudio(SoundType::button);
    if (choice.chosen && choice.path.find(L".txt") == std::wstring::npos) choice.path.append(L".txt");
    return choice;
}

void CivPickerController::LoadLog(SaveType type)
{
    std::wstring load_file_path = log_file_path;
    if (type != SaveType::automatic)
    {
        const CivPickerView::FileChoice choice = ChooseLogToLoad(type);
        if (!choice.chosen) return;
        load_file_path = choice.path;
        PrepareLogLoad();
    }

    if (!model.LoadLogFile(load_file_path, type)) return;
    if (type != SaveType::automatic || model.IsCacheLoading())
    {
        ShowTab(view.GetCurrentTab());
        view.ValidateAllDlcToggles();
        model.SetStartingUp(false);
    }
}

CivPickerView::FileChoice CivPickerController::ChooseLogToLoad(SaveType type)
{
    view.PlayAudio(SoundType::button);
    if (type == SaveType::quick)
    {
        const std::wstring quicksave_path = QuicksavePath();
        return { FileExists(quicksave_path), quicksave_path };
    }

    CivPickerView::FileChoice choice = view.AskOpenFilePath(SavesFolder());
    if (!choice.chosen)
    {
        view.PlayAudio(SoundType::button);
        model.SetStartingUp(false);
    }
    return choice;
}

void CivPickerController::CopyRoundLink()
{
    view.PlayAudio(SoundType::button);
    CopyLink(model.GetSharedPool());
}

void CivPickerController::CopyPoolLink()
{
    view.PlayAudio(SoundType::button);
    SharedPool pool = model.GetSharedPool();
    pool.draw_count = 0;
    CopyLink(pool);
}

void CivPickerController::CopyLink(const SharedPool& pool)
{
    const ShareLink::Text link = ShareLink::Write(model.Catalog(), pool);
    if (view.CopyText(link.Characters())) view.ShowLinkCopied(pool.draw_count);
    else ShowShareLinks();
}

void CivPickerController::OpenShare()
{
    view.PlayAudio(SoundType::button);
    ShowShareLinks();
}

void CivPickerController::ShowShareLinks()
{
    SharedPool pool = model.GetSharedPool();
    const ShareLink::Text round_link = ShareLink::Write(model.Catalog(), pool);
    const int draw_count = pool.draw_count;
    pool.draw_count = 0;
    const ShareLink::Text pool_link = ShareLink::Write(model.Catalog(), pool);
    view.ShowShareDialog(main_window, { pool_link.Characters(), round_link.Characters(), draw_count });
}

void CivPickerController::OpenShareLink()
{
    view.PlayAudio(SoundType::button);
    const ShareLink::Opened opened = view.AskShareLink(main_window);
    if (!opened.ok) return;
    const bool replaces = model.HasCustomState() || view.HasDrawnLog();
    if (replaces && !view.ConfirmSharedPool(main_window, opened.pool)) return;
    model.SetStartingUp(true);
    model.LoadSharedPool(opened.pool);
    model.SetStartingUp(false);
    ShowTab(view.GetCurrentTab());
    view.ValidateAllDlcToggles();
    if (replaces) view.PlayAudio(SoundType::button);
    view.ShowLinkLoaded();
}

bool CivPickerController::OpenEditionLink(int command_id)
{
    const int link = command_id - CivPickerView::EDITION_LINK_BASE_ID;
    if (link < 0 || link >= model.Catalog().LinkCount()) return false;
    view.PlayAudio(SoundType::button);
    OpenUrl(model.Catalog().LinkAt(link).url.c_str());
    return true;
}

void CivPickerController::OpenEditionPage()
{
    const EditionInfo& edition = model.Catalog().EditionAt(model.GetEdition());
    if (!edition.url.empty()) OpenUrl(edition.url.c_str());
    else view.ShowInformation(edition.store_note.c_str(), edition.short_name.c_str());
}

void CivPickerController::OpenTechTree()
{
    view.PlayAudio(SoundType::button);
    const int civ = model.FindCivIndex(model.GetCurrentCiv());
    std::wstring techtree_url = L"https://aoe2techtree.net/#";
    if (civ != CivCatalog::NONE) techtree_url += model.Catalog().CivAt(civ).tech_tree;
    OpenUrl(techtree_url.c_str());
}

void CivPickerController::OpenSurvapp()
{
    view.PlayAudio(SoundType::button);
    OpenUrl(L"https://aoe2vils.net/");
}

void CivPickerController::OpenAuthorPage() { OpenUrl(L"https://linktr.ee/hjoerleif"); }

void CivPickerController::OpenOptions(HWND owner)
{
    view.PlayAudio(SoundType::button);
    view.ShowOptionsDialog(owner);
}

void CivPickerController::OpenHotkeys(HWND owner)
{
    view.PlayAudio(SoundType::button);
    view.ShowHotkeysDialog(owner);
}

void CivPickerController::OpenAbout(HWND owner)
{
    view.PlayAudio(SoundType::button);
    view.ShowAboutDialog(owner);
}

void CivPickerController::OpenHistory(HWND owner)
{
    view.PlayAudio(SoundType::button);
    view.ShowHistoryDialog(owner);
}

void CivPickerController::JoinLobby()
{
    view.PlayAudio(SoundType::button);
    const CivPickerView::LobbyInput input = view.AskLobbyCode(main_window);
    if (input.result == IDC_BUTTON_OK) LaunchLobby(input.code, L"aoe2de://0/", L"aoe2de://1/");
    else if (input.result == IDC_BUTTON_SPEC) LaunchLobby(input.code, L"aoe2de://1/", L"aoe2de://0/");
}

void CivPickerController::LaunchLobby(std::wstring lobby_code, const wchar_t* prefix, const wchar_t* other_prefix) const
{
    if (lobby_code.length() == 9)
    {
        OpenUrl((prefix + lobby_code).c_str());
        return;
    }
    if (lobby_code.substr(0, 11) == other_prefix) lobby_code.replace(0, 11, prefix);
    OpenUrl(lobby_code.c_str());
}

bool CivPickerController::IsValidLobbyCode(const std::wstring& lobby_code)
{
    const size_t length = lobby_code.length();
    if (length != 9 && length != 20) return false;

    if (length == 20)
    {
        if (lobby_code.substr(0, 11) != L"aoe2de://0/" && lobby_code.substr(0, 11) != L"aoe2de://1/") return false;
        const std::wstring numbers = lobby_code.substr(12, 20);
        if (numbers.find_first_not_of(L"0123456789") != std::wstring::npos) return false;
    }
    if (length == 9 && lobby_code.find_first_not_of(L"0123456789") != std::wstring::npos) return false;
    return true;
}
