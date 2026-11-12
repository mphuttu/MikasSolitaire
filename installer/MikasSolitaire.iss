#define AppName "MikasSolitaire"
#define AppVersion "1.0.0"
#ifndef VCRedist
  #define VCRedist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Redist\MSVC\v145\vc_redist.x64.exe"
#endif

[Setup]
AppId={{C34110D1-489E-4F07-8D65-75E8C601FE22}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=MikasSolitaire
AppPublisherURL=https://github.com/mphuttu/MikasSolitaire
DefaultDirName={autopf}\MikasSolitaire
DefaultGroupName=MikasSolitaire
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
OutputDir=Output
OutputBaseFilename=MikasSolitaire-Setup-x64
SetupIconFile=..\res\MikasSolitaireMainIco.ico
UninstallDisplayIcon={app}\MikasSolitaire.exe
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ShowLanguageDialog=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"

[CustomMessages]
english.HelpEnglish=Help (English)
finnish.HelpEnglish=Ohje (englanti)
english.HelpFinnish=Help (Finnish)
finnish.HelpFinnish=Ohje (suomi)
english.LaunchApp=Launch MikasSolitaire
finnish.LaunchApp=Käynnistä MikasSolitaire
english.RuntimeStartError=Could not start the Microsoft Visual C++ runtime installer.
finnish.RuntimeStartError=Microsoft Visual C++ -ajonaikaisympäristön asennusta ei voitu käynnistää.
english.RuntimeInstallError=The Microsoft Visual C++ runtime installation failed (exit code %1).
finnish.RuntimeInstallError=Microsoft Visual C++ -ajonaikaisympäristön asennus epäonnistui (paluuarvo %1).

[Files]
Source: "..\x64\Release\MikasSolitaire.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\help\MikasSolitaire.chm"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\help\fi\MikasSolitaire-fi.chm"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\res\front\*.png"; DestDir: "{app}\res\front"; Flags: ignoreversion
Source: "..\res\back\*.png"; DestDir: "{app}\res\back"; Flags: ignoreversion
Source: "{#VCRedist}"; Flags: dontcopy

[Icons]
Name: "{autoprograms}\MikasSolitaire\MikasSolitaire"; Filename: "{app}\MikasSolitaire.exe"
Name: "{autoprograms}\MikasSolitaire\{cm:HelpEnglish}"; Filename: "{app}\MikasSolitaire.chm"
Name: "{autoprograms}\MikasSolitaire\{cm:HelpFinnish}"; Filename: "{app}\MikasSolitaire-fi.chm"

[Run]
Filename: "{app}\MikasSolitaire.exe"; Description: "{cm:LaunchApp}"; Flags: nowait postinstall skipifsilent runasoriginaluser; Check: CanLaunch

[Code]
var
  RuntimeRestartRequired: Boolean;

function CanLaunch: Boolean;
begin
  Result := not RuntimeRestartRequired;
end;

function NeedRestart: Boolean;
begin
  Result := RuntimeRestartRequired;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  ResultCode: Integer;
begin
  Result := '';
  ExtractTemporaryFile('vc_redist.x64.exe');
  if not Exec(ExpandConstant('{tmp}\vc_redist.x64.exe'), '/install /quiet /norestart', '',
    SW_HIDE, ewWaitUntilTerminated, ResultCode) then
    Result := CustomMessage('RuntimeStartError')
  else if ResultCode = 3010 then
    RuntimeRestartRequired := True
  else if (ResultCode <> 0) and (ResultCode <> 1638) then
    Result := FmtMessage(CustomMessage('RuntimeInstallError'), [IntToStr(ResultCode)]);
end;
