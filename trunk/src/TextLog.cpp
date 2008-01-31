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

#include "TextLog.h"
#include <QApplication>

TextLog::TextLog( QWidget * parent )
    : QTextEdit(parent)
{
    reset();
}

QStringList TextLog::levelList()
{
    QStringList list;
    list << tr("Low")
        << tr("Medium")
        << tr("High");

    return list;
}

void TextLog::reset()
{
    setTabStopWidth(20);
    setReadOnly(true);
    m_level = Low;
}

TextLog::Level TextLog::level() const
{
    return m_level;
}

void TextLog::setLevel( TextLog::Level level )
{
    m_level = level;
}

QMap<QString, QVariant> TextLog::toMap() const
{
	QMap<QString, QVariant> map;

	map.insert("level", m_level);

	return map;
}

void TextLog::fromMap(const QMap<QString, QVariant> & map )
{
	m_level = (Level)map.value("level").toInt();
}

TextLog & operator<<( TextLog & log, const QString & string )
{
    log.append(string);
   
    // Update the log
    QApplication::processEvents();

    return log;
}
