////////////////////////////////////////////////////////////////////////////////
//
// This file is part of Strata.
//
// Strata is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Strata is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// Strata.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2010-2018 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "SiteResponseModel.h"

#include "AbstractCalculator.h"
#include "AbstractDistribution.h"
#include "AbstractIterativeCalculator.h"
#include "Algorithms.h"
#include "BedrockDepthVariation.h"
#include "CompatibleRvtMotion.h"
#include "Dimension.h"
#include "EquivalentLinearCalculator.h"
#include "FrequencyDependentCalculator.h"
#include "LayerThicknessVariation.h"
#include "LinearElasticCalculator.h"
#include "MotionLibrary.h"
#include "MyRandomNumGenerator.h"
#include "NonlinearProperty.h"
#include "NonlinearPropertyRandomizer.h"
#include "NonlinearPropertyUncertainty.h"
#include "OutputCatalog.h"
#include "ProfileRandomizer.h"
#include "ProfilesOutputCatalog.h"
#include "ResponseSpectrum.h"
#include "RockLayer.h"
#include "RvtMotion.h"
#include "SoilLayer.h"
#include "SoilProfile.h"
#include "SoilType.h"
#include "SoilTypeCatalog.h"
#include "SoilTypesOutputCatalog.h"
#include "SourceTheoryRvtMotion.h"
#include "TextLog.h"
#include "TimeSeriesMotion.h"
#include "Units.h"
#include "VelocityLayer.h"
#include "VelocityVariation.h"

#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QMetaProperty>
#include <QProgressBar>
#include <QTextDocument>
#include <QTimer>

#include <QDebug>

#include <cmath>

namespace {
auto validNumber(double value) -> bool { return std::isfinite(value); }

void addError(QStringList &errors, const QString &message) {
  errors.append(message);
}

void validateDimension(QStringList &errors, const QString &name,
                       Dimension *dimension, double minimum, double maximum) {
  if (!validNumber(dimension->min()) || !validNumber(dimension->max()) ||
      dimension->min() < minimum || dimension->max() > maximum ||
      dimension->min() >= dimension->max()) {
    addError(errors, QObject::tr("%1 must have increasing values between %2 "
                                 "and %3.")
                         .arg(name)
                         .arg(minimum)
                         .arg(maximum));
  }
  if (dimension->size() < 2 || dimension->size() > 16384) {
    addError(errors,
             QObject::tr("%1 must contain 2 to 16384 points.").arg(name));
  }
}

void validateDistribution(QStringList &errors, const QString &name,
                          AbstractDistribution *distribution, bool positive) {
  if (!validNumber(distribution->avg()) ||
      (positive && distribution->avg() <= 0.0)) {
    addError(
        errors,
        QObject::tr("%1 average must be a finite positive value.").arg(name));
  }
  if (distribution->type() < AbstractDistribution::Uniform ||
      distribution->type() > AbstractDistribution::LogNormal) {
    addError(errors,
             QObject::tr("%1 has an invalid distribution type.").arg(name));
  }
  if (distribution->type() != AbstractDistribution::Uniform &&
      (!validNumber(distribution->stdev()) || distribution->stdev() < 0.0)) {
    addError(
        errors,
        QObject::tr("%1 standard deviation must be finite and nonnegative.")
            .arg(name));
  }
  if (distribution->hasMin() && (!validNumber(distribution->min()) ||
                                 (positive && distribution->min() <= 0.0))) {
    addError(
        errors,
        QObject::tr("%1 minimum must be a finite positive value.").arg(name));
  }
  if (distribution->hasMax() && (!validNumber(distribution->max()) ||
                                 (positive && distribution->max() <= 0.0))) {
    addError(
        errors,
        QObject::tr("%1 maximum must be a finite positive value.").arg(name));
  }
  if (distribution->hasMin() && distribution->hasMax() &&
      distribution->min() >= distribution->max()) {
    addError(
        errors,
        QObject::tr("%1 minimum must be less than its maximum.").arg(name));
  }
}

void validateNonlinearProperty(QStringList &errors, const QString &name,
                               NonlinearProperty *property) {
  if (!property || property->strain().size() < 2 ||
      property->strain().size() != property->average().size()) {
    addError(errors, QObject::tr("%1 must define at least two strain-property "
                                 "pairs.")
                         .arg(name));
    return;
  }

  for (int i = 0; i < property->strain().size(); ++i) {
    if (!validNumber(property->strain().at(i)) ||
        property->strain().at(i) <= 0.0 ||
        !validNumber(property->average().at(i))) {
      addError(errors, QObject::tr("%1 contains a non-finite or invalid value "
                                   "at row %2.")
                           .arg(name)
                           .arg(i + 1));
      return;
    }
    if (i > 0 && property->strain().at(i - 1) >= property->strain().at(i)) {
      addError(errors,
               QObject::tr("%1 strain values must be strictly increasing.")
                   .arg(name));
      return;
    }
  }
}

void validateUncertainty(QStringList &errors, const QString &name,
                         NonlinearPropertyUncertainty *uncertainty) {
  if (!validNumber(uncertainty->min()) || !validNumber(uncertainty->max()) ||
      uncertainty->min() <= 0.0 || uncertainty->min() >= uncertainty->max()) {
    addError(errors, QObject::tr("%1 bounds must be finite, positive, and "
                                 "increasing.")
                         .arg(name));
  }
  if (!validNumber(uncertainty->lnStdev()) || uncertainty->lnStdev() < 0.0 ||
      uncertainty->lnStdev() > 1.0) {
    addError(errors, QObject::tr("%1 logarithmic standard deviation must be "
                                 "between 0 and 1.")
                         .arg(name));
  }
}
} // namespace

