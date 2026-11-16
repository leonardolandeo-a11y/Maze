Unicode true
!include "x64.nsh"
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

Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "Circuito de Escape requires Windows x64."
    Abort
  ${EndIf}
  SetRegView 64
FunctionEnd

Function un.onInit
  SetRegView 64
FunctionEnd

Section "Install"
  SetOutPath "$INSTDIR"
  File /r "${STAGING}\*.*"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  CreateDirectory "$SMPROGRAMS\Circuito de Escape"
  CreateShortcut "$SMPROGRAMS\Circuito de Escape\Jugar.lnk" "$INSTDIR\launch-game.cmd"
  CreateShortcut "$SMPROGRAMS\Circuito de Escape\Generar mapas.lnk" "$INSTDIR\launch-generator.cmd"
  CreateShortcut "$SMPROGRAMS\Circuito de Escape\Jugar mapas generados.lnk" "$INSTDIR\launch-generated-game.cmd"
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
  Delete "$INSTDIR\bin\escape_circuit.exe"
  Delete "$INSTDIR\bin\map_generator.exe"
  Delete "$INSTDIR\bin\*.dll"
  RMDir "$INSTDIR\bin"
  Delete "$INSTDIR\assets\maps\mapa_facil.txt"
  Delete "$INSTDIR\assets\maps\mapa_medio.txt"
  Delete "$INSTDIR\assets\maps\mapa_dificil.txt"
  RMDir "$INSTDIR\assets\maps"
  RMDir "$INSTDIR\assets"
  Delete "$INSTDIR\README.md"
  Delete "$INSTDIR\launch-game.cmd"
  Delete "$INSTDIR\launch-generator.cmd"
  Delete "$INSTDIR\launch-generated-game.cmd"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
SectionEnd
