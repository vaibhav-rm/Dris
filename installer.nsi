; Dris Installer Script
; Requires NSIS

Target amd64-unicode

;--------------------------------
;General

  ;Name and file
  Name "Dris Image Viewer"
  OutFile "dist/windows/DrisInstaller.exe"
  Unicode True
  Target amd64-unicode

  ;Default usage
  InstallDir "$PROGRAMFILES64\Dris"
  InstallDirRegKey HKLM "Software\Dris" "Install_Dir"

  ;Request application privileges for Windows Vista+
  RequestExecutionLevel admin

;--------------------------------
;Interface Settings

  !include "MUI2.nsh"

  !define MUI_ABORTWARNING

  ;Pages
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Dris Core (Required)" SecDris
  SectionIn RO
  
  SetOutPath "$INSTDIR"
  
  ; Files to install
  File "dist/windows/dris.exe"
  File "dist/windows/SDL2.dll"
  File "dist/windows/README.txt"
  
  ; Store install folder
  WriteRegStr HKLM "Software\Dris" "Install_Dir" "$INSTDIR"
  
  ; Write uninstall keys
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dris" "DisplayName" "Dris Image Viewer"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dris" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dris" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dris" "NoRepair" 1
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
SectionEnd

Section "Start Menu Shortcuts" SecShortcuts
  CreateDirectory "$SMPROGRAMS\Dris"
  CreateShortcut "$SMPROGRAMS\Dris\Dris.lnk" "$INSTDIR\dris.exe" "" "$INSTDIR\dris.exe" 0
  CreateShortcut "$SMPROGRAMS\Dris\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

Section "Register File Associations" SecAssoc
  ; Register .png, .jpg, .bmp
  
  ; PNG
  WriteRegStr HKCR ".png" "" "Dris.Image"
  
  ; JPG
  WriteRegStr HKCR ".jpg" "" "Dris.Image"
  WriteRegStr HKCR ".jpeg" "" "Dris.Image"
  
  ; BMP
  WriteRegStr HKCR ".bmp" "" "Dris.Image"
  
  ; Dris File Type
  WriteRegStr HKCR "Dris.Image" "" "Image File"
  WriteRegStr HKCR "Dris.Image\DefaultIcon" "" "$INSTDIR\dris.exe,0"
  WriteRegStr HKCR "Dris.Image\shell\open\command" "" '"$INSTDIR\dris.exe" "%1"'
SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dris"
  DeleteRegKey HKLM "Software\Dris"
  
  ; Remove file associations (optional - strictly speaking we should only remove if we own them, but for simple app...)
  ; DeleteRegKey HKCR "Dris.Image" 
  ; Not deleting .png etc to avoid breaking other apps if they were set. Windows handles "Open With" gracefully.

  ; Remove files and uninstaller
  Delete "$INSTDIR\dris.exe"
  Delete "$INSTDIR\SDL2.dll"
  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\uninstall.exe"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Dris\*.*"
  RMDir "$SMPROGRAMS\Dris"

  ; Remove directories
  RMDir "$INSTDIR"

SectionEnd