SiteResponseModel::SiteResponseModel(QObject *parent)
    : QThread(parent), _calculator(nullptr) {
  _modified = false;
  _hasResults = false;
  _method = EquivalentLinear;
  _okToContinue = true;
  _isLoaded = false;

  connect(Units::instance(), &Units::systemChanged, this,
          [this]() { setModified(); });

  _randNumGen = new MyRandomNumGenerator(this);
  connect(_randNumGen, &MyRandomNumGenerator::wasModified, this,
          [this]() { setModified(); });

  _motionLibrary = new MotionLibrary(this);
  _motionLibrary->setReadOnly(_hasResults);
  connect(_motionLibrary, &MotionLibrary::wasModified, this,
          [this]() { setModified(); });
  connect(this, &SiteResponseModel::hasResultsChanged, _motionLibrary,
          &MotionLibrary::setReadOnly);

  _siteProfile = new SoilProfile(this);
  _siteProfile->setReadOnly(_hasResults);
  connect(_siteProfile, &SoilProfile::wasModified, this,
          [this]() { setModified(); });
  connect(this, &SiteResponseModel::hasResultsChanged, _siteProfile,
          &SoilProfile::setReadOnly);

  _outputCatalog = new OutputCatalog(this);
  _outputCatalog->setReadOnly(_hasResults);
  connect(this, &SiteResponseModel::hasResultsChanged, _outputCatalog,
          &OutputCatalog::setReadOnly);
  connect(_outputCatalog, &OutputCatalog::wasModified, this,
          [this]() { setModified(); });
  connect(this->motionLibrary(), &MotionLibrary::approachChanged,
          _outputCatalog->profilesCatalog(),
          &ProfilesOutputCatalog::setApproach);

  // Associate the output soil types catalog with the input soil types catalog
  // Should these have a stronger link? As in be the same object?
  _outputCatalog->soilTypesCatalog()->setSoilTypeCatalog(
      _siteProfile->soilTypeCatalog());

  setCalculator(new EquivalentLinearCalculator(this));

  _notes = new QTextDocument(this);
  connect(_notes, &QTextDocument::contentsChanged, this,
          [this]() { setModified(); });

  Units::instance()->reset();
}

auto SiteResponseModel::methodList() -> QStringList {
  QStringList list;

  list << tr("Linear Elastic") << tr("Equivalent Linear (EQL)");

#ifdef ADVANCED_FEATURES
  list << tr("Frequency Dependent EQL");
#endif

  return list;
}

auto SiteResponseModel::fileName() const -> QString { return _fileName; }

void SiteResponseModel::setFileName(const QString &fileName) {
  if (_fileName != fileName) {
    _fileName = fileName;

    emit fileNameChanged(_fileName);
    setModified(true);
  }
}

auto SiteResponseModel::method() const -> SiteResponseModel::Method {
  return _method;
}

void SiteResponseModel::setMethod(Method method) {
  if (_method != method) {
    _method = method;

    switch (_method) {
    case LinearElastic:
      setCalculator(new LinearElasticCalculator(this));
      break;
    case EquivalentLinear:
      setCalculator(new EquivalentLinearCalculator(this));
      break;
    case FrequencyDependent:
      setCalculator(new FrequencyDependentCalculator(this));
      break;
    }

    emit methodChanged(_method);
    emit dampingRequiredChanged(dampingRequired());
    emit nonlinearPropertiesRequiredChanged(nonlinearPropertiesRequired());
    setModified(true);
  }
}

void SiteResponseModel::setMethod(int method) {
  setMethod(static_cast<Method>(method));
}

auto SiteResponseModel::dampingRequired() const -> bool {
  return _method == LinearElastic;
}

auto SiteResponseModel::nonlinearPropertiesRequired() const -> bool {
  return (_method == EquivalentLinear || _method == FrequencyDependent);
}

auto SiteResponseModel::modified() const -> bool { return _modified; }

void SiteResponseModel::setModified(bool modified) {
  _modified = modified;
  emit modifiedChanged(modified);

  //     FIXME
  //    if (_modified != modified && _isLoaded) {
  //        // FIXME When opening a previously saved document, the QTextDocument
  //        // containing the notes signals that it has been modified.  This
  //        happens
  //        // before the modified flag is reset.
  //    }
}

auto SiteResponseModel::notes() const -> QTextDocument * { return _notes; }

void SiteResponseModel::stop() {
  _okToContinue = false;
  _calculator->stop();
}

