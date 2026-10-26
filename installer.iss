#define MyAppName "KramerLab 3x3"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "KramerLab"

[Setup]
AppId={{8A9C4D72-4A52-4B43-9E31-71A4E72C1190}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\KramerLab
DefaultGroupName=KramerLab 3x3
OutputDir=installer-output
OutputBaseFilename=KramerLab-Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "build\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\KramerLab 3x3"; Filename: "{app}\bin\KramerLab.exe"
Name: "{autodesktop}\KramerLab 3x3"; Filename: "{app}\bin\KramerLab.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Crear un acceso directo en el escritorio"; GroupDescription: "Accesos directos:"
