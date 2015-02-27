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

#include <boost/lexical_cast.hpp>

TextLog::TextLog(QObject * parent) :
        QObject(parent)

{
    m_level = Low;
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

TextLog::Level TextLog::level() const
{
    return m_level;
}

void TextLog::setLevel( TextLog::Level level )
{
    if (m_level != level) {
        m_level = level;

        emit wasModified();
    }
}

void TextLog::setLevel(int level)
{
    setLevel((Level)level);
}

const QStringList& TextLog::text() const
{
    return m_text;
}

TextLog & operator<<( TextLog & log, const QString & string )
{
    log.append(string);
    return log;
}

void TextLog::ptRead(const ptree &pt)
{
    m_level = (TextLog::Level) pt.get<int>("level");
    m_text.clear();
    foreach (const ptree::value_type & v, pt.get_child("text"))
    {
        QString t = QString::fromStdString(boost::lexical_cast<std::string>(v.second.data()));
        m_text.append(t);
    }
}

void TextLog::ptWrite(ptree &pt) const
{
    pt.put("level", (int) m_level);
    ptree text;
    foreach (const QString & line, m_text)
    {
        ptree val;
        val.put("", line.toStdString());
        text.push_back(std::make_pair("", val));
    }
    pt.put_child("text", text);
}

QDataStream & operator<< (QDataStream & out, const TextLog* tl)
{
    out << (quint8)1;

    out << (int)tl->m_level << tl->m_text;

    return out;
}

QDataStream & operator>> (QDataStream & in, TextLog* tl)
{
    quint8 ver;
    in >> ver;

    int level;

    in >> level >> tl->m_text;

    tl->m_level = (TextLog::Level)level;

    return in;
}
