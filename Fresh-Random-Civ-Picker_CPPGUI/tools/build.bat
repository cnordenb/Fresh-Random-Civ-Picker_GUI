@echo off
rem Builds FRCP_GUI.exe with MinGW-w64 (g++ and windres) into bin\, with data.json, the images and the jingles beside it.
cd /d "%~dp0"
set SRC=Fresh-Random-Civ-Picker_CPPGUI
set OUT=bin
if not exist %OUT% mkdir %OUT%

rem windres can't read the UTF-16 resource script Visual Studio saves, so it gets a UTF-8 copy.
powershell -NoProfile -Command "Get-Content '%SRC%\FRCP_GUI.rc' -Encoding Unicode | Set-Content '%OUT%\FRCP_GUI.rc' -Encoding UTF8" || exit /b 1
rem The manifest asks Windows for Common Controls 6, which the program needs to start.
echo 1 24 "%SRC%\\FRCP_GUI.manifest">> %OUT%\FRCP_GUI.rc
windres --include-dir %SRC% %OUT%\FRCP_GUI.rc -O coff -o %OUT%\app.res || exit /b 1

g++ -std=c++17 -O2 -municode -mwindows -static -D_WIN32_WINNT=0x0A00 -DUNICODE -D_UNICODE -DNDEBUG -D_WINDOWS -DNOMINMAX ^
    -I%SRC% -I%SRC%\common -I%SRC%\model -I%SRC%\controller -I%SRC%\view ^
    -I%SRC%\parchment\graphics -I%SRC%\parchment\controls -I%SRC%\parchment\windows ^
    %SRC%\Main.cpp %SRC%\common\*.cpp %SRC%\model\*.cpp %SRC%\controller\*.cpp %SRC%\view\*.cpp ^
    %SRC%\parchment\graphics\*.cpp %SRC%\parchment\controls\*.cpp %SRC%\parchment\windows\*.cpp ^
    %OUT%\app.res -o %OUT%\FRCP_GUI.exe ^
    -lcomctl32 -ld2d1 -ldwrite -lshcore -lshlwapi -lwindowscodecs -lwinmm -lole32 -luuid -lgdi32 -lcomdlg32 -lshell32 ^
    || exit /b 1

copy /y %SRC%\data.json %OUT%\ >nul || exit /b 1
xcopy /e /i /y /q %SRC%\images %OUT%\images >nul || exit /b 1
xcopy /e /i /y /q %SRC%\sounds\civ_jingles %OUT%\sounds\civ_jingles >nul || exit /b 1
echo Built %OUT%\FRCP_GUI.exe
