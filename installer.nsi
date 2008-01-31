;Installer for Strata
;Written by Albert Kottke

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;Revision of the respository
  !system 'svnversion.exe . | perl.exe -pne"s/(?:\d+:)?(\d+)(?:[MS]+)?$/!define REVISION \1/" > %TEMP%\revision.nsh'
  !include "$%TEMP%\revision.nsh"

;--------------------------------
;Variables
  Var MUI_TEMP
  Var STARTMENU_FOLDER

;--------------------------------
;General

  ;Name and file
  Name "Strata"
  OutFile "StrataInstaller-rev-${REVISION}.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Strata"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Strata" ""

  ;Vista redirects $SMPROGRAMS to all users without this
  RequestExecutionLevel admin

  ;Set the type of compress to LZMA
  SetCompressor lzma

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ; Start menu folder page configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Strata"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU Strata $STARTMENU_FOLDER
  !insertmacro MUI_PAGE_INSTFILES
  ; Finish page configuration
  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"
  !define MUI_FINISHPAGE_RUN "$INSTDIR\strata.exe"
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Association macros -- from: http://nsis.sourceforge.net/FileAssoc
!macro APP_ASSOCIATE EXT FILECLASS DESCRIPTION ICON COMMANDTEXT COMMAND
  ; Backup the previously associated file class
  ReadRegStr $R0 HKCR ".${EXT}" ""
  WriteRegStr HKCR ".${EXT}" "${FILECLASS}_backup" "$R0"
 
  WriteRegStr HKCR ".${EXT}" "" "${FILECLASS}"
 
  WriteRegStr HKCR "${FILECLASS}" "" `${DESCRIPTION}`
  WriteRegStr HKCR "${FILECLASS}\DefaultIcon" "" `${ICON}`
  WriteRegStr HKCR "${FILECLASS}\shell" "" "open"
  WriteRegStr HKCR "${FILECLASS}\shell\open" "" `${COMMANDTEXT}`
  WriteRegStr HKCR "${FILECLASS}\shell\open\command" "" `${COMMAND}`
!macroend

!macro APP_UNASSOCIATE EXT FILECLASS
  ; Backup the previously associated file class
  ReadRegStr $R0 HKCR ".${EXT}" `${FILECLASS}_backup`
  WriteRegStr HKCR ".${EXT}" "" "$R0"
 
  DeleteRegKey HKCR `${FILECLASS}`
!macroend

;--------------------------------
;Installer Sections

Section "!Core Files" SecProgram
  SectionIn RO
  SetOutPath "$INSTDIR"

  ;Main Strata files
  file "bin\strata.exe"
  file "/oname=$INSTDIR\readme.txt" "README"

  ;Icons
  file "/oname=$INSTDIR\strata.ico" "resources\images\application-icon.ico"
  file "/oname=$INSTDIR\strata-input.ico" "resources\images\file-input.ico"
  file "/oname=$INSTDIR\strata-output.ico" "resources\images\file-output.ico"

  ;Main libraries
  file "C:\devel\fftw-3.1.2\libfftw3-3.dll"
  file "C:\devel\GnuWin32\bin\libgsl.dll"
  file "C:\devel\GnuWin32\bin\libgslcblas.dll"
  file "C:\devel\Qt-4.3.3\bin\mingwm10.dll"
  file "c:\devel\Qt-4.3.3\bin\QtCore4.dll"
  file "c:\devel\Qt-4.3.3\bin\QtGui4.dll"
  file "c:\devel\Qt-4.3.3\bin\QtScript4.dll"
  file "c:\devel\Qt-4.3.3\bin\QtSvg4.dll"
  file "c:\devel\Qt-4.3.3\bin\QtXml4.dll"
  file "C:\devel\qwt-5.0\lib\qwt5.dll"
  
  ;Plugins for SVG icons
  SetOutPath "$INSTDIR\iconengines" 
  file "c:\devel\Qt-4.3.3\plugins\iconengines\qsvg4.dll"
  SetOutPath "$INSTDIR\imageformats" 
  file "c:\devel\Qt-4.3.3\plugins\imageformats\qsvg4.dll"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\Strata" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;File associations
  !insertmacro APP_ASSOCIATE "stri" "strata.inputfile" "Strata Input" "$INSTDIR\strata-input.ico" "Edit/Run with Strata" "$INSTDIR\strata.exe $\"%1$\""
  !insertmacro APP_ASSOCIATE "stro" "strata.outputfile" "Strata Output" "$INSTDIR\strata-output.ico" "View with Strata" "$INSTDIR\strata.exe $\"%1$\""

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Strata
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Strata.lnk" "$INSTDIR\Strata.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section "Documentation" SecDocumentation
	SetOutPath $INSTDIR

	file "manual\manual.pdf"

SectionEnd

Section "Examples" SecExamples
	
	SetOutPath "$INSTDIR\examples"

	;Files to install
	file "example\NIS090.AT2"
	file "example\shake-example.stri"
	file "example\shake-example-fas.stri"
	file "example\shake-example-rvt.stri"

SectionEnd

Section /o "Source" SecSource

	SetOutPath "$INSTDIR\src"

	;Files to install
	file "/oname=..\strata.pro" "strata.pro"
	file "src\*.h"
	file "src\*.cpp"
SectionEnd

;--------------------------------
;Descriptions
  ;Language strings
  LangString DESC_SecProgram ${LANG_ENGLISH} "Binaries required to run Strata."
  LangString DESC_SecDocumentation ${LANG_ENGLISH} "User manual."
  LangString DESC_SecExamples ${LANG_ENGLISH} "Example input files."
  LangString DESC_SecSource ${LANG_ENGLISH} "Source code."


  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecProgram} $(DESC_SecProgram)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDocumentation} $(DESC_SecDocumentation)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecExamples} $(DESC_SecExamples)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSource} $(DESC_SecSource)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  ;Remove the files
  Delete "$INSTDIR\libfftw3-3.dll"
  Delete "$INSTDIR\libgsl.dll"
  Delete "$INSTDIR\libgslcblas.dll"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtScript4.dll"
  Delete "$INSTDIR\QtSvg4.dll"
  Delete "$INSTDIR\QtXml4.dll"
  Delete "$INSTDIR\qwt5.dll"
  Delete "$INSTDIR\iconengines\qsvg4.dll"
  RMDir  "$INSTDIR\iconengines"
  Delete "$INSTDIR\imageformats\qsvg4.dll"
  RMDir  "$INSTDIR\imageformats"
  Delete "$INSTDIR\example\NIS090.AT2"
  Delete "$INSTDIR\example\shake-example.stri"
  Delete "$INSTDIR\example\shake-example-fas.stri"
  Delete "$INSTDIR\example\shake-example-rvt.stri"
  RMDir  "$INSTDIR\example\"
  Delete "$INSTDIR\strata.pro"

  ;Remove all source files
  RMDir /r "$INSTDIR\src"

  ;Remove the install directory
  RMDir $INSTDIR

  ;Remove the association with Strata
  !insertmacro APP_UNASSOCIATE "stri" "strata.inputfile"
  !insertmacro APP_UNASSOCIATE "stro" "strata.outputfile"

  ;Remove the Start menu items
  !insertmacro MUI_STARTMENU_GETFOLDER Strata $MUI_TEMP
   
  ;Delete the shortcuts
  Delete "$SMPROGRAMS\$MUI_TEMP\Strata.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  
  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
 
  startMenuDeleteLoop:
	ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    
    IfErrors startMenuDeleteLoopDone
  
    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

  DeleteRegKey /ifempty HKCU "Software\Strata"
SectionEnd
