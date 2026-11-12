# MikasSolitaire

A native Windows Klondike solitaire game built with C++, MFC and Direct2D.

## Features

- One-card stock draws with unlimited recycling.
- Click-to-move cards and sequences; double-click an exposed top card to move it to a foundation when legal.
- Six selectable card backs, remembered per Windows user.
- Save and open `.mksol` games, start a new deal, or restart the current deal from its original layout.
- Integrated Windows HTML Help with contents, index and search.

## Playing

Click the stock to draw; click the empty stock to recycle the waste. Click a face-up card or valid sequence and then its destination. Right-click cancels a selection. Double-click the top card of a tableau column or the waste to send it to a legal foundation.

Build tableau columns down in alternating colours; only kings can start an empty column. Build foundations up by suit from ace to king. A foundation's top card can be moved back to the tableau.

| Command | Action |
| --- | --- |
| Game > New game / Ctrl+N | Start a new shuffled deal |
| Game > Restart deal | Restart the current deal |
| Game > Open / Ctrl+O | Open a saved game |
| Game > Save / Ctrl+S | Save the current game |
| Game > Save as | Save a separate checkpoint |
| Game > Card back | Choose one of six backs |
| Help > Contents / F1 | Open the CHM help |

Save files contain both the current position and the original deal. Starting a new game, opening another game or exiting prompts about unsaved changes.

## Build

Install Visual Studio 2026 with the C++ desktop workload and MFC components, the Windows SDK, and [HTML Help Workshop](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/htmlhelp/microsoft-html-help-downloads). Open `MikasSolitaire.slnx` and build Debug or Release for x64. HTML Help Workshop compiles `help\MikasSolitaire.hhp` during the build. Its normal successful exit code is 1.

The output directory contains the executable, `MikasSolitaire.chm`, 52 face images in `res\front`, and six back images in `res\back`. Keep them together when running or distributing the application. The executable also needs the matching Microsoft Visual C++ runtime.

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

The tests cover dealing, moves, the foundation shortcut, saved-game validation and restarting a saved deal. For complete user instructions, press F1 or open the HTML pages in `help`.
