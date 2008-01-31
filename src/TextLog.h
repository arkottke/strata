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

#ifndef TEXT_LOG_H_
#define TEXT_LOG_H_

#include <QTextEdit>    
#include <QMap>
#include <QString>
#include <QVariant>

class TextLog : public QTextEdit
{
    Q_OBJECT

    public:
        TextLog( QWidget * parent = 0 );
        
        //! Level of detail of the logging
        enum Level {
            Low, //!< Print out of the input and the progress of the calculation
            Medium, //!< Low plus the results of each run of the calculation
            High //!< Results for each iteration of the calculation
        };
        
        static QStringList levelList();
        
        //! Reset the object to the default values
        void reset();

        Level level() const;
        void setLevel(Level level);

		QMap<QString, QVariant> toMap() const;
		void fromMap(const QMap<QString, QVariant> & map);

    private:
        //! Level of detail in the log
        Level m_level;
};
        
TextLog & operator<< ( TextLog & log, const QString & string );

#endif
