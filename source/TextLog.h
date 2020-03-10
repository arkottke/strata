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

#ifndef TEXT_LOG_H_
#define TEXT_LOG_H_

#include <QDataStream>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>


class TextLog : public QObject
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const TextLog* tl) -> QDataStream &;
    friend auto operator>> (QDataStream & in, TextLog* tl) -> QDataStream &;

    public:
    explicit TextLog(QObject *parent = nullptr);
        
        //! Level of detail of the logging
        enum Level {
            Low, //!< Print out of the input and the progress of the calculation
            Medium, //!< Low plus the results of each run of the calculation
            High //!< Results for each iteration of the calculation
        };
        
        static auto levelList() -> QStringList;

        //! Append text to the log
        void append(const QString & text);
        
        //! Clear the log
        void clear();

        auto level() const -> Level;
        void setLevel(Level level);

        auto text() const -> const QStringList&;

        void fromJson(const QJsonObject &json);
        auto toJson() const -> QJsonObject;


    public slots:
        void setLevel(int level);

    signals:
        void textCleared();
        void textChanged(const QString & text);

        void wasModified();

    private:
        //! Level of detail in the log
        Level _level;

        //! Text
        QStringList _text;
};
        
auto operator<< ( TextLog & log, const QString & string ) -> TextLog &;

#endif
