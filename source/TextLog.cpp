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

#include "TextLog.h"

#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>

TextLog::TextLog(QObject * parent) :
        QObject(parent), _level(TextLog::Medium)

{
}

auto TextLog::levelList() -> QStringList
{
    QStringList list = {tr("Low"), tr("Medium"), tr("High")};
    return list;
}

void TextLog::append(const QString & text)
{
    _text << text;
    emit textChanged(text); 
    emit wasModified();
}

void TextLog::clear()
{
    _text.clear();
    emit textCleared();
}

auto TextLog::level() const -> TextLog::Level
{
    return _level;
}

void TextLog::setLevel( TextLog::Level level )
{
    if (_level != level) {
        _level = level;

        emit wasModified();
    }
}

void TextLog::setLevel(int level)
{
    setLevel((Level)level);
}

auto TextLog::text() const -> const QStringList&
{
    return _text;
}

auto operator<<( TextLog & log, const QString & string ) -> TextLog &
{
    log.append(string);
    return log;
}

void TextLog::fromJson(const QJsonObject &json)
{
    _level = (TextLog::Level) json["level"].toInt();

    _text.clear();
    for (const QJsonValue &v : json["text"].toArray())
        _text << v.toString();
}

auto TextLog::toJson() const -> QJsonObject
{
    QJsonObject json;
    json["level"] = (int) _level;

    QJsonArray text;
    for (const QString &l : _text)
        text << QJsonValue(l);
    json["text"] = text;
    return json;
}


auto operator<< (QDataStream & out, const TextLog* tl) -> QDataStream &
{
    out << (quint8)1;

    out << (int)tl->_level << tl->_text;

    return out;
}

auto operator>> (QDataStream & in, TextLog* tl) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    int level;

    in >> level >> tl->_text;

    tl->_level = (TextLog::Level)level;

    return in;
}
