#ifndef SERIES_SMOOTHER_H_
#define SERIES_SMOOTHER_H_


#include <QVector>


//! Class to smooth a series of data using the Savitzky-Golay method

class SeriesSmoother 
{
    public:
        SeriesSmoother(int coeffCount, int pointCount);
        
        QVector<double> smooth( const QVector<double> & rawData );

    private:
        //! Compute the coefficients of the smoothing
        void computeCoeffs();

        //! Count of the coefficients used in the smoothing (order of the polynomial plus one)
        int m_coeffCount;

        //! Number of points to smooth over (needs to be odd)
        int m_pointCount;
       
        //! Coefficients for the convolution
        QVector<QVector<double> > m_coeffs;
};
#endif