void SiteResponseModel::clearResults() {
  _outputCatalog->clear();
  setHasResults(false);
}

void SiteResponseModel::setIsLoaded(bool isLoaded) { _isLoaded = isLoaded; }

auto SiteResponseModel::siteProfile() -> SoilProfile * { return _siteProfile; }

auto SiteResponseModel::motionLibrary() -> MotionLibrary * {
  return _motionLibrary;
}

auto SiteResponseModel::calculator() -> AbstractCalculator * {
  return _calculator;
}

auto SiteResponseModel::outputCatalog() -> OutputCatalog * {
  return _outputCatalog;
}

auto SiteResponseModel::randNumGen() -> MyRandomNumGenerator * {
  return _randNumGen;
}

void SiteResponseModel::setCalculator(AbstractCalculator *calculator) {
  if (_calculator != calculator) {
    if (_calculator)
      _calculator->deleteLater();

    _calculator = calculator;
    _calculator->setTextLog(_outputCatalog->log());

    // FIXME: Is this really the correct solution?
    connect(_calculator, &AbstractCalculator::wasModified, this,
            [this]() { setModified(); });

    emit calculatorChanged(_calculator);
  }
}

auto SiteResponseModel::loadBinary(const QString &fileName) -> bool {
  setModified(false);
  _isLoaded = false;

  // Save the file name
  _fileName = fileName;

  QFile file(fileName);
  // If the file can't be opened halt
  if (!file.open(QIODevice::ReadOnly)) {
    qCritical("Unable to open file: %s", qPrintable(fileName));
    return false;
  }
  // Open the data stream
  QDataStream inStream(&file);

  // Read and check the header
  quint32 magic;
  inStream >> magic;

  if (magic != 0xA1B2) {
    qCritical() << "Bad file format!";
    return false;
  }

  // Attempt to read the QDataStream version used for serialization
  // Files created with Qt6 Strata will have this version marker
  // Legacy files (Qt5/Qt4) will not have this marker
  quint32 datastreamFormatVersion;
  qint64 checkpointPosition = file.pos();
  inStream >> datastreamFormatVersion;

  // Check if this looks like a valid QDataStream version number
  // Valid versions are typically between 5 (Qt_4_2) and 30+ (future Qt
  // versions) If the value is outside this range, it's likely actual data from
  // a legacy file
  bool isLegacyFormat =
      (datastreamFormatVersion < 5 || datastreamFormatVersion > 50);

  if (isLegacyFormat) {
    // Rewind - this was actual data, not a version marker
    file.seek(checkpointPosition);
    inStream.setVersion(QDataStream::Qt_4_0);
    qDebug() << "Loading legacy binary format (Qt_4_0)";
  } else {
    // Use the stored format version
    inStream.setVersion(
        static_cast<QDataStream::Version>(datastreamFormatVersion));
    qDebug() << "Loading binary format with QDataStream version:"
             << datastreamFormatVersion;
  }

  // Read the data
  inStream >> this;

  setModified(false);
  // Wait until all events have been processed and then reset the modified flag
  // QTimer::singleShot(10, this, SLOT(setIsLoaded()));

  return true;
}

auto SiteResponseModel::loadJson(const QString &fileName) -> bool {
  setModified(false);
  _isLoaded = false;

  // Save the file name
  _fileName = fileName;

  QFile file(_fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    qCritical("Unable to open file: %s", qPrintable(fileName));
    return false;
  }

  QByteArray savedData = file.readAll();
  QJsonDocument jsonDoc = QJsonDocument::fromJson(savedData);
  QJsonObject json = jsonDoc.object();

  //
  _notes->setHtml(json["notes"].toString());
  Units::instance()->setSystem(json["system"].toInt());

  _randNumGen->fromJson(json["randNumGen"].toObject());
  _siteProfile->fromJson(json["siteProfile"].toObject());
  _motionLibrary->fromJson(json["motionLibrary"].toObject());
  _outputCatalog->fromJson(json["outputCatalog"].toObject());

  setMethod(json["method"].toInt());
  const QJsonObject cjo = json["calculator"].toObject();
  switch (_method) {
  case SiteResponseModel::EquivalentLinear:
    qobject_cast<EquivalentLinearCalculator *>(_calculator)->fromJson(cjo);
    break;
  case SiteResponseModel::FrequencyDependent:
    qobject_cast<FrequencyDependentCalculator *>(_calculator)->fromJson(cjo);
    break;
  case SiteResponseModel::LinearElastic:
    break;
  }

  setHasResults(json["hasResults"].toBool());

  _outputCatalog->initialize(
      _siteProfile->isVaried() ? _siteProfile->profileCount() : 1,
      _motionLibrary);

  if (_hasResults) {
    _outputCatalog->finalize();
  }

  setModified(false);
  return true;
}

