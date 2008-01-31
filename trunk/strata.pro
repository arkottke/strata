TEMPLATE = app
CONFIG += release warn_on console
DESTDIR += bin
QT += script

unix {
	DEFINES += REVISION=$$system("svnversion . | perl -pne's/(?:\d+:)?(\d+)(?:[MS]+)?$/\1/'")
	LIBS += -lm -lfftw3 -lgsl -lqwt -lgslcblas
	INCLUDEPATH += . "/usr/lib/include/"
}

win32 {
	DEFINES += REVISION=$$system(cmd.exe /C "svnversion.exe . | perl.exe -pne"s/(?:\d+:)?(\d+)(?:[MS]+)?$/\1/"")
	LIBS += -lm  -lfftw3-3 -L"C:\devel\fftw-3.1.2" -lqwt5 -L"C:\devel\qwt-5.0\lib" -lgsl -L"C:\devel\GnuWin32\lib"
	INCLUDEPATH += . "C:\devel\fftw-3.1.2" "C:\devel\qwt-5.0\src" "C:\devel\GnuWin32\include"
	RC_FILE = strata.rc
}

macx {
	DEFINES += REVISION=$$system("svnversion . | perl -pne's/(?:\d+:)?(\d+)(?:[MS]+)?$/\1/'")
	LIBS += -lm -lfftw3 -lqwt -L"/usr/local/qwt-5.0.2/lib" -lgsl
	INCLUDEPATH += . "/usr/local/qwt-5.0.2/include"
	ICON = resources/images/application-icon.ico
}

INCLUDEPATH += src

MOC_DIR += build
RCC_DIR += build
OBJECTS_DIR += build
UI_DIR +=   build

HEADERS += \
    src/Algorithms.cpp \
    src/ComputePage.h\
    src/ConfigurePlotDialog.h\
    src/DepthComboBox.h \
    src/DepthComboBoxDelegate.h \
    src/Dimension.h \
    src/Distribution.h \
    src/EquivLinearCalc.h \
    src/FourierSpectrumModel.h \
    src/GeneralPage.h \
    src/HelpDialog.h \
    src/IndexedDepth.h \
    src/InputWidget.h \
    src/ListGroupBox.h \
    src/Location.h \
    src/MainWindow.h \
    src/Motion.h \
    src/MotionPage.h \
    src/MyGroupBox.h \ 
    src/MyProgressDialog.h \
    src/MyTableView.h \
    src/NonLinearProperty.h \
    src/NonLinearPropertyLibrary.h \
    src/NonLinearPropertyLibraryDialog.h \
    src/NonLinearPropertyLibraryTableModel.h \
    src/NonLinearPropertyListModel.h \
    src/NonLinearPropertyTableModel.h \
    src/Output.h \
    src/OutputExportDialog.h \
    src/OutputPage.h \
    src/OutputTableModel.h \
    src/OutputCsvSelectionTableModel.h \
    src/OutputWidget.h \
    src/ProfileVariation.h \
    src/RatioLocation.h \
    src/RatioLocationTableModel.h \
    src/RecordedMotion.h \
    src/RecordedMotionDialog.h \
    src/RecordedMotionModel.h \
    src/ResponseLocation.h \ 
    src/ResponseLocationTableModel.h \ 
    src/ResponseSpectrum.h \
    src/ResponseSpectrumModel.h \
    src/RockLayer.h \
    src/RvtMotion.h \
    src/Serializer.h \
    src/SiteProfile.h \
    src/SiteResponseModel.h \
    src/SiteResponseOutput.h \
    src/SoilLayer.h \
    src/SoilProfilePage.h \
    src/SoilProfileModel.h \
    src/SoilTypeTableModel.h \
    src/SoilType.h \
    src/SoilTypePage.h \
    src/SoilTypeOutputTableModel.h \
    src/NonLinearPropertyVariation.h \
    src/StringListDelegate.h \
    src/SubLayer.h \
    src/TableGroupBox.h \
    src/TextLog.h \
    src/Units.h \
    src/VelocityLayer.h
SOURCES +=   \
    src/Algorithms.cpp \
    src/ComputePage.cpp \
    src/ConfigurePlotDialog.cpp\
    src/DepthComboBox.cpp \
    src/DepthComboBoxDelegate.cpp \
    src/Dimension.cpp \
    src/Distribution.cpp \
    src/EquivLinearCalc.cpp \
    src/FourierSpectrumModel.cpp \
    src/GeneralPage.cpp \
    src/HelpDialog.cpp \
    src/IndexedDepth.cpp \
    src/InputWidget.cpp \
	src/ListGroupBox.cpp \
    src/Location.cpp \
    src/MainWindow.cpp \
    src/main.cpp \
    src/Motion.cpp \
    src/MotionPage.cpp \
    src/MyGroupBox.cpp \ 
    src/MyProgressDialog.cpp \
    src/MyTableView.cpp \
    src/NonLinearProperty.cpp \
    src/NonLinearPropertyLibrary.cpp \
    src/NonLinearPropertyLibraryDialog.cpp \
    src/NonLinearPropertyLibraryTableModel.cpp \
    src/NonLinearPropertyListModel.cpp \
    src/NonLinearPropertyTableModel.cpp \
    src/Output.cpp \
    src/OutputExportDialog.cpp \
    src/OutputPage.cpp \
    src/OutputCsvSelectionTableModel.cpp \
    src/OutputTableModel.cpp \
    src/OutputWidget.cpp \
    src/ProfileVariation.cpp \
    src/RatioLocation.cpp \
    src/RatioLocationTableModel.cpp \
    src/RecordedMotion.cpp \
    src/RecordedMotionDialog.cpp \
    src/RecordedMotionModel.cpp \
    src/ResponseLocation.cpp \ 
    src/ResponseLocationTableModel.cpp \ 
    src/ResponseSpectrum.cpp \
    src/ResponseSpectrumModel.cpp \
    src/RockLayer.cpp \
    src/RvtMotion.cpp \
    src/Serializer.cpp \
    src/SiteProfile.cpp \
    src/SiteResponseModel.cpp \
    src/SiteResponseOutput.cpp \
    src/SoilLayer.cpp \
    src/SoilProfilePage.cpp \
    src/SoilProfileModel.cpp \
    src/SoilType.cpp \
    src/SoilTypePage.cpp \
    src/SoilTypeTableModel.cpp \
    src/SoilTypeOutputTableModel.cpp \
    src/NonLinearPropertyVariation.cpp \
    src/StringListDelegate.cpp \
    src/SubLayer.cpp \
    src/TableGroupBox.cpp \
    src/TextLog.cpp \
    src/Units.cpp \
    src/VelocityLayer.cpp
RESOURCES +=  resources/resources.qrc
