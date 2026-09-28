#ifndef MyAppVersion
#define MyAppVersion "0.0.0"
#endif
#ifndef DeployDir
#define DeployDir "..\..\dist\windows"
#endif

#define MyAppName "God of Pixels 3"
#define MyAppExeName "GodOfPixels3.exe"
#define MyAppPublisher "NikitaRiabovSoft"
#define MyAppURL "https://github.com/Nikita-080/God-of-pixels-3"

[Setup]
AppId={{8C3E1A7B-6D2F-4C91-9E4A-2B0F7D91C3A1}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=eula.txt
OutputDir=.
OutputBaseFilename=GodOfPixels3-{#MyAppVersion}-windows-x64-setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ChangesAssociations=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkedonce

[Files]
Source: "{#DeployDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Classes\.planet"; ValueType: string; ValueName: ""; ValueData: "GodOfPixels3.planet"; Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\Classes\GodOfPixels3.planet"; ValueType: string; ValueName: ""; ValueData: "God of Pixels 3 planet"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\GodOfPixels3.planet\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKCU; Subkey: "Software\Classes\GodOfPixels3.planet\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
