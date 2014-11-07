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
// Copyright 2007 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "NonlinearProperty.h"

#include "Dimension.h"
#include "SoilType.h"

#include <QBrush>
#include <QColor>
#include <QDebug>

#include <cfloat>
#include <cmath>

NonlinearProperty::NonlinearProperty(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_interp = 0;
    m_acc = gsl_interp_accel_alloc();
}

NonlinearProperty::NonlinearProperty(
        const QString &name, Type type, const QVector<double> &strain,
        const QVector<double> &property, QObject *parent)
            : QAbstractTableModel(parent), m_name(name), m_type(type), m_strain(strain),
                                         m_average(property), m_varied(property)
{
    m_interp = 0;
    m_acc = gsl_interp_accel_alloc();
    initialize();
}

NonlinearProperty::~NonlinearProperty()
{
    gsl_interp_accel_free(m_acc);

    if (m_interp)
        gsl_interp_free(m_interp);
}

NonlinearProperty::Type NonlinearProperty::type() const
{
    return m_type;
}

const QString & NonlinearProperty::name() const
{
    return m_name;
}

double NonlinearProperty::interp(const double strain) const
{
    double d;

    if (strain < m_strain.first()) {
        d = m_varied.first();
    } else if (strain > m_strain.last()) {
        d = m_varied.last();
    } else if (m_strain.size() == 1) {
        // Interpolater won't be intialized
        d = m_varied.first();
    } else {
        Q_ASSERT(m_interp);
        d = gsl_interp_eval(m_interp, m_strain.data(), m_varied.data(), strain, m_acc);
    }

    return d;
}

QString NonlinearProperty::toHtml() const
{  
    QString html = tr(
            "<h4>%1</h4>"
            "<table border = \"0\">"
            "<tr><th>Type:</th><td>%1</td></tr>"
            "<tr><th>Name:</th><td>%2</td></tr>"
            "</table>")
            .arg(m_type == ModulusReduction ?
                 tr("Shear Modulus Reduction") : tr("Damping Ratio"))
            .arg(m_name);


    html += "<table border=\"1\"><tr>";

    // Header
    for (int i = 0; i < columnCount(); ++i)
        html += "<th>" + headerData(i, Qt::Horizontal).toString() + "</th>";

    html += "</tr>";

    // Data
    for (int r = 0; r < rowCount(); ++r) {
        html += "<tr>";
        for (int c = 0; c < columnCount(); ++c)
            html += "<td>" + data(index(r, c)).toString() + "</td>";

        html += "</tr>";
    }
    html += "</table>";

    return html;
}

int NonlinearProperty::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_strain.size();
}

int NonlinearProperty::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 2;
}

QVariant NonlinearProperty::data(const QModelIndex &index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case StrainColumn:
            return QString::number(m_strain.at(index.row()), 'e', 2);
        case PropertyColumn:
            return QString::number(m_average.at(index.row()), 'f', 3);
        }
    }

    return QVariant();
}

QVariant NonlinearProperty::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        switch (section) {
        case StrainColumn:
            return tr("Strain (%)");
        case PropertyColumn:
            switch (m_type) {
            case ModulusReduction:
                return tr("G/Gmax");
            case Damping:
                return tr("Damping (%)");
            }
        }
    case Qt::Vertical:
        return section + 1;
    }

    return QVariant();
}


const QVector<double> & NonlinearProperty::strain() const
{
    return m_strain;
}

const QVector<double> & NonlinearProperty::average() const
{
    return m_average;
}

const QVector<double> & NonlinearProperty::varied() const
{    
    return m_varied;
}

void NonlinearProperty::setVaried(const QVector<double> &varied)
{
    m_varied = varied;
    gsl_interp_accel_reset(m_acc);
}


void NonlinearProperty::initialize()
{
    if (m_strain.size() > 2) {
        if (m_interp)
            gsl_interp_free(m_interp);

        // Remove values that have the same x
        for (int i = 0; i < m_strain.size(); ++i) {
            int j = i + 1;

            while (j < m_strain.size()) {
                if (fabs(m_strain.at(i) - m_strain.at(j)) <= DBL_EPSILON) {
                    m_strain.remove(j);
                    m_varied.remove(j);
                } else {
                    ++j;
                }
            }
        }

        m_interp = gsl_interp_alloc(gsl_interp_linear, m_strain.size());
        gsl_interp_init(m_interp, m_strain.data(), m_varied.data(), m_strain.size());
        gsl_interp_accel_reset(m_acc);
    }
}

NonlinearProperty *NonlinearProperty::duplicate() const
{
    return new NonlinearProperty(m_name, m_type, m_strain, m_average);
}

QDataStream & operator<< (QDataStream & out, const NonlinearProperty* np)
{
    out << (quint8)1;

    out
            << np->m_name
            << (int)np->m_type
            << np->m_strain
            << np->m_average;

    return out;
}

QDataStream & operator>> (QDataStream & in, NonlinearProperty* np)
{
    quint8 ver;
    in >> ver;

    np->beginResetModel();

    int type;

    in >> np->m_name
            >> type
            >> np->m_strain
            >> np->m_average;

    np->m_type = (NonlinearProperty::Type)type;
    np->m_varied = np->m_average;
    np->initialize();

    np->endResetModel();

    return in;
}
