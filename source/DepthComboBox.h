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

#ifndef DEPTH_COMBO_BOX_H_
#define DEPTH_COMBO_BOX_H_

#include <QComboBox>

//! ComboBox used to specify a location either a depth or Bedrock.

class DepthComboBox : public QComboBox
{
    Q_OBJECT

    public:
        DepthComboBox( QWidget * parent = 0 );

        double depth() const;
        void setDepth(double depth);

    signals:
        void depthChanged(double depth);

    protected slots:
        void updateEditable(int index);
        void toDouble(const QString & string);
};
#endif
