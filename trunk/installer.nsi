;Installer for Strata
;Written by Albert Kottke

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;Revision of the respository
  !system 'python getSvnVersion.py "!define REVISION" > %TEMP%\revision.nsh'
  !include "$%TEMP%\revision.nsh"

;--------------------------------
; Path to Qt
!Define QT_PATH "C:\devel\Qt\4.8.6"
!Define MINGW_PATH "C:\Program Files\mingw-w64\i686-4.8.2-posix-dwarf-rt_v3-rev4\bin"

;--------------------------------
;Variables
  Var MUI_TEMP
  Var STARTMENU_FOLDER

;--------------------------------
;General

  ;Name and file
  Name "Strata"
  OutFile "Strata-rev-${REVISION}.exe"

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
  !define MUI_FINISHPAGE_RUN "$INSTDIR\strata.exe"
  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"
  !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
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
  file "release\strata.exe"
  file "/oname=$INSTDIR\readme.txt" "README"

  ;Icons
  file "/oname=$INSTDIR\strata.ico" "resources\images\application-icon.ico"
  file "/oname=$INSTDIR\strata-data.ico" "resources\images\file-data.ico"

  ;Main libraries
  file "C:\devel\fftw-3.3.4\libfftw3-3.dll"
  file "C:\devel\GnuWin32\bin\libgsl.dll"
  file "C:\devel\GnuWin32\bin\libgslcblas.dll"
  file "${QT_PATH}\bin\QtCore4.dll"
  file "${QT_PATH}\bin\QtOpenGL4.dll"
  file "${QT_PATH}\bin\QtGui4.dll"
  file "${QT_PATH}\bin\QtNetwork4.dll"
  file "${QT_PATH}\bin\QtScript4.dll"
  file "${QT_PATH}\bin\QtSvg4.dll"
  file "${QT_PATH}\bin\QtXml4.dll"
  file "${MINGW_PATH}\libstdc++-6.dll"
  file "${MINGW_PATH}\libgcc_s_dw2-1.dll"
  file "${MINGW_PATH}\libwinpthread-1.dll"
  ;file "${MINGW_PATH}\mingwm10.dll"
  file "C:\devel\qwt-6.1\lib\qwt.dll"
  
  ;Plugins for SVG icons
  SetOutPath "$INSTDIR\iconengines" 
  file "${QT_PATH}\plugins\iconengines\qsvgicon4.dll"
  SetOutPath "$INSTDIR\imageformats" 
  file "${QT_PATH}\plugins\imageformats\qsvg4.dll"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\Strata" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

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
	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Manual.lnk" "$INSTDIR\manual.pdf"

SectionEnd

Section "Examples" SecExamples
	
	SetOutPath "$INSTDIR\examples"

	;Files to install
	file "example\example-1-td.strata"
	file "example\example-2-td.strata"
	file "example\example-3-rvt.strata"
	file "example\suite-10-1.csv"
	file "example\response-spectrum.csv"
	file /r "example\*.AT2"

	CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Examples.lnk" "$INSTDIR\examples\"

SectionEnd

Section "File Associations" SecAssociation
  ;File associations
  !insertmacro APP_ASSOCIATE "strata" "strata.data" "Strata Data" "$INSTDIR\strata-data.ico" "Edit/Run with Strata" "$INSTDIR\strata.exe $\"%1$\""
SectionEnd

Section /o "Source" SecSource

	SetOutPath "$INSTDIR\src"

	;Files to install
	file "/oname=..\strata.pro" "strata.pro"
	file "src\*.h"
	file "src\*.cpp"

	; 13:07 <Dirm> I normally just do something like exec("cmd /c start http://www.google.com")
	; CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Source Documentation.lnk" "cmd.exe" "/c weee"
SectionEnd

;--------------------------------
;Descriptions
  ;Language strings
  LangString DESC_SecProgram ${LANG_ENGLISH} "Binaries required to run Strata."
  LangString DESC_SecDocumentation ${LANG_ENGLISH} "User manual."
  LangString DESC_SecExamples ${LANG_ENGLISH} "Example input files."
  LangString DESC_SecAssociation ${LANG_ENGLISH} "Associate Strata data files with Strata."
  LangString DESC_SecSource ${LANG_ENGLISH} "Source code."


  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecProgram} $(DESC_SecProgram)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDocumentation} $(DESC_SecDocumentation)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecExamples} $(DESC_SecExamples)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecAssociation} $(DESC_SecAssociation)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSource} $(DESC_SecSource)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  ;Remove the files
  Delete "$INSTDIR\strata.exe"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\manual.pdf"
  Delete "$INSTDIR\libfftw3-3.dll"
  Delete "$INSTDIR\libgsl.dll"
  Delete "$INSTDIR\libgslcblas.dll"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtNetwork4.dll"
  Delete "$INSTDIR\QtScript4.dll"
  Delete "$INSTDIR\QtSvg4.dll"
  Delete "$INSTDIR\QtXml4.dll"
  Delete "$INSTDIR\qwt.dll"
  Delete "$INSTDIR\iconengines\qsvgicon4.dll"
  RMDir  "$INSTDIR\iconengines"
  Delete "$INSTDIR\imageformats\qsvg4.dll"
  RMDir  "$INSTDIR\imageformats"
  Delete "$INSTDIR\examples\example-1-td.strata"
  Delete "$INSTDIR\examples\example-2-td.strata"
  Delete "$INSTDIR\examples\example-3-rvt.strata"
  Delete "$INSTDIR\examples\suite-10-1.csv"
  Delete "$INSTDIR\examples\response-spectrum.csv"
  RMDIR /r "$INSTDIR\examples\motions"
  RMDir  "$INSTDIR\examples"
  Delete "$INSTDIR\strata.pro"
  Delete "$INSTDIR\strata.ico"
  Delete "$INSTDIR\strata-data.ico"
  Delete "$INSTDIR\Uninstall.exe"

  ;Remove all source files
  RMDir /r "$INSTDIR\src"

  ;Remove the install directory
  RMDir $INSTDIR

  ;Remove the association with Strata
  !insertmacro APP_UNASSOCIATE "strata" "strata.datafile"

  ;Remove the Start menu items
  !insertmacro MUI_STARTMENU_GETFOLDER Strata $MUI_TEMP
   
  ;Delete the shortcuts
  Delete "$SMPROGRAMS\$MUI_TEMP\Strata.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Manual.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Examples.lnk"
  
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
