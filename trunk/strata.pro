########################################################################
# Strata 
# Copyright (C) 2011   Albert R. Kottke
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GPL License, Version 3.0
########################################################################

# Embed the manifest
CONFIG += embed_manifest_exe

# Grab the revision number using svnversion and clean it up.
DEFINES += REVISION=$$system(python getSvnVersion.py)

# Flag based on if the program is compiled in debug mode. 
CONFIG(debug, debug|release) {
   DEFINES += DEBUG
}

# Directories for building
CONFIG(debug, debug|release) {
   DESTDIR = debug
} else {
   DESTDIR = release
}

# add console output
CONFIG += console

TEMPLATE = app
TARGET = strata
QT += script xml network

########################################################################
# Enable advanced options that should not be included in the version for
# standard users. If only the basic features are needed, comment out the
# following line.
########################################################################
DEFINES += ADVANCED_OPTIONS

########################################################################
# Build type. For most cases this should be release, however during
# development of the software using the debug configuration can be
# beneficial.
########################################################################
#CONFIG += debug
#CONFIG += release
CONFIG += debug_and_release

########################################################################
# Compiler warning messages.
########################################################################
CONFIG += warn_on

########################################################################
# Enable console for debug versions
########################################################################
CONFIG(debug, debug|release) {
    CONFIG += console
}

########################################################################
# Setup of for the various libraries. This is most important on Windows.
########################################################################
unix {
    LIBS += -lm \
        -lfftw3 \
        -lgsl \
        -lgslcblas \
        -lboost_system \
        -lboost_serialization \
        -L$$PWD/../qwt-6.1/lib \
        -lqwt
    INCLUDEPATH += $$PWD/../qwt-6.1/src

}
win32 { 
  LIBS += -lm \
        -lfftw3-3 \
        -lgsl \
        -lgslcblas \
        -LC:/devel/fftw-3.3.4 \
        -LC:/devel/GnuWin32/bin \
        -LC:/devel/qwt-6.1/lib \
		-LC:/devel/boost_1_57_0/stage/lib \
        -L"C:/Program Files/mingw-w64/i686-4.8.2-posix-dwarf-rt_v3-rev4/bin"
    INCLUDEPATH += . \
        C:/devel/fftw-3.3.4 \
        C:/devel/qwt-6.1/src \
        C:/devel/GnuWin32/include \
        C:/devel/boost_1_57_0
    RC_FILE = strata.rc
    CONFIG(debug, debug|release ) {
        LIBS += -lqwtd \
                -lboost_serialization-mgw48-mt-d-1_57  \
                -lboost_system-mgw48-mt-d-1_57
    } else {
        LIBS += -lqwt \
                -lboost_serialization-mgw48-mt-1_57  \
                -lboost_system-mgw48-mt-1_57
    }
}

