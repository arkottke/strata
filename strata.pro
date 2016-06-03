########################################################################
# Strata 
# Copyright (C) 2011-16   Albert R. Kottke
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GPL License, Version 3.0
########################################################################

TEMPLATE = app
TARGET = strata
QT += gui printsupport script widgets xml core
CONFIG += c++14

########################################################################
# Enable advanced options that should not be included in the version for
# standard users. If only the basic features are needed, comment out the
# following line.
########################################################################
DEFINES += ADVANCED_OPTIONS

########################################################################
# Use FFTW for the FFT alogorithm, otherwise GSL is used.
########################################################################
DEFINES += USE_FFTW
LIBS += -lfftw3
#LIBS += -lfftw3 -LC:/devel/fftw-3.3.4
#INCLUDEPATH += C:/devel/fftw-3.3.4

########################################################################
# Build type. For most cases this should be release, however during
# development of the software using the debug configuration can be
# beneficial.
########################################################################
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
# Flag based on if the program is compiled in debug mode. 
########################################################################
CONFIG(debug, debug|release) {
   DEFINES += DEBUG
}

########################################################################
# Libraries for linking
########################################################################
LIBS += -lgsl -lgslcblas

# Qwt requires debugging libraries
CONFIG(debug, debug|release) {
    LIBS += -lqwtd
} else {
    LIBS += -lqwt
}

# win32 { 
#     # INCLUDEPATH += /mingw64/include/qwt
#     # See: http://doc.qt.io/qt-5/qmake-platform-notes.html
#     # RC_FILE = strata.rc
# }

########################################################################
# Add all of the headers and sources
########################################################################
HEADERS += source/AbstractCalculator.h \
    source/AbstractDistribution.h \
    source/AbstractIterativeCalculator.h \
    source/AbstractLocationOutput.h \
    source/AbstractMotion.h \
    source/AbstractMutableOutputCatalog.h \
    source/AbstractNonlinearPropertyFactory.h \
    source/AbstractNonlinearPropertyStandardDeviation.h \
    source/AbstractOutput.h \
    source/AbstractOutputCatalog.h \
    source/AbstractOutputInterpolater.h \
    source/AbstractPage.h \
    source/AbstractProfileOutput.h \
    source/AbstractRatioOutput.h \
    source/AbstractRvtMotion.h \
    source/AbstractSteppedProfileOutput.h \
    source/AbstractTimeSeriesOutput.h \
    source/AccelTimeSeriesOutput.h \
    source/AccelTransferFunctionOutput.h \
    source/Algorithms.h \
    source/AriasIntensityProfileOutput.h\
    source/BedrockDepthVariation.h \
    source/CompatibleRvtMotion.h \
    source/CompatibleRvtMotionDialog.h \
    source/ComputePage.h \
    source/ConfigurePlotDialog.h \
    source/ConfiningStressDialog.h \
    source/ConfiningStressTableModel.h \
    source/CrustalAmplification.h \
    source/CrustalModel.h \
    source/CustomNonlinearProperty.h \
    source/DampingFactory.h \
    source/DampingProfileOutput.h \
    source/DampingStandardDeviation.h \
    source/DarendeliNonlinearProperty.h \
    source/DepthComboBox.h \
    source/DepthComboBoxDelegate.h \
    source/Dimension.h \
    source/DimensionLayout.h \
    source/DispTimeSeriesOutput.h \
    source/DissipatedEnergyProfileOutput.h \ 
    source/Distribution.h \
    source/EditActions.h \
    source/EquivalentLinearCalculator.h \
    source/EquivalentLinearCalculatorWidget.h \
    source/FinalVelProfileOutput.h \
    source/FourierSpectrumOutput.h \
    source/FrequencyDependentCalculator.h \
    source/FrequencyDependentCalculatorWidget.h \
    source/GeneralPage.h \
    source/HelpDialog.h \
    source/InitialVelProfileOutput.h \
    source/LayerThicknessVariation.h \
    source/LinearElasticCalculator.h \
    source/LinearOutputInterpolater.h \
    source/Location.h \
    source/MainWindow.h \
    source/MaxAccelProfileOutput.h \
    source/MaxErrorProfileOutput.h \
    source/MaxStrainProfileOutput.h \
    source/MaxStressProfileOutput.h \
    source/MaxVelProfileOutput.h \
    source/MethodGroupBox.h \
    source/ModulusFactory.h \
    source/ModulusProfileOutput.h \
    source/ModulusStandardDeviation.h \
    source/MotionLibrary.h \
    source/MotionPage.h \
    source/MotionTypeDelegate.h \
    source/MyAbstractTableModel.h \
    source/MyPlot.h \
    source/MyRandomNumGenerator.h \ 
    source/MyTableView.h \
    source/NonlinearProperty.h \
    source/NonlinearPropertyCatalog.h \
    source/NonlinearPropertyCatalogDialog.h \
    source/NonlinearPropertyDelegate.h \
    source/NonlinearPropertyFactoryGroupBox.h \
    source/NonlinearPropertyOutput.h \
    source/NonlinearPropertyRandomizer.h \
    source/NonlinearPropertyStandardDeviationWidget.h \
    source/OutputCatalog.h \
    source/OutputExportDialog.h \
    source/OutputPage.h \
    source/OutputStatistics.h \
    source/OutputTableFrame.h \
    source/ProfileRandomizer.h \
    source/ProfilesOutputCatalog.h \
    source/RatiosOutputCatalog.h \
    source/ResponseSpectrum.h \
    source/ResponseSpectrumOutput.h \
    source/ResultsPage.h \
    source/RockLayer.h \
    source/RvtMotion.h \
    source/RvtMotionDialog.h \
    source/SiteResponseModel.h \
    source/SoilLayer.h \
    source/SoilProfile.h \
    source/SoilProfilePage.h \
    source/SoilType.h \
    source/SoilTypeCatalog.h \
    source/SoilTypeDelegate.h \
    source/SoilTypeOutput.h \
    source/SoilTypeOutputTableModel.h \
    source/SoilTypePage.h \
    source/SoilTypesOutputCatalog.h \
    source/SourceTheoryRvtMotion.h \
    source/SourceTheoryRvtMotionDialog.h \
    source/SpectralRatioOutput.h \
    source/SpectraOutputCatalog.h \
    source/SteppedOutputInterpolater.h \
    source/StrainTimeSeriesOutput.h \
    source/StrainTransferFunctionOutput.h \
    source/StressRatioProfileOutput.h \
    source/StressReducCoeffProfileOutput.h \
    source/StressTimeSeriesOutput.h \
    source/SubLayer.h \
    source/TableGroupBox.h \
    source/TextLog.h \
    source/TimeSeriesMotion.h \
    source/TimeSeriesMotionDialog.h \
    source/TimeSeriesOutputCatalog.h \
    source/Units.h \
    source/VelocityLayer.h \
    source/VelocityVariation.h \
    source/VelTimeSeriesOutput.h \
    source/VerticalEffectiveStressProfileOutput.h \
    source/VerticalTotalStressProfileOutput.h \
    source/ViscoElasticStressTimeSeriesOutput.h \
    source/MaxDispProfileOutput.h \
    source/MyQwtCompatibility.h \
    source/OnlyIncreasingDelegate.h \
    source/BatchRunner.h

