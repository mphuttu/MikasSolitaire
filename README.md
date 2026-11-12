# MikasSolitaire

A native Windows Klondike solitaire game built with C++, MFC and Direct2D.

## Features

- One-card stock draws with unlimited recycling.
- Click or drag cards and sequences; double-click an exposed top card to move it to a foundation when legal.
- Undo moves, stock draws and recycling; get a warning when no forward moves remain and celebrate a win with a card animation.
- Six selectable card backs, remembered per Windows user.
- Save and open `.mksol` games, start a new deal, or restart the current deal from its original layout.
- Integrated Windows HTML Help with contents, index and search.
- English and Finnish application and help, plus a bilingual installer.
- A custom application and installer icon.

## Recent updates

- Added undo, card dragging with a hand cursor, no-forward-moves notifications and a win animation.
- Added Finnish localization for the game and help, and an English/Finnish installer.
- Corrected Finnish accented characters in menus. On the first Finnish startup after upgrading, the saved menu is refreshed, resetting any menu customizations.

## Playing

Click the stock to draw; click the empty stock to recycle the waste. Drag a face-up card or valid sequence to its destination, or click the card and then its destination. Right-click cancels a selection. Double-click the top card of a tableau column or the waste to send it to a legal foundation.

Build tableau columns down in alternating colours; only kings can start an empty column. Build foundations up by suit from ace to king. A foundation's top card can be moved back to the tableau.

| Command | Action |
| --- | --- |
| Game > New game / Ctrl+N | Start a new shuffled deal |
| Game > Restart deal | Restart the current deal |
| Game > Undo / Ctrl+Z | Undo the last move, draw or recycle |
| Game > Open / Ctrl+O | Open a saved game |
| Game > Save / Ctrl+S | Save the current game |
| Game > Save as | Save a separate checkpoint |
| Game > Card back | Choose one of six backs |
| Game > Language / Kieli | Choose English or Finnish for the next application startup |
| Help > Contents / F1 | Open the CHM help |

Save files contain both the current position and the original deal, but not undo history. Undo history is cleared when starting, restarting or opening a deal. Starting a new game, opening another game or exiting prompts about unsaved changes.

The application language is a per-user preference and takes effect after restarting the application; help opens in that language. Changing language resets customized menus. Some Windows and MFC-provided dialogs may use the Windows language or English. The installer offers its own independent language choice.

The no-forward-moves warning checks tableau moves and all cards reachable by cycling the stock. It excludes moves from foundations and moving an entire king column to an empty column; such moves and undo remain available after dismissing the warning. You can restart the same deal, save, continue, start a new deal or exit.

## Build

Install Visual Studio 2026 with the C++ desktop workload and MFC components, the Windows SDK, and [HTML Help Workshop](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/htmlhelp/microsoft-html-help-downloads). Open `MikasSolitaire.slnx` and build Debug or Release for x64. HTML Help Workshop compiles `help\MikasSolitaire.hhp` and `help\fi\MikasSolitaire.hhp` during the build. Its normal successful exit code is 1.

Keep `res\Finnish.rc2` encoded as UTF-16 LE with a byte-order mark: the Windows resource compiler otherwise corrupts accented menu text.

The output directory contains the executable, `MikasSolitaire.chm`, `MikasSolitaire-fi.chm`, 52 face images in `res\front`, and six back images in `res\back`. Keep them together when running or distributing the application. The executable also needs the matching Microsoft Visual C++ runtime.

## Installer

Build **Release | x64**, then run:

```powershell
& 'C:\Program Files (x86)\Inno Setup 6\ISCC.exe' 'installer\MikasSolitaire.iss'
```

The resulting `installer\Output\MikasSolitaire-Setup-x64.exe` targets 64-bit Windows 10 or later and includes the application, card images, help, and the x64 Visual C++ redistributable. Setup requests administrator permission and installs or repairs the runtime. The script expects the redistributable at `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Redist\MSVC\v145\vc_redist.x64.exe`; adjust the `VCRedist` definition in the script if your Visual Studio installation differs. The installer is not code-signed.

## Tests

Build Debug x64. From an x64 Native Tools Command Prompt in the project directory, run:

```bat
cl /EHsc /MDd /D_DEBUG /D_AFXDLL /D_UNICODE /DUNICODE /I. tests\GameTests.cpp MikasSolitaire\x64\Debug\Card.obj MikasSolitaire\x64\Debug\Deck.obj MikasSolitaire\x64\Debug\Game.obj MikasSolitaire\x64\Debug\pch.obj /Fe:GameTests.exe
GameTests.exe
```

From the same prompt, check the embedded language resources, menus and icon:

```bat
cl /EHsc tests\LocalizationTests.cpp /Fe:LocalizationTests.exe
LocalizationTests.exe x64\Debug\MikasSolitaire.exe res\MikasSolitaireMainIco.ico
```

The tests cover dealing, moves, undo, forward-move detection, the foundation shortcut, saved-game validation and restarting a saved deal. For complete user instructions, press F1 or open the HTML pages in `help`.