########################################################################
# Add all of the headers and sources
########################################################################
HEADERS += src/AbstractCalculator.h \
    src/AbstractDistribution.h \
    src/AbstractIterativeCalculator.h \
    src/AbstractLocationOutput.h \
    src/AbstractMotion.h \
    src/AbstractMutableOutputCatalog.h \
    src/AbstractNonlinearPropertyFactory.h \
    src/AbstractNonlinearPropertyStandardDeviation.h \
    src/AbstractOutput.h \
    src/AbstractOutputCatalog.h \
    src/AbstractOutputInterpolater.h \
    src/AbstractPage.h \
    src/AbstractProfileOutput.h \
    src/AbstractRatioOutput.h \
    src/AbstractRvtMotion.h \
    src/AbstractSteppedProfileOutput.h \
    src/AbstractTimeSeriesOutput.h \
    src/AccelTimeSeriesOutput.h \
    src/AccelTransferFunctionOutput.h \
    src/Algorithms.h \
    src/AriasIntensityProfileOutput.h\
    src/BedrockDepthVariation.h \
    src/CompatibleRvtMotion.h \
    src/CompatibleRvtMotionDialog.h \
    src/ComputePage.h \
    src/ConfigurePlotDialog.h \
    src/ConfiningStressDialog.h \
    src/ConfiningStressTableModel.h \
    src/CrustalAmplification.h \
    src/CrustalModel.h \
    src/CustomNonlinearProperty.h \
    src/DampingFactory.h \
    src/DampingProfileOutput.h \
    src/DampingStandardDeviation.h \
    src/DarendeliNonlinearProperty.h \
    src/DepthComboBox.h \
    src/DepthComboBoxDelegate.h \
    src/Dimension.h \
    src/DimensionLayout.h \
    src/DispTimeSeriesOutput.h \
    src/DissipatedEnergyProfileOutput.h \ 
    src/Distribution.h \
    src/EditActions.h \
    src/EquivalentLinearCalculator.h \
    src/EquivalentLinearCalculatorWidget.h \
    src/FinalVelProfileOutput.h \
    src/FourierSpectrumOutput.h \
    src/FrequencyDependentCalculator.h \
    src/FrequencyDependentCalculatorWidget.h \
    src/GeneralPage.h \
    src/HelpDialog.h \
    src/InitialVelProfileOutput.h \
    src/LayerThicknessVariation.h \
    src/LinearElasticCalculator.h \
    src/LinearOutputInterpolater.h \
    src/Location.h \
    src/MainWindow.h \
    src/MaxAccelProfileOutput.h \
    src/MaxErrorProfileOutput.h \
    src/MaxStrainProfileOutput.h \
    src/MaxStressProfileOutput.h \
    src/MaxVelProfileOutput.h \
    src/MethodGroupBox.h \
    src/ModulusFactory.h \
    src/ModulusProfileOutput.h \
    src/ModulusStandardDeviation.h \
    src/MotionLibrary.h \
    src/MotionPage.h \
    src/MotionTypeDelegate.h \
    src/MyAbstractTableModel.h \
    src/MyPlot.h \
    src/MyRandomNumGenerator.h \ 
    src/MyTableView.h \
    src/NonlinearProperty.h \
    src/NonlinearPropertyCatalog.h \
    src/NonlinearPropertyCatalogDialog.h \
    src/NonlinearPropertyDelegate.h \
    src/NonlinearPropertyFactoryGroupBox.h \
    src/NonlinearPropertyOutput.h \
    src/NonlinearPropertyRandomizer.h \
    src/NonlinearPropertyStandardDeviationWidget.h \
    src/OutputCatalog.h \
    src/OutputExportDialog.h \
    src/OutputPage.h \
    src/OutputStatistics.h \
    src/OutputTableFrame.h \
    src/ProfileRandomizer.h \
    src/ProfilesOutputCatalog.h \
    src/RatiosOutputCatalog.h \
    src/ResponseSpectrum.h \
    src/ResponseSpectrumOutput.h \
    src/ResultsPage.h \
    src/RockLayer.h \
    src/RvtMotion.h \
    src/RvtMotionDialog.h \
    src/Serializer.h \
    src/SeriesSmoother.h \
    src/SiteResponseModel.h \
    src/SoilLayer.h \
    src/SoilProfile.h \
    src/SoilProfilePage.h \
    src/SoilType.h \
    src/SoilTypeCatalog.h \
    src/SoilTypeDelegate.h \
    src/SoilTypeOutput.h \
    src/SoilTypeOutputTableModel.h \
    src/SoilTypePage.h \
    src/SoilTypesOutputCatalog.h \
    src/SourceTheoryRvtMotion.h \
    src/SourceTheoryRvtMotionDialog.h \
    src/SpectralRatioOutput.h \
    src/SpectraOutputCatalog.h \
    src/SteppedOutputInterpolater.h \
    src/StrainTimeSeriesOutput.h \
    src/StrainTransferFunctionOutput.h \
    src/StressRatioProfileOutput.h \
    src/StressReducCoeffProfileOutput.h \
    src/StressTimeSeriesOutput.h \
    src/SubLayer.h \
    src/TableGroupBox.h \
    src/TextLog.h \
    src/TimeSeriesMotion.h \
    src/TimeSeriesMotionDialog.h \
    src/TimeSeriesOutputCatalog.h \
    src/Units.h \
    src/UpdateDialog.h \
    src/VelocityLayer.h \
    src/VelocityVariation.h \
    src/VelTimeSeriesOutput.h \
    src/VerticalEffectiveStressProfileOutput.h \
    src/VerticalTotalStressProfileOutput.h \
    src/ViscoElasticStressTimeSeriesOutput.h \
    src/MaxDispProfileOutput.h \
    src/MyQwtCompatibility.h \
    src/OnlyIncreasingDelegate.h \
    src/BatchRunner.h

