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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#ifndef ABSTRACT_OUTPUT_H
#define ABSTRACT_OUTPUT_H

#include <QAbstractTableModel>

#include <qwt_plot_curve.h>
#include <qwt_plot.h>

class AbstractCalculator;
class AbstractOutputInterpolater;
class OutputCatalog;
class OutputStatistics;

class AbstractOutput : public QAbstractTableModel
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractOutput* ao);
    friend QDataStream & operator>> (QDataStream & in, AbstractOutput* ao);

public:
    explicit AbstractOutput(OutputCatalog* catalog);

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

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
    virtual QString name() const = 0;

    //! Fullname that includes the general class of the output
    virtual QString fullName() const = 0;

    //! Remove all saved data
    virtual void clear();

    //! If the output is enabled for export to text file
    bool exportEnabled() const;

    //! If the series is enabled and included in the statistics
    bool seriesEnabled(int site, int motion);

    //! If the Output needs depth for reference
    virtual bool needsDepth() const;

    //! If the Output needs frequency for reference
    virtual bool needsFreq() const;

    //! If the Output need period and damping for reference
    virtual bool needsPeriod() const;

    //! If the Output needs time for reference
    virtual bool needsTime() const;

    int motionIndex() const;

    //! Data for a given motion and site index
    virtual const QVector<double>& data(int site, int motion) const;

    //! Reference for a given motion and site index
    virtual const QVector<double>& ref(int motion = 0) const = 0;

    //! Convert between general index and the site and motion
    void intToSiteMotion(int i, int* site, int* motion) const;

    //! Convert between a general index and a site index
    int intToSite(int i) const;

    //! Convert between a general index and a motion index
    int intToMotion(int i) const;

    //! If the output is independent of the site
    virtual bool siteIndependent() const;

    //! If the output is independent of the motion
    virtual bool motionIndependent() const;

    //! Number of known sites
    const int siteCount() const;

    //! Number of known motions
    const int motionCount() const;

    //! Type of Curve
    virtual QwtPlotCurve::CurveType curveType() const;

    //! Z order for a data curve
    static int zOrder();

    //! Offset for plotting the curves
    int offset() const;

signals:
    void exportEnabledChanged(bool exportEnabled);
    void wasModified();
    void cleared();

public slots:
    void setExportEnabled(bool exportEnabled);
    void setMotionIndex(int motionIndex);

protected:       
    //! Name suitable for a text file
    virtual QString fileName(int motion = 0) const = 0;

    //! Abbreviated name suitable for files
    virtual QString shortName() const = 0;

    //! Set the font and labels of the axes
    virtual void labelAxes(QwtPlot* const qwtPlot) const;

    //! Reference axis
    virtual QwtScaleEngine* xScaleEngine() const = 0;

    //! Data axis
    virtual QwtScaleEngine* yScaleEngine() const = 0;

    //! Name of the reference label
    virtual const QString xLabel() const = 0;

    //! Name of the data label
    virtual const QString yLabel() const = 0;

    //! Prefix for the name and fileName
    virtual const QString prefix() const;

    //! Suffix for the name and fileName
    virtual const QString suffix() const;

    //! Extract the data from the calculator
    virtual void extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const = 0;

    //! Convert from a table column to a site motion pair
    void columnToSiteMotion(const int column, int* site, int* motion) const;

    //! If the output is to be exported and saved to a text file
    bool m_exportEnabled;

    //! Reference the catalog the stores the reference information
    OutputCatalog* m_catalog;

    //! Data values
    QList<QList<QVector<double> > > m_data;

    //! Statistics of the output
    OutputStatistics* m_statistics;

    //! Interpolater used to interpolate the output
    AbstractOutputInterpolater* m_interp;

    /*! Initial offset of the data for plotting
     * For some output the first data value is ignore because it is zero and
     * causes problems when plotting on a log-log scale.
     */
    int m_offset;

    //! Which motion should be displayed by the table -- only important for output that needs time
    int m_motionIndex;

    //! Size of the longest output
    int m_maxSize;
};

#endif // ABSTRACT_OUTPUT_H
