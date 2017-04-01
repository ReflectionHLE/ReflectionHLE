; reflection-keen.nsi
;
; This script is based on example2.nsi
;
;--------------------------------

; The name of the installer
Name "Reflection Keen"

; The file to write
OutFile "reflection-keen-installer.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Reflection Keen"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Reflection Keen" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Reflection Keen (required)"

  SectionIn RO
  
  ; Set output paths and put files.
  
  SetOutPath "$INSTDIR"
  File "reflection-kdreams.exe"
  File "reflection-cat3d.exe"
  File "reflection-catabyss.exe"
  File "reflection-catarm.exe"
  File "reflection-catapoc.exe"
  File "SDL2.dll"
  File "libspeexdsp-1.dll"
  File "README.txt"
  File "LICENSE.txt"
  File "LICENSE-depklite.txt"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Reflection_Keen "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Reflection Keen" "DisplayName" "Reflection Keen"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Reflection Keen" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Reflection Keen" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Reflection Keen" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  SetShellVarContext all

  CreateDirectory "$SMPROGRAMS\Reflection Keen"
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Reflection Keen Dreams.lnk" "$INSTDIR\reflection-kdreams.exe" "" "$INSTDIR\reflection-kdreams.exe" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Reflection Keen Dreams (Software).lnk" "$INSTDIR\reflection-kdreams.exe" "-softlauncher" "$INSTDIR\reflection-kdreams.exe" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Reflection Catacomb 3-D.lnk" "$INSTDIR\reflection-cat3d.exe" "" "$INSTDIR\reflection-cat3d.exe" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Reflection Catacomb 3-D (Software).lnk" "$INSTDIR\reflection-cat3d.exe" "-softlauncher" "$INSTDIR\reflection-cat3d.exe" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Reflection Catacomb Abyss.lnk" "$INSTDIR\reflection-catabyss.exe" "" "$INSTDIR\reflection-catabyss.exe" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Reflection Catacomb Abyss (Software).lnk" "$INSTDIR\reflection-catabyss.exe" "-softlauncher" "$INSTDIR\reflection-catabyss.exe" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Reflection Catacomb Armageddon.lnk" "$INSTDIR\reflection-catarm.exe" "" "$INSTDIR\reflection-catarm.exe" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Reflection Catacomb Armageddon (Software).lnk" "$INSTDIR\reflection-catarm.exe" "-softlauncher" "$INSTDIR\reflection-catarm.exe" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Reflection Catacomb Apocalypse.lnk" "$INSTDIR\reflection-catapoc.exe" "" "$INSTDIR\reflection-catapoc.exe" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Reflection Catacomb Apocalypse (Software).lnk" "$INSTDIR\reflection-catapoc.exe" "-softlauncher" "$INSTDIR\reflection-catapoc.exe" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Readme.lnk" "$INSTDIR\readme.txt" "" "$INSTDIR\readme.txt" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\License.lnk" "$INSTDIR\license.txt" "" "$INSTDIR\license.txt" 0
  CreateShortCut "$SMPROGRAMS\Reflection Keen\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Reflection Keen"
  DeleteRegKey HKLM SOFTWARE\Reflection_Keen

  ; Remove files and uninstaller
  Delete $INSTDIR\reflection-kdreams.exe
  Delete $INSTDIR\reflection-cat3d.exe
  Delete $INSTDIR\reflection-catabyss.exe
  Delete $INSTDIR\reflection-catarm.exe
  Delete $INSTDIR\reflection-catapoc.exe
  Delete $INSTDIR\SDL2.dll
  Delete $INSTDIR\libspeexdsp-1.dll
  Delete $INSTDIR\README.txt
  Delete $INSTDIR\LICENSE.txt
  Delete $INSTDIR\LICENSE-depklite.txt
  Delete $INSTDIR\uninstall.exe

  SetShellVarContext all

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Reflection Keen\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Reflection Keen"
  RMDir "$INSTDIR"

SectionEnd