auto SiteResponseModel::saveBinary() -> bool {
  QFile file(_fileName);
  // If the file can't be opened halt
  if (!file.open(QIODevice::WriteOnly)) {
    qCritical() << "Error opening file:" << _fileName;
    return false;
  }
  // Open the data stream
  QDataStream outStream(&file);
  outStream.setVersion(QDataStream::Qt_6_0);

  // Write a header with a "magic number", QDataStream version, and data
  outStream << static_cast<quint32>(0xA1B2);
  // Store the QDataStream version for backward compatibility detection
  outStream << static_cast<quint32>(QDataStream::Qt_6_0);
  outStream << this;

  setModified(false);
  return true;
}

auto SiteResponseModel::saveJson() -> bool {
  QJsonObject json;

  json["notes"] = _notes->toPlainText();
  json["method"] = static_cast<int>(_method);
  json["hasResults"] = _hasResults;
  json["system"] = static_cast<int>(Units::instance()->system());

  json["randNumGen"] = _randNumGen->toJson();
  json["siteProfile"] = _siteProfile->toJson();
  json["motionLibrary"] = _motionLibrary->toJson();
  json["outputCatalog"] = _outputCatalog->toJson();

  switch (_method) {
  case SiteResponseModel::EquivalentLinear:
    json["calculator"] =
        qobject_cast<EquivalentLinearCalculator *>(_calculator)->toJson();
    break;
  case SiteResponseModel::FrequencyDependent:
    json["calculator"] =
        qobject_cast<FrequencyDependentCalculator *>(_calculator)->toJson();
    break;
  case SiteResponseModel::LinearElastic:
    break;
  }

  QFile file(_fileName);
  if (!file.open(QIODevice::WriteOnly)) {
    qWarning("Couldn't open save file.");
    return false;
  }

  QJsonDocument saveDoc(json);
  file.write(saveDoc.toJson(QJsonDocument::Indented));

  setModified(false);
  return true;
}

auto SiteResponseModel::hasResults() const -> bool { return _hasResults; }

