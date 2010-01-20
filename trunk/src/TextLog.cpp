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

#include <QDebug>

TextLog::TextLog( QObject * parent )
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

void TextLog::append(const QString & text)
{
    m_text << text;
    emit textChanged(text); 
    emit wasModified();
}

void TextLog::clear()
{
    m_text.clear();
    emit textCleared();
}

void TextLog::reset()
{
    clear();
    m_level = Low;
}

TextLog::Level TextLog::level() const
{
    return m_level;
}

void TextLog::setLevel( TextLog::Level level )
{
    if ( m_level != level ) {
        emit wasModified();
    }

    m_level = level;
}

void TextLog::setLevel(int level)
{
    setLevel((TextLog::Level)level);
}

QMap<QString, QVariant> TextLog::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("level", m_level);
    map.insert("text", m_text);

    return map;
}

void TextLog::fromMap(const QMap<QString, QVariant> & map )
{
    m_level = (Level)map.value("level").toInt();

    QList<QVariant> list = map.value("text").toList();

    m_text.clear();
    emit textCleared();

    QString text;

    foreach( QVariant var, list ) {
        emit textChanged(var.toString());
        m_text << var.toString();
    }
}

TextLog & operator<<( TextLog & log, const QString & string )
{
    log.append(string);
    return log;
}
