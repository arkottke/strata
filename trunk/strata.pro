TEMPLATE = app
CONFIG += debug_and_release \
    warn_on
QT += script \
    xml \
    network
unix { 
    DEFINES += REVISION=$$system("svnversion . | perl -pne's/(?:\d+:)?(\d+)(?:[MS]+)?$/\1/'")
    LIBS += -lm \
        -lfftw3 \
        -lgsl \
        -lgslcblas \
        -lqwt
    INCLUDEPATH += . \
        "/usr/include/qwt"
}
win32 { 
    # DEFINES += REVISION=$$system(cmd.exe /C "svnversion.exe . | perl.exe -pne"s/(?:\d+:)?(\d+)(?:[MS]+)?$/\1/"")
    DEFINES += REVISION=$$system(cmd.exe /C "SubWCRev.exe . | perl.exe -ne\"if (/^Last/){s/\D+//; print;}\"")
    LIBS += -lm \
        -lfftw3-3 \
        -L"C:\devel\fftw-3.2.2" \
        -lgsl \
        -lgslcblas \
        -L"C:\devel\GnuWin32\lib"
    INCLUDEPATH += . \
        "C:\devel\fftw-3.2.2" \
        "C:\devel\qwt-5.2\src" \
        "C:\devel\GnuWin32\include"
    RC_FILE = strata.rc
    CONFIG( debug, debug|release ) {
        LIBS += -lqwtd5 \
            -L"C:\devel\qwt-5.2\lib"
    } else {
        LIBS += -lqwt5 \
            -L"C:\devel\qwt-5.2\lib"
    }
}

HEADERS += src/Algorithms.h \
    src/ColumnTableModel.h \
    src/ComputePage.h \
    src/ConfigurePlotDialog.h \
    src/ConfiningStressDialog.h \
    src/ConfiningStressTableModel.h \
    src/CrustalAmpModel.h \
    src/CrustalVelModel.h \
    src/DepthComboBox.h \
    src/DepthComboBoxDelegate.h \
    src/Dimension.h \
    src/Distribution.h \
    src/EditActions.h \
    src/EquivLinearCalc.h \
    src/FourierSpectrumModel.h \
    src/GeneralPage.h \
    src/HelpDialog.h \
    src/IndexedDepth.h \
    src/ListGroupBox.h \
    src/Location.h \
    src/MainWindow.h \
    src/MinimumFinder.h \
    src/Motion.h \
    src/MotionPage.h \
    src/MyAbstractTableModel.h \
    src/MyGroupBox.h \
    src/MyProgressDialog.h \
    src/MyTableView.h \
    src/NonlinearProperty.h \
    src/NonlinearPropertyLibrary.h \
    src/NonlinearPropertyLibraryDialog.h \
    src/NonlinearPropertyLibraryTableModel.h \
    src/NonlinearPropertyListModel.h \
    src/NonlinearPropertyTableModel.h \
    src/Output.h \
    src/OutputExportDialog.h \
    src/OutputPage.h \
    src/OutputTableModel.h \
    src/OutputCsvSelectionTableModel.h \
    src/PointSourceModel.h \
    src/ProfileVariation.h \
    src/RatioLocation.h \
    src/RatioLocationTableModel.h \
    src/RecordedMotion.h \
    src/RecordedMotionDialog.h \
    src/RecordedMotionModel.h \
    src/ResponseLocation.h \
    src/ResponseLocationTableModel.h \
    src/ResponseSpectrum.h \
    src/ResultsPage.h \
    src/RockLayer.h \
    src/RvtMotion.h \
    src/Serializer.h \
    src/SeriesSmoother.h \
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
    src/NonlinearPropertyVariation.h \
    src/StringListDelegate.h \
    src/SubLayer.h \
    src/TableGroupBox.h \
    src/TextLog.h \
    src/Units.h \
    src/UpdateDialog.h \
    src/VelocityLayer.h
SOURCES += src/Algorithms.cpp \
    src/ColumnTableModel.cpp \
    src/ComputePage.cpp \
    src/ConfigurePlotDialog.cpp \
    src/ConfiningStressDialog.cpp \
    src/ConfiningStressTableModel.cpp \
    src/CrustalAmpModel.cpp \
    src/CrustalVelModel.cpp \
    src/DepthComboBox.cpp \
    src/DepthComboBoxDelegate.cpp \
    src/Dimension.cpp \
    src/Distribution.cpp \
    src/EditActions.cpp \
    src/EquivLinearCalc.cpp \
    src/FourierSpectrumModel.cpp \
    src/GeneralPage.cpp \
    src/HelpDialog.cpp \
    src/IndexedDepth.cpp \
    src/ListGroupBox.cpp \
    src/Location.cpp \
    src/MainWindow.cpp \
    src/MinimumFinder.cpp \
    src/Motion.cpp \
    src/MotionPage.cpp \
    src/MyAbstractTableModel.cpp \
    src/MyGroupBox.cpp \
    src/MyProgressDialog.cpp \
    src/MyTableView.cpp \
    src/NonlinearProperty.cpp \
    src/NonlinearPropertyLibrary.cpp \
    src/NonlinearPropertyLibraryDialog.cpp \
    src/NonlinearPropertyLibraryTableModel.cpp \
    src/NonlinearPropertyListModel.cpp \
    src/NonlinearPropertyTableModel.cpp \
    src/Output.cpp \
    src/OutputExportDialog.cpp \
    src/OutputPage.cpp \
    src/OutputCsvSelectionTableModel.cpp \
    src/OutputTableModel.cpp \
    src/PointSourceModel.cpp \
    src/ProfileVariation.cpp \
    src/RatioLocation.cpp \
    src/RatioLocationTableModel.cpp \
    src/RecordedMotion.cpp \
    src/RecordedMotionDialog.cpp \
    src/RecordedMotionModel.cpp \
    src/ResponseLocation.cpp \
    src/ResponseLocationTableModel.cpp \
    src/ResponseSpectrum.cpp \
    src/ResultsPage.cpp \
    src/RockLayer.cpp \
    src/RvtMotion.cpp \
    src/Serializer.cpp \
    src/SeriesSmoother.cpp \
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
    src/NonlinearPropertyVariation.cpp \
    src/StringListDelegate.cpp \
    src/SubLayer.cpp \
    src/TableGroupBox.cpp \
    src/TextLog.cpp \
    src/Units.cpp \
    src/UpdateDialog.cpp \
    src/VelocityLayer.cpp \
    src/main.cpp
RESOURCES += resources/resources.qrc
