#ifndef ABSTRACTPEAKCALCULATOR_H
#define ABSTRACTPEAKCALCULATOR_H

#include <QMap>
#include <QString>
#include <QVector>

#include <complex>

class AbstractPeakCalculator {
public:
    explicit AbstractPeakCalculator(const QString &name);

    double calcPeak(
            double duration,
            const QVector<double> &freqs,
            const QVector<double> &fourierAmps,
            double oscFreq = 0,
            double oscDamping = 0,
            const QVector<std::complex<double> > &siteTransFunc = QVector<std::complex<double> >());

    virtual double calcDurationRms(
            double duration,
            double oscFreq,
            double oscDamping,
            const QVector<std::complex<double> > &siteTransFunc);

protected:
    virtual double calcPeakFactor(double duration, double oscFreq, double oscDamping) = 0;


    void initCache(const QVector<double> &freqs,
                   const QVector<double> &fourierAmps);
    void clearCache();

    double getMoment(int power);

    double limitZeroCrossings(double) const;

    QString _name;

    QVector<double> _freqs;
    QVector<double> _squaredAmps;

    QMap<int, double> _momentCache;
};

#endif // ABSTRACTPEAKCALCULATOR_H
