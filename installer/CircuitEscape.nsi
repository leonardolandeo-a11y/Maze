Unicode true
RequestExecutionLevel user
SetCompressor /SOLID lzma

!ifndef STAGING
  !error "STAGING must point to the prepared distribution directory."
!endif
!ifndef OUTPUT
  !error "OUTPUT must specify the setup executable."
!endif

Name "Circuito de Escape"
OutFile "${OUTPUT}"
InstallDir "$LOCALAPPDATA\Programs\CircuitoEscape"
ShowInstDetails show
ShowUninstDetails show

Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section "Install"
  SetOutPath "$INSTDIR"
  File /r "${STAGING}\*.*"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  CreateDirectory "$SMPROGRAMS\Circuito de Escape"
  CreateShortcut "$SMPROGRAMS\Circuito de Escape\Jugar.lnk" "$INSTDIR\launch-game.cmd"
  CreateShortcut "$SMPROGRAMS\Circuito de Escape\Generar mapas.lnk" "$INSTDIR\launch-generator.cmd"
  CreateShortcut "$SMPROGRAMS\Circuito de Escape\Desinstalar.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortcut "$DESKTOP\Circuito de Escape.lnk" "$INSTDIR\launch-game.cmd"

  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\CircuitoEscape" "DisplayName" "Circuito de Escape"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\CircuitoEscape" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\CircuitoEscape" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\CircuitoEscape" "Publisher" "CS2013 - Programacion III"
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\CircuitoEscape" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\CircuitoEscape" "NoRepair" 1
SectionEnd

Section "Uninstall"
  Delete "$DESKTOP\Circuito de Escape.lnk"
  RMDir /r "$SMPROGRAMS\Circuito de Escape"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\CircuitoEscape"
  RMDir /r "$INSTDIR"
SectionEnd
