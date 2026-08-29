; Inno Setup script for the DRUMid installer.
;
;   iscc /DAppVersion=1.0.0 /DPluginDir=<folder holding DRUMid.vst3> ^
;        /DOutputDir=<where to write the exe> packaging\windows\drumid.iss
;
; The plugin goes into the company folder inside the shared VST3 directory,
; which is where hosts look and where the macOS side puts it too.

#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif
#ifndef PluginDir
  #define PluginDir "..\..\build\DRUMid_artefacts\Release\VST3"
#endif
#ifndef OutputDir
  #define OutputDir "..\..\dist"
#endif

[Setup]
AppId={{7C3F9A41-6E52-4B08-9D17-2A85C6E4F310}
AppName=DRUMid
AppVersion={#AppVersion}
AppPublisher=Nowhr Dynamics
DefaultDirName={commoncf64}\VST3\Nowhr Dynamics
; Ask where to install. The default is the shared VST3 folder every DAW scans,
; so most people just click Next - but a host pointed at its own folder
; (Cubase, for instance) is common enough that a fixed path would leave those
; people with a plugin the DAW never looks at.
DisableDirPage=no
; What you choose is where it goes, instead of Inno appending the company name
; to a folder you picked on purpose.
AppendDefaultDirName=no
DisableProgramGroupPage=yes
UninstallDisplayName=DRUMid
; Follows the chosen folder, not the default.
UninstallFilesDir={app}\DRUMid-uninstall
OutputDir={#OutputDir}
OutputBaseFilename=DRUMid-{#AppVersion}-Windows
Compression=lzma2
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=admin
WizardStyle=modern

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "pt"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Files]
Source: "{#PluginDir}\DRUMid.vst3\*"; DestDir: "{app}\DRUMid.vst3"; \
    Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]

[Run]