auto SiteResponseModel::validationErrors() -> QStringList {
  QStringList errors;

  if (_siteProfile->soilLayers().isEmpty()) {
    addError(errors, tr("Add at least one soil layer."));
  }
  if (_motionLibrary->motionCount() == 0) {
    addError(errors, tr("Enable at least one input motion."));
  }

  if (!validNumber(_siteProfile->waterTableDepth()) ||
      _siteProfile->waterTableDepth() < 0.0) {
    addError(errors, tr("Water table depth must be finite and nonnegative."));
  }
  if (!validNumber(_siteProfile->maxFreq()) || _siteProfile->maxFreq() < 15.0 ||
      _siteProfile->maxFreq() > 100.0) {
    addError(errors, tr("Maximum frequency must be between 15 and 100 Hz."));
  }
  if (!validNumber(_siteProfile->waveFraction()) ||
      _siteProfile->waveFraction() < 0.10 ||
      _siteProfile->waveFraction() > 0.35) {
    addError(errors, tr("Wavelength fraction must be between 0.10 and 0.35."));
  }

  validateDistribution(errors, tr("Bedrock shear-wave velocity"),
                       _siteProfile->bedrock(), true);
  if (!validNumber(_siteProfile->bedrock()->untWt()) ||
      _siteProfile->bedrock()->untWt() < 10.0 ||
      _siteProfile->bedrock()->untWt() > 200.0) {
    addError(errors, tr("Bedrock unit weight must be between 10 and 200."));
  }
  if (!validNumber(_siteProfile->bedrock()->avgDamping()) ||
      _siteProfile->bedrock()->avgDamping() < 0.1 ||
      _siteProfile->bedrock()->avgDamping() > 5.0) {
    addError(errors, tr("Bedrock damping must be between 0.1 and 5 percent."));
  }

  for (int i = 0; i < _siteProfile->soilLayers().size(); ++i) {
    SoilLayer *layer = _siteProfile->soilLayers().at(i);
    const QString name = tr("Soil layer %1").arg(i + 1);
    if (!validNumber(layer->thickness()) || layer->thickness() <= 0.0) {
      addError(errors,
               tr("%1 thickness must be a finite positive value.").arg(name));
    }
    if (!layer->soilType()) {
      addError(errors, tr("%1 must have an assigned soil type.").arg(name));
    }
    validateDistribution(errors, name + tr(" shear-wave velocity"), layer,
                         true);
  }

  SoilTypeCatalog *soilTypes = _siteProfile->soilTypeCatalog();
  for (int i = 0; i < soilTypes->rowCount(); ++i) {
    SoilType *soilType = soilTypes->soilType(i);
    const QString name = tr("Soil type %1").arg(i + 1);
    if (!validNumber(soilType->untWt()) || soilType->untWt() <= 0.0) {
      addError(errors,
               tr("%1 unit weight must be a finite positive value.").arg(name));
    }
    if (!validNumber(soilType->damping()) || soilType->damping() < 0.0) {
      addError(
          errors,
          tr("%1 initial damping must be finite and nonnegative.").arg(name));
    }
    if (nonlinearPropertiesRequired()) {
      validateNonlinearProperty(errors, name + tr(" modulus curve"),
                                soilType->modulusModel());
      validateNonlinearProperty(errors, name + tr(" damping curve"),
                                soilType->dampingModel());
    }
  }

  if (auto *iterative =
          qobject_cast<AbstractIterativeCalculator *>(_calculator)) {
    if (iterative->maxIterations() < 2 || iterative->maxIterations() > 60) {
      addError(errors, tr("Maximum iterations must be between 2 and 60."));
    }
    if (!validNumber(iterative->errorTolerance()) ||
        iterative->errorTolerance() < 0.5 ||
        iterative->errorTolerance() > 10.0) {
      addError(errors,
               tr("Error tolerance must be between 0.5 and 10 percent."));
    }
  }
  if (auto *equivalent =
          qobject_cast<EquivalentLinearCalculator *>(_calculator)) {
    if (!validNumber(equivalent->strainRatio()) ||
        equivalent->strainRatio() < 0.45 || equivalent->strainRatio() > 0.80) {
      addError(errors,
               tr("Effective strain ratio must be between 0.45 and 0.80."));
    }
  }

  if (_siteProfile->isVaried() && _siteProfile->profileCount() < 1) {
    addError(errors, tr("Number of realizations must be at least one."));
  }
  if (_siteProfile->profileRandomizer()->enabled()) {
    ProfileRandomizer *randomizer = _siteProfile->profileRandomizer();
    VelocityVariation *velocity = randomizer->velocityVariation();
    if (velocity->enabled()) {
      if (velocity->stdevModel() < VelocityVariation::Custom ||
          velocity->stdevModel() > VelocityVariation::USGS_D ||
          velocity->correlModel() < VelocityVariation::Custom ||
          velocity->correlModel() > VelocityVariation::USGS_D) {
        addError(errors, tr("Velocity variation has an invalid model."));
      }
      if (velocity->stdevCustomEnabled() &&
          (!validNumber(velocity->stdev()) || velocity->stdev() < 0.0 ||
           velocity->stdev() > 1.0)) {
        addError(errors, tr("Velocity variation standard deviation must be "
                            "between 0 and 1."));
      }
      if (velocity->correlCustomEnabled() &&
          (!validNumber(velocity->correlInitial()) ||
           !validNumber(velocity->correlFinal()) ||
           !validNumber(velocity->correlDelta()) ||
           !validNumber(velocity->correlIntercept()) ||
           !validNumber(velocity->correlExponent()) ||
           velocity->correlInitial() < -1.0 ||
           velocity->correlInitial() > 1.0 || velocity->correlFinal() < -1.0 ||
           velocity->correlFinal() > 1.0 || velocity->correlDelta() < 0.0 ||
           velocity->correlDelta() > 10.0 ||
           velocity->correlIntercept() < 0.0 ||
           velocity->correlIntercept() > 100.0 ||
           velocity->correlExponent() < 0.0 ||
           velocity->correlExponent() > 1.0)) {
        addError(errors, tr("Velocity variation correlation parameters are out "
                            "of range."));
      }
    }

    LayerThicknessVariation *layering = randomizer->layerThicknessVariation();
    if (layering->enabled() && layering->customEnabled() &&
        (!validNumber(layering->coeff()) || !validNumber(layering->initial()) ||
         !validNumber(layering->exponent()) || layering->coeff() < 0.0 ||
         layering->coeff() > 100.0 || layering->initial() < 0.0 ||
         layering->initial() > 100.0 || layering->exponent() < -5.0 ||
         layering->exponent() > 0.0)) {
      addError(errors,
               tr("Layer-thickness variation parameters are out of range."));
    }
    BedrockDepthVariation *bedrockDepth = randomizer->bedrockDepthVariation();
    if (bedrockDepth->enabled()) {
      validateDistribution(errors, tr("Bedrock-depth variation"), bedrockDepth,
                           true);
    }
  }

  NonlinearPropertyRandomizer *nonlinearRandomizer =
      _siteProfile->nonlinearPropertyRandomizer();
  if (nonlinearRandomizer->enabled()) {
    if (nonlinearRandomizer->model() < NonlinearPropertyRandomizer::SPID ||
        nonlinearRandomizer->model() > NonlinearPropertyRandomizer::Darendeli) {
      addError(errors,
               tr("Nonlinear-property variation has an invalid model."));
    }
    if (!validNumber(nonlinearRandomizer->correl()) ||
        nonlinearRandomizer->correl() < -1.0 ||
        nonlinearRandomizer->correl() > 1.0) {
      addError(errors,
               tr("Nonlinear-property correlation must be between -1 and 1."));
    }
    if (nonlinearRandomizer->customEnabled()) {
      validateUncertainty(errors, tr("Modulus uncertainty"),
                          nonlinearRandomizer->modulusUncert());
      validateUncertainty(errors, tr("Damping uncertainty"),
                          nonlinearRandomizer->dampingUncert());
    }
  }

  for (int i = 0; i < _motionLibrary->rowCount(); ++i) {
    AbstractMotion *motion = _motionLibrary->motionAt(i);
    if (!motion->enabled())
      continue;
    const QString name = tr("Motion %1").arg(i + 1);
    if (auto *timeSeries = qobject_cast<TimeSeriesMotion *>(motion)) {
      if (!timeSeries->isLoaded() || timeSeries->accel().size() < 2) {
        addError(errors, tr("%1 must have loaded time-series data.").arg(name));
      }
      if (!validNumber(timeSeries->timeStep()) ||
          timeSeries->timeStep() < 0.0001 || timeSeries->timeStep() > 0.05) {
        addError(errors,
                 tr("%1 time step must be between 0.0001 and 0.05 seconds.")
                     .arg(name));
      }
      if (!validNumber(timeSeries->scale()) || timeSeries->scale() < 0.001 ||
          timeSeries->scale() > 20.0) {
        addError(errors,
                 tr("%1 scale must be between 0.001 and 20.").arg(name));
      }
    } else if (auto *rvt = qobject_cast<RvtMotion *>(motion)) {
      if (!validNumber(rvt->duration()) || rvt->duration() <= 0.0 ||
          rvt->freq().size() < 2 ||
          rvt->freq().size() != rvt->fourierAcc().size()) {
        addError(errors,
                 tr("%1 must define a positive duration and at least two "
                    "frequency-amplitude pairs.")
                     .arg(name));
      } else {
        for (int j = 0; j < rvt->freq().size(); ++j) {
          if (!validNumber(rvt->freq().at(j)) || rvt->freq().at(j) <= 0.0 ||
              !validNumber(rvt->fourierAcc().at(j)) ||
              rvt->fourierAcc().at(j) <= 0.0 ||
              (j > 0 && rvt->freq().at(j - 1) >= rvt->freq().at(j))) {
            addError(errors,
                     tr("%1 has invalid frequency-amplitude data.").arg(name));
            break;
          }
        }
      }
    } else if (auto *compatible = qobject_cast<CompatibleRvtMotion *>(motion)) {
      validateDimension(errors, name + tr(" frequency"),
                        compatible->freqDimension(), 0.001, 1000.0);
      ResponseSpectrum *spectrum = compatible->targetRespSpec();
      if (!validNumber(compatible->duration()) ||
          compatible->duration() <= 0.0 || spectrum->period().size() < 2 ||
          spectrum->period().size() != spectrum->sa().size()) {
        addError(errors,
                 tr("%1 must define a positive duration and target response "
                    "spectrum.")
                     .arg(name));
      } else {
        for (int j = 0; j < spectrum->period().size(); ++j) {
          if (!validNumber(spectrum->period().at(j)) ||
              spectrum->period().at(j) <= 0.0 ||
              !validNumber(spectrum->sa().at(j)) ||
              spectrum->sa().at(j) < 0.0 ||
              (j > 0 &&
               spectrum->period().at(j - 1) >= spectrum->period().at(j))) {
            addError(
                errors,
                tr("%1 has invalid target response-spectrum data.").arg(name));
            break;
          }
        }
      }
    } else if (auto *source = qobject_cast<SourceTheoryRvtMotion *>(motion)) {
      validateDimension(errors, name + tr(" frequency"),
                        source->freqDimension(), 0.001, 1000.0);
      if (!validNumber(source->magnitude()) || source->magnitude() < 4.0 ||
          source->magnitude() > 9.0 || !validNumber(source->distance()) ||
          source->distance() < 0.0 || source->distance() > 2000.0 ||
          !validNumber(source->depth()) || source->depth() <= 0.0 ||
          !validNumber(source->duration()) || source->duration() <= 0.0) {
        addError(
            errors,
            tr("%1 has invalid source-theory scenario parameters.").arg(name));
      }
      if (source->isCustomized() &&
          (!validNumber(source->stressDrop()) || source->stressDrop() <= 0.0 ||
           !validNumber(source->geoAtten()) || source->geoAtten() <= 0.0 ||
           !validNumber(source->pathAttenCoeff()) ||
           source->pathAttenCoeff() <= 0.0 ||
           !validNumber(source->pathAttenPower()) ||
           source->pathAttenPower() <= 0.0 ||
           !validNumber(source->shearVelocity()) ||
           source->shearVelocity() <= 0.0 || !validNumber(source->density()) ||
           source->density() <= 0.0 || !validNumber(source->siteAtten()) ||
           source->siteAtten() < 0.0)) {
        addError(errors,
                 tr("%1 has invalid customized source-theory parameters.")
                     .arg(name));
      }
    }
  }

  if (_outputCatalog->periodIsNeeded()) {
    if (!validNumber(_outputCatalog->damping()) ||
        _outputCatalog->damping() < 1.0 || _outputCatalog->damping() > 50.0) {
      addError(
          errors,
          tr("Response spectrum damping must be between 1 and 50 percent."));
    }
    validateDimension(errors, tr("Response spectrum periods"),
                      _outputCatalog->period(), 0.001, 100.0);
  }
  if (_outputCatalog->frequencyIsNeeded()) {
    validateDimension(errors, tr("Output frequencies"),
                      _outputCatalog->frequency(), 0.001, 1000.0);
  }

  return errors;
}