SOURCES +=     source/AbstractCalculator.cpp \
    source/AbstractDistribution.cpp \
    source/AbstractIterativeCalculator.cpp \
    source/AbstractLocationOutput.cpp \
    source/AbstractMotion.cpp \
    source/AbstractMutableOutputCatalog.cpp \
    source/AbstractNonlinearPropertyFactory.cpp \
    source/AbstractNonlinearPropertyStandardDeviation.cpp \
    source/AbstractOutput.cpp \
    source/AbstractOutputCatalog.cpp \
    source/AbstractOutputInterpolater.cpp \
    source/AbstractPage.cpp \
    source/AbstractProfileOutput.cpp \
    source/AbstractRatioOutput.cpp \
    source/AbstractRvtMotion.cpp \
    source/AbstractSteppedProfileOutput.cpp \
    source/AbstractTimeSeriesOutput.cpp \
    source/AccelTimeSeriesOutput.cpp \
    source/AccelTransferFunctionOutput.cpp \
    source/Algorithms.cpp \
    source/AriasIntensityProfileOutput.cpp\
    source/BedrockDepthVariation.cpp \
    source/CompatibleRvtMotion.cpp \
    source/CompatibleRvtMotionDialog.cpp \
    source/ComputePage.cpp \
    source/ConfigurePlotDialog.cpp \
    source/ConfiningStressDialog.cpp \
    source/ConfiningStressTableModel.cpp \
    source/CrustalAmplification.cpp \
    source/CrustalModel.cpp \
    source/CustomNonlinearProperty.cpp \
    source/DampingFactory.cpp \
    source/DampingProfileOutput.cpp \
    source/DampingStandardDeviation.cpp \
    source/DarendeliNonlinearProperty.cpp \
    source/DepthComboBox.cpp \
    source/DepthComboBoxDelegate.cpp \
    source/Dimension.cpp \
    source/DimensionLayout.cpp \
    source/DispTimeSeriesOutput.cpp \
    source/DissipatedEnergyProfileOutput.cpp \
    source/Distribution.cpp \
    source/EditActions.cpp \
    source/EquivalentLinearCalculator.cpp \
    source/EquivalentLinearCalculatorWidget.cpp \
    source/FinalVelProfileOutput.cpp \
    source/FourierSpectrumOutput.cpp \
    source/FrequencyDependentCalculator.cpp \
    source/FrequencyDependentCalculatorWidget.cpp \
    source/GeneralPage.cpp \
    source/HelpDialog.cpp \
    source/InitialVelProfileOutput.cpp \
    source/LayerThicknessVariation.cpp \
    source/LinearElasticCalculator.cpp \
    source/LinearOutputInterpolater.cpp \
    source/Location.cpp \
    source/main.cpp \
    source/MainWindow.cpp \
    source/MaxAccelProfileOutput.cpp \
    source/MaxErrorProfileOutput.cpp \
    source/MaxStrainProfileOutput.cpp \
    source/MaxStressProfileOutput.cpp \
    source/MaxVelProfileOutput.cpp \
    source/MethodGroupBox.cpp \
    source/ModulusFactory.cpp \
    source/ModulusProfileOutput.cpp \
    source/ModulusStandardDeviation.cpp \
    source/MotionLibrary.cpp \
    source/MotionPage.cpp \
    source/MotionTypeDelegate.cpp \
    source/MyAbstractTableModel.cpp \
    source/MyPlot.cpp \
    source/MyRandomNumGenerator.cpp \ 
    source/MyTableView.cpp \
    source/NonlinearProperty.cpp \
    source/NonlinearPropertyCatalog.cpp \
    source/NonlinearPropertyCatalogDialog.cpp \
    source/NonlinearPropertyDelegate.cpp \
    source/NonlinearPropertyFactoryGroupBox.cpp \
    source/NonlinearPropertyOutput.cpp \
    source/NonlinearPropertyRandomizer.cpp \
    source/NonlinearPropertyStandardDeviationWidget.cpp \
    source/OutputCatalog.cpp \
    source/OutputExportDialog.cpp \
    source/OutputPage.cpp \
    source/OutputStatistics.cpp \
    source/OutputTableFrame.cpp \
    source/ProfileRandomizer.cpp \
    source/ProfilesOutputCatalog.cpp \
    source/RatiosOutputCatalog.cpp \
    source/ResponseSpectrum.cpp \
    source/ResponseSpectrumOutput.cpp \
    source/ResultsPage.cpp \
    source/RockLayer.cpp \
    source/RvtMotion.cpp \
    source/RvtMotionDialog.cpp \
    source/SiteResponseModel.cpp \
    source/SoilLayer.cpp \
    source/SoilProfile.cpp \
    source/SoilProfilePage.cpp \
    source/SoilType.cpp \
    source/SoilTypeCatalog.cpp \
    source/SoilTypeDelegate.cpp \
    source/SoilTypeOutput.cpp \
    source/SoilTypeOutputTableModel.cpp \
    source/SoilTypePage.cpp \
    source/SoilTypesOutputCatalog.cpp \
    source/SourceTheoryRvtMotion.cpp \
    source/SourceTheoryRvtMotionDialog.cpp \
    source/SpectralRatioOutput.cpp \
    source/SpectraOutputCatalog.cpp \
    source/SteppedOutputInterpolater.cpp \
    source/StrainTimeSeriesOutput.cpp \
    source/StrainTransferFunctionOutput.cpp \
    source/StressRatioProfileOutput.cpp \
    source/StressReducCoeffProfileOutput.cpp \
    source/StressTimeSeriesOutput.cpp \
    source/SubLayer.cpp \
    source/TableGroupBox.cpp \
    source/TextLog.cpp \
    source/TimeSeriesMotion.cpp \
    source/TimeSeriesMotionDialog.cpp \
    source/TimeSeriesOutputCatalog.cpp \
    source/Units.cpp \
    source/VelocityLayer.cpp \
    source/VelocityVariation.cpp \
    source/VelTimeSeriesOutput.cpp \
    source/VerticalEffectiveStressProfileOutput.cpp \
    source/VerticalTotalStressProfileOutput.cpp \
    source/ViscoElasticStressTimeSeriesOutput.cpp \
    source/MaxDispProfileOutput.cpp \
    source/MyQwtCompatibility.cpp \
    source/OnlyIncreasingDelegate.cpp \
    source/BatchRunner.cpp

RESOURCES += resources/resources.qrc
