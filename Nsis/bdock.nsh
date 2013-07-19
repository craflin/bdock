; bdock - NSIS Installer Script
; Copyright (C) 2013 Colin Graf

;--------------------------------
;Includes

  !include "MUI2.nsh"
  !include "WinMessages.nsh"

;--------------------------------
;Configuration

  ; General
!if "${ARCHITECTURE}" == "64-bit"
  Name "BDock ${VERSION} (${ARCHITECTURE})"
!else
  Name "BDock ${VERSION}"
!endif
  OutFile "BDock_${VERSION}_${ARCHITECTURE}.exe"
  
  ; Request admin rights
  RequestExecutionLevel admin
  
  ; Folder selection page
  InstallDir "$PROGRAMFILES\BDock"

  ; Get install folder from registry if available
  InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "InstallDirectory"

Function .onInit

  ; 64-bit mode?
!if "${ARCHITECTURE}" == "64-bit"
  SetRegView 64
  ReadRegStr $0 HKLM Software\Microsoft\Windows\CurrentVersion ProgramFilesDir
  StrCpy $INSTDIR "$0\BDock"
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "InstallDirectory"
  StrCmp $0 "" skipInstDirReg
  StrCpy $INSTDIR "$0"
  skipInstDirReg:
!endif

  ; Offer to start BDock in finish page
  !define MUI_FINISHPAGE_RUN_TEXT "Start BDock"
  !define MUI_FINISHPAGE_RUN "$INSTDIR\BDock.exe"

FunctionEnd

Function un.onInit

  ; 64-bit mode?
!if "${ARCHITECTURE}" == "64-bit"
  SetRegView 64
  ReadRegStr $0 HKLM Software\Microsoft\Windows\CurrentVersion ProgramFilesDir
  StrCpy $INSTDIR "$0\BDock"
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "InstallDirectory"
  StrCmp $0 "" skipInstDirReg
  StrCpy $INSTDIR "$0"
  skipInstDirReg:
!endif
  
FunctionEnd


;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "..\LICENSE"
;  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
;  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "!BDock ${VERSION}" MainSection

  SectionIn RO

  ; 64-bit mode?
!if "${ARCHITECTURE}" == "64-bit"
  SetRegView 64
!endif

  ; Close running version of bdock
  FindWindow $0 "" "BDOCK"
  StrCmp $0 0 notRunning
    SendMessage $0 ${WM_DESTROY} 0 0 
    sleep 500
  notRunning:

  ; Install program files
  SetOutPath "$INSTDIR"
  File "..\${BUILDDIR}\BDock.exe"
  SetOutPath "$INSTDIR\Plugins\Launcher"
  File "..\${BUILDDIR}\Launcher.dll"
  SetOutPath "$INSTDIR\Plugins\HideTaskbar"
  File "..\${BUILDDIR}\HideTaskbar.dll"
  
  ; Install classic skin
  SetOutPath "$INSTDIR\Skins\Default"
  File "..\Skins\Default\*.bmp"

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Install startup link FOR ALL USERS
  StrCpy $0 0
  loop:
    EnumRegKey $1 HKU "" $0
    StrCmp $1 "" done
    IntOp $0 $0 + 1
    ReadRegStr $2 HKU "$1\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Startup"
    StrCmp $2 "" loop
    CreateShortCut "$2\BDock.lnk" "$INSTDIR\BDock.exe" "" "$INSTDIR\BDock.exe" 0
    Goto loop
  done:

  ; Store install folder in registry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "InstallDirectory" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "InstallVersion" "${VERSION}"

  ; Install uninstaller to registry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "DisplayName" "${DISPLAYNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "DisplayIcon" "$INSTDIR\BDock.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "Publisher" "${PUBLISHER}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "NoModify" 0x01
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "NoRepair" 0x01
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "EstimatedSize" ${ESTIMATEDSIZE}
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock" "UninstallString" '"$INSTDIR\uninstall.exe"'

  ; Install start menu entries
  SetShellVarContext all
  Delete "$SMPROGRAMS\BDock\*.*"
  RMDir "$SMPROGRAMS\BDock"
  CreateDirectory "$SMPROGRAMS\BDock"
  CreateShortCut "$SMPROGRAMS\BDock\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\BDock\BDock.lnk" "$INSTDIR\BDock.exe" "" "$INSTDIR\BDock.exe" 0

SectionEnd

;--------------------------------
;Descriptions

  LangString DESC_MainSection ${LANG_ENGLISH} "BDock program files."

;  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
;    !insertmacro MUI_DESCRIPTION_TEXT ${MainSection} $(DESC_MainSection)
;  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ; 64-bit mode?
!if "${ARCHITECTURE}" == "64-bit"
  SetRegView 64
!endif

  ; Close running version of bdock
  FindWindow $0 "" "BDOCK"
  StrCmp $0 0 notRunning
    SendMessage $0 ${WM_DESTROY} 0 0 
    sleep 500
  notRunning:

  ; Delete program dir
  RMDir /R "$INSTDIR"
  
  ; Remove settings directory in appdata folder and startup link FOR ALL USERS
  StrCpy $0 0
  loop:
    EnumRegKey $1 HKU "" $0
    StrCmp $1 "" done
    IntOp $0 $0 + 1
    ReadRegStr $2 HKU "$1\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "AppData"
    StrCmp $2 "" loop
    RMDir /R "$2\BDock"
    ReadRegStr $2 HKU "$1\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Startup"
    Delete "$2\BDock.lnk"
    Goto loop
  done:

  ; Remove uninstaller from registry
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BDock"

  ; Delete start menu entries
  SetShellVarContext all
  Delete "$SMPROGRAMS\BDock\*.*"
  RMDir "$SMPROGRAMS\BDock"
  
SectionEnd