void SiteResponseModel::setHasResults(bool hasResults) {
  if (_hasResults != hasResults) {
    _hasResults = hasResults;

    emit hasResultsChanged(hasResults);
  }
}

void SiteResponseModel::run() {
  _okToContinue = true;
  setHasResults(false);

  const QStringList errors = validationErrors();
  if (!errors.isEmpty()) {
    _outputCatalog->clear();
    _outputCatalog->log()->append(tr("<b>Calculation not started:</b>"));
    for (const QString &error : errors)
      _outputCatalog->log()->append(tr(" - %1").arg(error));
    _okToContinue = false;
    return;
  }

  _outputCatalog->clear();
  _outputCatalog->log()->append(tr("<b>Starting Strata Calculation</b>"));

  // Determine the number of sites to be used in the computation
  const int siteCount =
      _siteProfile->isVaried() ? _siteProfile->profileCount() : 1;

  // Initialize the random number generator
  _randNumGen->init();

  // Initialize the output
  _outputCatalog->initialize(siteCount, _motionLibrary);

  // Setup the progress bar with the number of steps
  const int motionCount = _motionLibrary->motionCount();
  const int totalCount = motionCount * siteCount;
  emit progressRangeChanged(0, totalCount);
  emit progressChanged(0);

  _outputCatalog->log()->append(tr("%1 Trial(s) (%2 Site(s) and %3 Motion(s) )")
                                    .arg(totalCount)
                                    .arg(siteCount)
                                    .arg(motionCount));

  int count = 0;
  for (int i = 0; i < siteCount; ++i) {
    // Break if not okay to continue
    if (!_okToContinue) {
      break;
    }

    _outputCatalog->log()->append(
        QString(tr("[%1 of %2] Generating site and soil properties"))
            .arg(i + 1)
            .arg(siteCount));

    // Create the sublayers -- this randomizes the properties
    _siteProfile->createSubLayers(_outputCatalog->log());

    // FIXME -- check the site profile to ensure that the waves can be
    // computed for the intial coniditions
    int motionCountOffset = 0;
    bool profileResultsSaved = false;
    for (int j = 0; j < _motionLibrary->rowCount(); ++j) {
      if (!_motionLibrary->motionAt(j)->enabled()) {
        // Skip the disabled motion
        ++motionCountOffset;
        continue;
      }

      if (!_okToContinue) {
        // Break if not okay to continue
        break;
      }

      // Output status
      _outputCatalog->log()->append(
          QString(tr("\t[%1 of %2] Computing site response for motion: %3"))
              .arg(j - motionCountOffset + 1)
              .arg(motionCount)
              .arg(_motionLibrary->motionAt(j)->name()));

      // Compute the site response
      bool calcOk = _calculator->run(_motionLibrary->motionAt(j), _siteProfile);

      if (!calcOk || (_siteProfile->onlyConverged() &&
                      _calculator->status() == NoConvergence)) {
        if (siteCount > 1) {
          // Error in the calculation -- need to remove the site
          _outputCatalog->log()->append(
              tr("\tCalculation failed -- removing site."));
          // Remove the results if they were saved
          if (profileResultsSaved) {
            _outputCatalog->removeLastSite();
          }
          // Reset site count and try once again
          --i;
          // Stop iterating over motions
          break;
        } else {
          _okToContinue = false;
          break;
        }
      }

      // Generate the output
      _outputCatalog->saveResults(j - motionCountOffset, _calculator);
      profileResultsSaved = true;
      // Increment the progress bar
      ++count;
      emit progressChanged(count);

      // Reset the sublayers
      _siteProfile->resetSubLayers();
    }
  }

  if (_okToContinue) {
    // Compute the statistics of the output
    _outputCatalog->log()->append(tr("Computing statistics."));
    _outputCatalog->finalize();
    setHasResults(true);
  } else {
    _outputCatalog->log()->append(tr("<b>No results!</b>"));
    setHasResults(false);
  }
}

