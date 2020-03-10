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

#ifndef ABSTRACT_OUTPUT_H
#define ABSTRACT_OUTPUT_H

#include <QAbstractTableModel>
#include <QDataStream>
#include <QJsonObject>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class AbstractCalculator;
class AbstractOutputInterpolater;
class OutputCatalog;
class OutputStatistics;

class AbstractOutput : public QAbstractTableModel
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const AbstractOutput* ao) -> QDataStream &;
    friend auto operator>> (QDataStream & in, AbstractOutput* ao) -> QDataStream &;

public:
    enum CurveType{
        Yfx,
        Xfy,
    };

    explicit AbstractOutput(OutputCatalog* catalog);

    virtual auto rowCount(const QModelIndex & parent = QModelIndex()) const -> int;
    virtual auto columnCount(const QModelIndex & parent = QModelIndex()) const -> int;

    virtual auto data(const QModelIndex & index, int role = Qt::DisplayRole) const -> QVariant;
    virtual auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant;

    //! Add the data to the output
    virtual void addData(int motion, AbstractCalculator* const calculator);

    //! Finalize the output by computing statistics if possible
    virtual void finalize();

    //! Remove the last site from the output
    void removeLastSite();

    //! Configure the plot
    virtual void plot(QwtPlot* const qwtPlot, QList<QwtPlotCurve*> & curves) const;

    /*! Create a text file from the data
     *
     * \param path location to save the files
     * \param separator character used to separate the columns of data
     * \param prefix prefix to append to the start of filename
     */
    virtual void exportData(const QString &path, const QString &separator, const QString &prefix);

    //! Short name to identify the output
    virtual auto name() const -> QString = 0;

    //! Fullname that includes the general class of the output
    virtual auto fullName() const -> QString = 0;

    //! Remove all saved data
    virtual void clear();

    //! If the output is enabled for export to text file
    auto exportEnabled() const -> bool;

    //! If the series is enabled and included in the statistics
    auto seriesEnabled(int site, int motion) -> bool;

    //! If the Output needs depth for reference
    virtual auto needsDepth() const -> bool;

    //! If the Output needs frequency for reference
    virtual auto needsFreq() const -> bool;

    //! If the Output need period and damping for reference
    virtual auto needsPeriod() const -> bool;

    //! If the Output needs time for reference
    virtual auto needsTime() const -> bool;

    //! If the Output is only provided for Time Series
    virtual auto timeSeriesOnly() const -> bool;

    auto motionIndex() const -> int;

    //! Data for a given motion and site index
    virtual auto data(int site, int motion) const -> const QVector<double>&;

    //! Reference for a given motion and site index
    virtual auto ref(int motion = 0) const -> const QVector<double>& = 0;

    //! Convert between general index and the site and motion
    void intToSiteMotion(int i, int* site, int* motion) const;

    //! Convert between a general index and a site index
    auto intToSite(int i) const -> int;

    //! Convert between a general index and a motion index
    auto intToMotion(int i) const -> int;

    //! If the output is independent of the site
    virtual auto siteIndependent() const -> bool;

    //! If the output is independent of the motion
    virtual auto motionIndependent() const -> bool;

    //! Number of known sites
    auto siteCount() const -> int;

    //! Number of known motions
    auto motionCount() const -> int;

    //! Type of Curve
    virtual auto curveType() const -> AbstractOutput::CurveType;

    //! Z order for a data curve
    static auto zOrder() -> int;

    //! Offset for plotting the curves
    auto offset() const -> int;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

signals:
    void exportEnabledChanged(bool exportEnabled);
    void wasModified();
    void cleared();

public slots:
    void setExportEnabled(bool exportEnabled);
    void setMotionIndex(int motionIndex);

protected:       
    //! Name suitable for a text file
    virtual auto fileName(int motion = 0) const -> QString = 0;

    //! Abbreviated name suitable for files
    virtual auto shortName() const -> QString = 0;

    //! Set the font and labels of the axes
    virtual void labelAxes(QwtPlot* const qwtPlot) const;

    //! Reference axis
    virtual auto xScaleEngine() const -> QwtScaleEngine* = 0;

    //! Data axis
    virtual auto yScaleEngine() const -> QwtScaleEngine* = 0;

    //! Name of the reference label
    virtual auto xLabel() const -> const QString = 0;

    //! Name of the data label
    virtual auto yLabel() const -> const QString = 0;

    //! Prefix for the name and fileName
    virtual auto prefix() const -> const QString;

    //! Suffix for the name and fileName
    virtual auto suffix() const -> const QString;

    //! Extract the data from the calculator
    virtual void extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const = 0;

    //! Convert from a table column to a site motion pair
    void columnToSiteMotion(const int column, int* site, int* motion) const;

    //! If the output is to be exported and saved to a text file
    bool _exportEnabled;

    //! Reference the catalog the stores the reference information
    OutputCatalog* _catalog;

    //! Data values
    QList<QList<QVector<double> > > _data;

    //! Statistics of the output
    OutputStatistics* _statistics;

    //! Interpolater used to interpolate the output
    AbstractOutputInterpolater* _interp;

    /*! Initial offset of the data for plotting
     * For some output the first data value is ignore because it is zero and
     * causes problems when plotting on a log-log scale.
     */
    int _offset;

    //! Which motion should be displayed by the table -- only important for output that needs time
    int _motionIndex;

    //! Size of the longest output
    int _maxSize;
};

#endif // ABSTRACT_OUTPUT_H
