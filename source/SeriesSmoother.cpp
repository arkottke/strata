#include "SeriesSmoother.h"

#include <cmath>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

#include <QDebug>

SeriesSmoother::SeriesSmoother(int coeffCount, int pointCount)
    : m_coeffCount(coeffCount), m_pointCount(pointCount)
{
    // Compute the coefficients
    computeCoeffs();
}

QVector<double> SeriesSmoother::smooth( const QVector<double> & rawData)
{
    // New smooth data
    QVector<double> smData(rawData.size());

    const int midPoint = (m_pointCount-1)/2;

    // Coefficient index
    int coeffIdx = 0;
    // Index of first data point
    int startIdx = 0;

    for (int i = 0; i < rawData.size(); ++i) {
        // Determine which row of coefficients to use
        if ( i < midPoint ) {
            // Start of the data
            coeffIdx = i;
            startIdx = 0;
        } else if ( rawData.size() < i + midPoint ) {
            // End of the data
            coeffIdx = rawData.size() - i;
            startIdx = rawData.size() - m_pointCount;
        } else {
            // Middle of the data
            coeffIdx = midPoint;
            startIdx = i - midPoint;
        }
        
        double sum = 0;

        for (int j = 0; j < m_pointCount; ++j) {
            sum += m_coeffs.at(coeffIdx).at(j) * rawData.at(j+startIdx);
        }

        smData[i] = sum;
    }

    return smData;
}

void SeriesSmoother::computeCoeffs()
{
    gsl_matrix * X = gsl_matrix_alloc( m_pointCount, m_coeffCount );

    // Shift between the x index and the row number
    int delta = (m_pointCount-1)/2;

    for( int i = 0; i < m_pointCount; ++i )
        for (int j = 0; j < m_coeffCount; ++j)
            gsl_matrix_set( X, i, j, pow( i-delta, j ) );

    // [X]^T [X]
    gsl_matrix * XtX = gsl_matrix_alloc( m_coeffCount, m_coeffCount );
    gsl_blas_dgemm( CblasTrans, CblasNoTrans, 1, X, X, 0, XtX );

    // ([X]^T [X])^-1
    gsl_matrix * XtXi = gsl_matrix_alloc( m_coeffCount, m_coeffCount );
    int iXtXi; //the calculation needs an int pointer
    gsl_permutation * pXtX = gsl_permutation_alloc( m_coeffCount );
    gsl_linalg_LU_decomp( XtX, pXtX, &iXtXi );
    gsl_linalg_LU_invert( XtX, pXtX, XtXi );

    // [X] ([X]^t [X])^-1
    gsl_matrix * XXtXi = gsl_matrix_alloc( m_pointCount, m_coeffCount );
    gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1, X, XtXi, 0, XXtXi );

    // [coeffs] = [X] ([X]^t [X])^-1 [X]^t
    gsl_matrix * coeffs = gsl_matrix_alloc( m_pointCount, m_pointCount );
    gsl_blas_dgemm( CblasNoTrans, CblasTrans, 1, XXtXi, X, 0, coeffs );


    // Move the coeffs matrix to m_coeffs
    m_coeffs.resize(m_pointCount);

    for (int i = 0; i < m_coeffs.size(); ++i) {
        m_coeffs[i].resize(m_pointCount);
        
        for (int j = 0; j < m_pointCount; ++j)
            m_coeffs[i][j] = gsl_matrix_get( coeffs, i, j );
    }

    // Delete the matrices
    gsl_matrix_free(X);
    gsl_matrix_free(XtX);
    gsl_matrix_free(XtXi);
    gsl_permutation_free(pXtX);
    gsl_matrix_free(XXtXi);
    gsl_matrix_free(coeffs);
}
