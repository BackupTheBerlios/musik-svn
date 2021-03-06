; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!
#define BUILDDIR "Build\bin\x64"
#define MyAppName "wxMusik" ; define variable
#define MyAppVer GetFileVersion(BUILDDIR+"\Release Unicode\" + MyAppName + ".exe") ; define variable
#define MyAppExe MyAppName + ".exe" ;
#define MyAppPdb MyAppName + ".pdb" ;
[Setup]
AppName={#MyAppName}
#emit 'AppVerName=' + MyAppName + ' ' + MyAppVer
AppPublisherURL=http://musik.berlios.de
AppSupportURL=http://musik.berlios.de
AppUpdatesURL=http://musik.berlios.de
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=true
LicenseFile=license.txt

#emit 'OutputBaseFilename=' + MyAppName + '_v' + MyAppVer+ '_Setup_x64'
VersionInfoVersion={#MyAppVer}
VersionInfoDescription=A music player and library application
Compression=lzma/ultra64
ShowLanguageDialog=yes
AppID={{9EB87F75-0968-42F8-AD3F-01C2B13A1FE2}}
AlwaysShowDirOnReadyPage=true
AlwaysShowGroupOnReadyPage=true
AppCopyright=� 2008, Gunnar Roth, Casey Langen, Simon Windmill
DisableStartupPrompt=false
DisableFinishedPage=true
FlatComponentsList=false
ShowTasksTreeLines=true
ChangesAssociations=false
PrivilegesRequired=none
AppMutex=.{#MyAppName}.single_instance_check
InternalCompressLevel=ultra64
SolidCompression=true
MinVersion=0,5.01.2600
AppVersion=0.4.3
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: changelog.txt; DestDir: {app}; Flags: ignoreversion
Source: {#BUILDDIR}\Release Unicode\fmodex64.dll; DestDir: {app}; Flags: replacesameversion 64bit; MinVersion: 0,5.0.2195sp3; Components: ; Tasks: ; Languages: 
Source: license.txt; DestDir: {app}; Flags: ignoreversion
Source: {#BUILDDIR}\Release Unicode\MMShellHook.dll; DestDir: {app}; Flags: replacesameversion 64bit; MinVersion: 0,5.0.2195sp3
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

Source: contrib\playlists\*.mpd; DestDir: {userappdata}\..\.Musik\playlists; Flags: overwritereadonly; Components: Dynamic_Playlist_Examples
Source: contrib\playlists\*.mpu; DestDir: {userappdata}\..\.Musik\playlists; Flags: overwritereadonly; Components: Radio_Channels
Source: {#BUILDDIR}\Release Unicode\{#MyAppExe}; DestDir: {app}; MinVersion: 0,5.0.2195sp3; Flags: promptifolder replacesameversion 64bit; Components: ; Tasks: ; Languages: 
Source: data\*; DestDir: {app}\data; Flags: overwritereadonly recursesubdirs; Excludes: CMakeLists.txt
Source: locale\de\*.mo; DestDir: {app}\locale\de; Flags: overwritereadonly; Languages: de
Source: locale\fr\*.mo; DestDir: {app}\locale\fr; Flags: overwritereadonly; Languages: fr
Source: locale\cs\*.mo; DestDir: {app}\locale\cs; Flags: overwritereadonly; Languages: cs
Source: locale\nb\*.mo; DestDir: {app}\locale\nb; Flags: overwritereadonly; Languages: nb
Source: locale\pt_BR\*.mo; DestDir: {app}\locale\pt_BR; Flags: overwritereadonly; Languages: pt
Source: locale\es\*.mo; DestDir: {app}\locale\es; Flags: overwritereadonly; Languages: es
Source: locale\nl\*.mo; DestDir: {app}\locale\nl; Flags: overwritereadonly; Languages: nl
Source: locale\it\*.mo; DestDir: {app}\locale\it; Flags: overwritereadonly; Languages: it
Source: locale\ru\*.mo; DestDir: {app}\locale\ru; Flags: overwritereadonly; Languages: ru
Source: locale\pl\*.mo; DestDir: {app}\locale\pl; Flags: overwritereadonly; Languages: pl
[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#MyAppExe}; WorkingDir: {app}; IconIndex: 0
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {userdesktop}\{#MyAppName}; Filename: {app}\{#MyAppExe}; Tasks: desktopicon; WorkingDir: {app}; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}; Filename: {app}\{#MyAppExe}; Tasks: quicklaunchicon; WorkingDir: {app}; IconIndex: 0

[Run]
Filename: {app}\{#MyAppExe}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent
[Components]
Name: Dynamic_Playlist_Examples; Description: {cm:Examples_of_Dynamic_Playlists}; Types: custom full; MinVersion: 0,4.0.1381
Name: Radio_Channels; Description: {cm:Examples_of_Net_Radio_Channels}; Types: custom full; MinVersion: 0,4.0.1381
[InstallDelete]
Name: {app}\Musik.exe; Type: files
Name: {app}\*.pdb; Type: files
[UninstallDelete]

[Registry]

[Messages]
UninstalledAll=%1 was successfully removed from your computer. Click OK to decide what to do with your data files.

[Languages]
Name: en; MessagesFile: compiler:Default.isl
Name: de; MessagesFile: compiler:Languages\German.isl
Name: fr; MessagesFile: compiler:Languages\French.isl
Name: cs; MessagesFile: compiler:Languages\Czech.isl
Name: nb; MessagesFile: compiler:Languages\Norwegian.isl
Name: pt; MessagesFile: compiler:Languages\Portuguese.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl
Name: nl; MessagesFile: compiler:Languages\Dutch.isl
Name: it; MessagesFile: compiler:Languages\Italian.isl
Name: ru; MessagesFile: compiler:Languages\Russian.isl
Name: pl; MessagesFile: compiler:Languages\Polish.isl

[CustomMessages]
en.Examples_of_Dynamic_Playlists=Examples of Dynamic Playlists
en.Examples_of_Net_Radio_Channels=Examples of Net Radio Channels
de.Examples_of_Dynamic_Playlists=Beispiele f�r dynamische Liedlisten
de.Examples_of_Net_Radio_Channels=Beispiele f�r Netzradios
fr.Examples_of_Dynamic_Playlists=Exemples de s�lections dynamiques
fr.Examples_of_Net_Radio_Channels=Exemples de webradios
cs.Examples_of_Dynamic_Playlists=Uk�zky dynamick�ch seznamu
cs.Examples_of_Net_Radio_Channels=Uk�zky S�tov�ch Kan�lu R�dia
cs.Examples_of_Dynamic_Playlists=Examples of Dynamic Playlists
cs.Examples_of_Net_Radio_Channels=Examples of Net Radio Channels
nb.Examples_of_Dynamic_Playlists=Eksempler p� Dynamiske Spillelister
nb.Examples_of_Net_Radio_Channels=Eksempler p� Nettradio Kanaler
pt.Examples_of_Dynamic_Playlists=Exemplos de Sele��es musicais din�micas
pt.Examples_of_Net_Radio_Channels=Exemplos de webr�dios
es.Examples_of_Dynamic_Playlists=Ejemplos de Listas de reproducci�n Din�micas
es.Examples_of_Net_Radio_Channels=Ejemplos de canales de Radio de Red
nl.Examples_of_Dynamic_Playlists=Voorbeelden van dynamische afspeellijsten
nl.Examples_of_Net_Radio_Channels=Voorbeelden van webradiokanalen
it.Examples_of_Dynamic_Playlists=Esempi di Playlist Dinamiche
it.Examples_of_Net_Radio_Channels=Esempi di Canali Radio in Rete
ru.Examples_of_Dynamic_Playlists=Examples of Dynamic Playlists
ru.Examples_of_Net_Radio_Channels=Examples of Net Radio Channels
pl.Examples_of_Dynamic_Playlists=Examples of Dynamic Playlists
pl.Examples_of_Net_Radio_Channels=Examples of Net Radio Channels
[Code]
//when completing an uninstall
//ask user about any files they may want to keep
procedure DeInitializeUninstall();
var
  Success:  Boolean;
begin;
  if MsgBox('Delete all data files like the database, playlists etc. ?', mbConfirmation,MB_YESNO) = IDYES then begin;
	Success:= DelTree(ExpandConstant('{app}')+'\.Musik',True,True,True);
	Success:= DelTree(ExpandConstant('{userappdata}') + '\..\.Musik)',True,True,True);
	MsgBox('Removal of data files is complete',mbInformation, MB_OK);
  end;
end;