auto SiteResponseModel::toHtml() -> QString {
  QString html;

  // Define the html header
  html +=
      "<html><head>"
      "<title>Strata Input</title>"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />"
      "<style type=\"text/css\">"
      "ol {"
      "list-style-type: upper-roman;"
      "font-size: 	medium;"
      "font-weight:   bold;"
      "}"
      "ol ol {"
      "list-style-type: upper-alpha;"
      "font-size: 	medium;"
      "font-weight:   bold;"
      "}"
      "ol ol ol {"
      "list-style-type: decimal;"
      "font-size: 	medium;"
      "font-weight:   bold;"
      "}"
      "ol ol ol ol {"
      "list-style-type: lower-alpha;"
      "font-size: 	medium;"
      "font-weight:   bold;"
      "}"
      "strong {"
      "font-size:     small;"
      "font-weight:   bold;"
      "}"
      "th {"
      "font-size:     small;"
      "font-weight:   bold;"
      "padding: 2px 4px 2px 4px;"
      "align: right;"
      "}"
      "td {"
      "font-size: 	small;"
      "font-weight:   normal;"
      "padding: 2px 4px 2px 4px;"
      "}"
      "table {"
      "border-style:  solid;"
      "border-collapse:  collapse;"
      "}"
      "</style>"
      "</head>";

  // Project
  html +=
      tr("<h1>%1</h1>"
         "<ol>"
         "<li>General Settings"
         "<ol>"
         "<li>Project"
         "<table border=\"0\">"
         "<tr><th>Title:</th><td>%1</td></tr>"
         "<tr><th>Notes:</th><td>%2</td></tr>"
         "<tr><th>File preffix:</th><td>%3</td></tr>"
         "<tr><th>Units System:</th><td>%4</td></tr>"
         "</table>"
         "</li>")
          .arg(_outputCatalog->title(), _notes->toHtml(),
               _outputCatalog->filePrefix(),
               Units::instance()->systemList().at(Units::instance()->system()));

  // Type of Analysis
  html += tr("<li>Type of Analysis"
             "<table border=\"0\">"
             "<tr><th>Analysis Method:</th><td>%1</td></tr>"
             "<tr><th>Approach:</th><td>%2</td></tr>"
             "<tr><th>Properties Varied:</th><td>%3</td></tr>"
             "</table>"
             "</li>")
              .arg(methodList().at(_method),
                   MotionLibrary::approachList().at(_motionLibrary->approach()),
                   boolToString(_siteProfile->isVaried()));

  // Site Variation
  if (_siteProfile->isVaried())
    html +=
        tr("<li>Site Property Variation"
           "<table border=\"0\">"
           "<tr><th>Number of realizations:</th><td>%1</td></tr>"
           "<tr><th>Vary the nonlinear soil properties:</th><td>%2</td></tr>"
           "<tr><th>Vary the site profile:</th><td>%3</td></tr>"
           "</table>"
           "</li>")
            .arg(_siteProfile->profileCount())
            .arg(boolToString(
                     _siteProfile->nonlinearPropertyRandomizer()->enabled()),
                 boolToString(_siteProfile->profileRandomizer()->enabled()));

  // Layer Discretization
  html += tr("<li>Layer Discretization"
             "<table border=\"0\">"
             "<tr><th>Maximum frequency:</th><td>%1 Hz</td></tr>"
             "<tr><th>Wavelength fraction:</th><td>%2</td></tr>"
             "</table>"
             "</li>")
              .arg(_siteProfile->maxFreq())
              .arg(_siteProfile->waveFraction());

  // Calculator parameters
  html += _calculator->toHtml();

  html += "</ol>";

  // Site profile
  html += "<li>" + _siteProfile->toHtml() + "</li>";

  // Motions
  html += tr("<li>Motion(s)");

  QString loc;
  if (_siteProfile->inputDepth() < 0)
    loc = "Bedrock";
  else
    loc = QString("%1 %2")
              .arg(_siteProfile->inputDepth())
              .arg(Units::instance()->length());

  html += tr("<table border=\"0\"><tr><th>Input "
             "Location:</th><td>%1</td></tr></table>")
              .arg(loc);

  html += _motionLibrary->toHtml() + "</li>";

  // Close the html file
  html += "</ol></html>";

  return html;
}