SOURCES +=     src/AbstractCalculator.cpp \
    src/AbstractDistribution.cpp \
    src/AbstractIterativeCalculator.cpp \
    src/AbstractLocationOutput.cpp \
    src/AbstractMotion.cpp \
    src/AbstractMutableOutputCatalog.cpp \
    src/AbstractNonlinearPropertyFactory.cpp \
    src/AbstractNonlinearPropertyStandardDeviation.cpp \
    src/AbstractOutput.cpp \
    src/AbstractOutputCatalog.cpp \
    src/AbstractOutputInterpolater.cpp \
    src/AbstractPage.cpp \
    src/AbstractProfileOutput.cpp \
    src/AbstractRatioOutput.cpp \
    src/AbstractRvtMotion.cpp \
    src/AbstractSteppedProfileOutput.cpp \
    src/AbstractTimeSeriesOutput.cpp \
    src/AccelTimeSeriesOutput.cpp \
    src/AccelTransferFunctionOutput.cpp \
    src/Algorithms.cpp \
    src/AriasIntensityProfileOutput.cpp\
    src/BedrockDepthVariation.cpp \
    src/CompatibleRvtMotion.cpp \
    src/CompatibleRvtMotionDialog.cpp \
    src/ComputePage.cpp \
    src/ConfigurePlotDialog.cpp \
    src/ConfiningStressDialog.cpp \
    src/ConfiningStressTableModel.cpp \
    src/CrustalAmplification.cpp \
    src/CrustalModel.cpp \
    src/CustomNonlinearProperty.cpp \
    src/DampingFactory.cpp \
    src/DampingProfileOutput.cpp \
    src/DampingStandardDeviation.cpp \
    src/DarendeliNonlinearProperty.cpp \
    src/DepthComboBox.cpp \
    src/DepthComboBoxDelegate.cpp \
    src/Dimension.cpp \
    src/DimensionLayout.cpp \
    src/DispTimeSeriesOutput.cpp \
    src/DissipatedEnergyProfileOutput.cpp \
    src/Distribution.cpp \
    src/EditActions.cpp \
    src/EquivalentLinearCalculator.cpp \
    src/EquivalentLinearCalculatorWidget.cpp \
    src/FinalVelProfileOutput.cpp \
    src/FourierSpectrumOutput.cpp \
    src/FrequencyDependentCalculator.cpp \
    src/FrequencyDependentCalculatorWidget.cpp \
    src/GeneralPage.cpp \
    src/HelpDialog.cpp \
    src/InitialVelProfileOutput.cpp \
    src/LayerThicknessVariation.cpp \
    src/LinearElasticCalculator.cpp \
    src/LinearOutputInterpolater.cpp \
    src/Location.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/MaxAccelProfileOutput.cpp \
    src/MaxErrorProfileOutput.cpp \
    src/MaxStrainProfileOutput.cpp \
    src/MaxStressProfileOutput.cpp \
    src/MaxVelProfileOutput.cpp \
    src/MethodGroupBox.cpp \
    src/ModulusFactory.cpp \
    src/ModulusProfileOutput.cpp \
    src/ModulusStandardDeviation.cpp \
    src/MotionLibrary.cpp \
    src/MotionPage.cpp \
    src/MotionTypeDelegate.cpp \
    src/MyAbstractTableModel.cpp \
    src/MyPlot.cpp \
    src/MyRandomNumGenerator.cpp \ 
    src/MyTableView.cpp \
    src/NonlinearProperty.cpp \
    src/NonlinearPropertyCatalog.cpp \
    src/NonlinearPropertyCatalogDialog.cpp \
    src/NonlinearPropertyDelegate.cpp \
    src/NonlinearPropertyFactoryGroupBox.cpp \
    src/NonlinearPropertyOutput.cpp \
    src/NonlinearPropertyRandomizer.cpp \
    src/NonlinearPropertyStandardDeviationWidget.cpp \
    src/OutputCatalog.cpp \
    src/OutputExportDialog.cpp \
    src/OutputPage.cpp \
    src/OutputStatistics.cpp \
    src/OutputTableFrame.cpp \
    src/ProfileRandomizer.cpp \
    src/ProfilesOutputCatalog.cpp \
    src/RatiosOutputCatalog.cpp \
    src/ResponseSpectrum.cpp \
    src/ResponseSpectrumOutput.cpp \
    src/ResultsPage.cpp \
    src/RockLayer.cpp \
    src/RvtMotion.cpp \
    src/RvtMotionDialog.cpp \
    src/Serializer.cpp \
    src/SeriesSmoother.cpp \
    src/SiteResponseModel.cpp \
    src/SoilLayer.cpp \
    src/SoilProfile.cpp \
    src/SoilProfilePage.cpp \
    src/SoilType.cpp \
    src/SoilTypeCatalog.cpp \
    src/SoilTypeDelegate.cpp \
    src/SoilTypeOutput.cpp \
    src/SoilTypeOutputTableModel.cpp \
    src/SoilTypePage.cpp \
    src/SoilTypesOutputCatalog.cpp \
    src/SourceTheoryRvtMotion.cpp \
    src/SourceTheoryRvtMotionDialog.cpp \
    src/SpectralRatioOutput.cpp \
    src/SpectraOutputCatalog.cpp \
    src/SteppedOutputInterpolater.cpp \
    src/StrainTimeSeriesOutput.cpp \
    src/StrainTransferFunctionOutput.cpp \
    src/StressRatioProfileOutput.cpp \
    src/StressReducCoeffProfileOutput.cpp \
    src/StressTimeSeriesOutput.cpp \
    src/SubLayer.cpp \
    src/TableGroupBox.cpp \
    src/TextLog.cpp \
    src/TimeSeriesMotion.cpp \
    src/TimeSeriesMotionDialog.cpp \
    src/TimeSeriesOutputCatalog.cpp \
    src/Units.cpp \
    src/UpdateDialog.cpp \
    src/VelocityLayer.cpp \
    src/VelocityVariation.cpp \
    src/VelTimeSeriesOutput.cpp \
    src/VerticalEffectiveStressProfileOutput.cpp \
    src/VerticalTotalStressProfileOutput.cpp \
    src/ViscoElasticStressTimeSeriesOutput.cpp \
    src/MaxDispProfileOutput.cpp \
    src/MyQwtCompatibility.cpp \
    src/OnlyIncreasingDelegate.cpp \
    src/BatchRunner.cpp

RESOURCES += resources/resources.qrc
