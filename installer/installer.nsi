; ============================================================================
; FFmpeg Converter — NSIS Installer Script
; ============================================================================
; Requirements:
;   - NSIS 3.x         (https://nsis.sourceforge.io)
;   - Build output in ..\build-release\Release\
;   - Icon at          ..\resources\icons\app.ico  (optional but recommended)
;
; Called automatically by deploy.ps1, or manually:
;   makensis installer\installer.nsi
; ============================================================================

Unicode True

; ----------------------------------------------------------------------------
; General metadata
; ----------------------------------------------------------------------------
!define APP_NAME        "FFmpeg Converter"
!define APP_EXE         "FFmpegWrapper.exe"
!define APP_VERSION     "1.0.0"
!define APP_PUBLISHER   "FFmpegWrapper"
!define APP_URL         "https://github.com/youruser/ffmpeg-wrapper"
!define INSTALL_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\FFmpegConverter"
!define BUILD_DIR       "..\build-release\Release"

; Output filename (generated next to this .nsi file)
OutFile "FFmpegConverter-Setup-${APP_VERSION}.exe"

; Default install directory
InstallDir "$PROGRAMFILES64\${APP_NAME}"

; Registry key to remember install path for upgrades / uninstall
InstallDirRegKey HKLM "${INSTALL_REG_KEY}" "InstallLocation"

; Request admin rights — needed to write to Program Files
RequestExecutionLevel admin

; ----------------------------------------------------------------------------
; Appearance
; ----------------------------------------------------------------------------
Name "${APP_NAME} ${APP_VERSION}"
BrandingText "${APP_PUBLISHER}"

; Modern UI
!include "MUI2.nsh"

!define MUI_ICON              "..\resources\icons\app.ico"
!define MUI_UNICON             "..\resources\icons\app.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP  "welcome.bmp"   ; 164x314 BMP
!define MUI_ABORTWARNING

; Finish page — offer to launch the app
!define MUI_FINISHPAGE_RUN          "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT     "Launch ${APP_NAME}"
!define MUI_FINISHPAGE_SHOWREADME   ""
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED

; ----------------------------------------------------------------------------
; Installer pages
; ----------------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
Page custom ShortcutsPage ShortcutsPageLeave
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; ----------------------------------------------------------------------------
; Language
; ----------------------------------------------------------------------------
!insertmacro MUI_LANGUAGE "English"

; ----------------------------------------------------------------------------
; Shortcuts page (custom) — checkboxes for Desktop and Start Menu
; ----------------------------------------------------------------------------
Var ShortcutDesktop
Var ShortcutStartMenu

Function ShortcutsPage
    !insertmacro MUI_HEADER_TEXT "Create Shortcuts" "Choose where to create shortcuts."

    nsDialogs::Create 1018
    Pop $0

    ${NSD_CreateLabel} 0 0 100% 20u "Select the shortcuts you want the installer to create:"
    Pop $0

    ${NSD_CreateCheckbox} 10u 30u 100% 12u "Create a shortcut on the Desktop"
    Pop $ShortcutDesktop
    ${NSD_SetState} $ShortcutDesktop ${BST_CHECKED}

    ${NSD_CreateCheckbox} 10u 50u 100% 12u "Add to the Start Menu"
    Pop $ShortcutStartMenu
    ${NSD_SetState} $ShortcutStartMenu ${BST_CHECKED}

    nsDialogs::Show
FunctionEnd

Function ShortcutsPageLeave
    ${NSD_GetState} $ShortcutDesktop  $ShortcutDesktop
    ${NSD_GetState} $ShortcutStartMenu $ShortcutStartMenu
FunctionEnd

; ----------------------------------------------------------------------------
; Main install section
; ----------------------------------------------------------------------------
Section "Main Application" SecMain
    SectionIn RO   ; required — user cannot deselect

    SetOutPath "$INSTDIR"

    ; Copy everything from the Release build directory
    File /r "${BUILD_DIR}\*.*"

    ; --- Desktop shortcut ---
    ${If} $ShortcutDesktop == ${BST_CHECKED}
        CreateShortcut "$DESKTOP\${APP_NAME}.lnk" \
            "$INSTDIR\${APP_EXE}" "" \
            "$INSTDIR\${APP_EXE}" 0
    ${EndIf}

    ; --- Start Menu shortcut ---
    ${If} $ShortcutStartMenu == ${BST_CHECKED}
        CreateDirectory "$SMPROGRAMS\${APP_NAME}"
        CreateShortcut  "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" \
            "$INSTDIR\${APP_EXE}" "" \
            "$INSTDIR\${APP_EXE}" 0
        CreateShortcut  "$SMPROGRAMS\${APP_NAME}\Uninstall ${APP_NAME}.lnk" \
            "$INSTDIR\Uninstall.exe"
    ${EndIf}

    ; --- Write uninstaller ---
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; --- Add/Remove Programs entry ---
    WriteRegStr   HKLM "${INSTALL_REG_KEY}" "DisplayName"      "${APP_NAME}"
    WriteRegStr   HKLM "${INSTALL_REG_KEY}" "DisplayVersion"   "${APP_VERSION}"
    WriteRegStr   HKLM "${INSTALL_REG_KEY}" "Publisher"        "${APP_PUBLISHER}"
    WriteRegStr   HKLM "${INSTALL_REG_KEY}" "URLInfoAbout"     "${APP_URL}"
    WriteRegStr   HKLM "${INSTALL_REG_KEY}" "InstallLocation"  "$INSTDIR"
    WriteRegStr   HKLM "${INSTALL_REG_KEY}" "UninstallString"  "$INSTDIR\Uninstall.exe"
    WriteRegStr   HKLM "${INSTALL_REG_KEY}" "DisplayIcon"      "$INSTDIR\${APP_EXE}"
    WriteRegDWORD HKLM "${INSTALL_REG_KEY}" "NoModify"         1
    WriteRegDWORD HKLM "${INSTALL_REG_KEY}" "NoRepair"         1

    ; Estimate install size (in KB) for Add/Remove Programs
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "${INSTALL_REG_KEY}" "EstimatedSize" "$0"

SectionEnd

; ----------------------------------------------------------------------------
; Uninstall section
; ----------------------------------------------------------------------------
Section "Uninstall"

    ; Remove installed files
    RMDir /r "$INSTDIR"

    ; Remove shortcuts
    Delete "$DESKTOP\${APP_NAME}.lnk"
    RMDir /r "$SMPROGRAMS\${APP_NAME}"

    ; Remove Add/Remove Programs entry
    DeleteRegKey HKLM "${INSTALL_REG_KEY}"

SectionEnd