auto operator<<(QDataStream &out, const SiteResponseModel *srm)
    -> QDataStream & {
  out << static_cast<quint8>(
      3); // Version 3: Qt6 migration with QDataStream version tracking

  out << Units::instance() << srm->_notes->toPlainText()
      << (quint32)srm->_method << srm->_siteProfile << srm->_motionLibrary
      << srm->_outputCatalog << srm->_randNumGen << srm->_hasResults;

  switch (srm->_method) {
  case SiteResponseModel::EquivalentLinear:
    out << qobject_cast<EquivalentLinearCalculator *>(srm->_calculator);
    break;
  case SiteResponseModel::FrequencyDependent:
    out << qobject_cast<FrequencyDependentCalculator *>(srm->_calculator);
    break;
  case SiteResponseModel::LinearElastic:
    break;
  }

  return out;
}

auto operator>>(QDataStream &in, SiteResponseModel *srm) -> QDataStream & {
  quint8 ver;
  in >> ver;

  QString notes;
  quint32 method;
  bool hasResults;

  in >> Units::instance() >> notes >> method;

  in >> srm->_siteProfile;
  // TODO Need to update motion count
  in >> srm->_motionLibrary;
  in >> srm->_outputCatalog;

  if (ver > 1) {
    in >> srm->_randNumGen;
  }

  in >> hasResults;

  srm->_notes->setPlainText(notes);
  srm->setMethod(method);
  srm->_outputCatalog->initialize(
      srm->_siteProfile->isVaried() ? srm->_siteProfile->profileCount() : 1,
      srm->_motionLibrary);

  if (hasResults)
    srm->_outputCatalog->finalize();

  switch (srm->_method) {
  case SiteResponseModel::EquivalentLinear:
    in >> qobject_cast<EquivalentLinearCalculator *>(srm->_calculator);
    break;
  case SiteResponseModel::FrequencyDependent:
    in >> qobject_cast<FrequencyDependentCalculator *>(srm->_calculator);
    break;
  case SiteResponseModel::LinearElastic:
    break;
  }

  // Need to update the other objects that the model has data and should not be
  // editted.
  srm->setHasResults(hasResults);

  return in;
}
