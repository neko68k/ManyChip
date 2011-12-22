

; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "Highly Advance GSF Player"
!define PRODUCT_VERSION "0.11ni"
!define PRODUCT_WEB_SITE "http://www.caitsith2.com/gsf/"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "HA_${PRODUCT_VERSION}.exe"


; The default installation directory
InstallDir $PROGRAMFILES\Winamp

; detect winamp path from uninstall string if available
InstallDirRegKey HKLM \
                 "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" \
                 "UninstallString"

; The text to prompt the user to enter a directory
DirText "Please select your Winamp path below (you will be able to proceed when Winamp is detected):"
# currently doesn't work - DirShow hide

; automatically close the installer when done.
AutoCloseWindow true

; hide the "show details" box
ShowInstDetails nevershow

;--------------------------------

;Pages

Page directory
Page instfiles

;--------------------------------

; The stuff to install

Section ""

  Call QueryWinampPath

  SetOutPath $1
  File "No_Interpolation\in_gsf.dll"
	File "ha version history.txt"


SectionEnd

;--------------------------------

Function .onVerifyInstDir

  ;Check for Winamp installation

  IfFileExists $INSTDIR\Winamp.exe Good
	IfFileExists $INSTDIR\xmplay.exe XMGood
    Abort
  Good:
		StrCpy $1 $INSTDIR\Plugins
		Goto End
	XMGood:
		StrCpy $1 $INSTDIR
	End:

FunctionEnd

Function QueryWinampPath ; sets $1 with path

  ;StrCpy $1 $INSTDIR
  
FunctionEnd