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
SetupIconFile=..\res\MikasSolitaire.ico
UninstallDisplayIcon={app}\MikasSolitaire.exe
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

[Files]
Source: "..\x64\Release\MikasSolitaire.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\help\MikasSolitaire.chm"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\res\front\*.png"; DestDir: "{app}\res\front"; Flags: ignoreversion
Source: "..\res\back\*.png"; DestDir: "{app}\res\back"; Flags: ignoreversion
Source: "{#VCRedist}"; Flags: dontcopy

[Icons]
Name: "{autoprograms}\MikasSolitaire\MikasSolitaire"; Filename: "{app}\MikasSolitaire.exe"
Name: "{autoprograms}\MikasSolitaire\Help"; Filename: "{app}\MikasSolitaire.chm"

[Run]
Filename: "{app}\MikasSolitaire.exe"; Description: "Launch MikasSolitaire"; Flags: nowait postinstall skipifsilent runasoriginaluser; Check: CanLaunch

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
    Result := 'Could not start the Microsoft Visual C++ runtime installer.'
  else if ResultCode = 3010 then
    RuntimeRestartRequired := True
  else if (ResultCode <> 0) and (ResultCode <> 1638) then
    Result := Format('The Microsoft Visual C++ runtime installation failed (exit code %d).', [ResultCode]);
end;
