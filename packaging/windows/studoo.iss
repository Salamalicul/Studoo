; ============================================================================
;  Instalador de Studoo para Windows (Inno Setup 6)
;  Lo compila GitHub automáticamente. Para compilarlo a mano:
;    ISCC.exe /DMyAppVersion=1.2.0 /DSourceExe=C:\ruta\Studoo.exe /OC:\salida studoo.iss
; ============================================================================
#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif
#ifndef SourceExe
  #define SourceExe "..\..\build\Studoo_artefacts\Release\Studoo.exe"
#endif

[Setup]
AppId={{8F3C2B1A-5D4E-4C7B-9A1F-3E2D1C0B9A87}
AppName=Studoo
AppVersion={#MyAppVersion}
AppVerName=Studoo {#MyAppVersion}
AppPublisher=Studoo
AppPublisherURL=https://github.com/Salamalicul/Studoo
AppComments=Estudio para hacer música y beats, en español
DefaultDirName={autopf}\Studoo
DefaultGroupName=Studoo
DisableProgramGroupPage=yes
; Se puede instalar sin ser administrador (solo para tu usuario)
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputBaseFilename=Studoo-Instalador
SetupIconFile=studoo.ico
UninstallDisplayIcon={app}\Studoo.exe
UninstallDisplayName=Studoo
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "{#SourceExe}"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\Studoo"; Filename: "{app}\Studoo.exe"; Comment: "Haz música y beats"
Name: "{autodesktop}\Studoo"; Filename: "{app}\Studoo.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\Studoo.exe"; Description: "{cm:LaunchProgram,Studoo}"; Flags: nowait postinstall skipifsilent
; Al actualizar desde la app (modo silencioso), volver a abrir Studoo
Filename: "{app}\Studoo.exe"; Flags: nowait; Check: WizardSilent
