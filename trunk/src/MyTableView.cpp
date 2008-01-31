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

#include "MyTableView.h"
#include <QtAlgorithms>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

MyTableView::MyTableView( QWidget * parent)
    : QTableView(parent)
{
    // Create the context menu
    m_contextMenu = new QMenu;
    m_contextMenu->addAction(tr("Copy"), this, SLOT(copy()), QKeySequence(tr("Ctrl+c")));
    m_contextMenu->addAction(tr("Paste"), this, SLOT(paste()), QKeySequence(tr("Ctrl+v")));
}

void MyTableView::copy()
{
    QString data;
    
    // Determine the currently selected rows
    QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();

    // Sort the indexes
    qSort( selectedIndexes.begin(), selectedIndexes.end());

    for ( int i = 0; i < selectedIndexes.size(); ++i) {
        // Add the data in the cell
        data += model()->data(selectedIndexes.at(i)).toString();

        if ( i + 1 < selectedIndexes.size() ) {
            if (selectedIndexes.at(i).row() != selectedIndexes.at(i+1).row() )
                // Add a line break when the row changes
                data += "\n";
            else
                // Add a tab to separate values on the same row
                data += "\t";
        }
    }
    
    // Set the text of the clipboard
    QApplication::clipboard()->setText(data);
}

void MyTableView::paste()
{
    // Grab the text from the clipboard and split it into lines
    QStringList rows = QApplication::clipboard()->text().split(QRegExp("\\n"), QString::SkipEmptyParts);

    // Return if the row list is empty
    if (rows.size() == 0) 
        return;

    // Get the currently selected items
    QModelIndex initial = currentIndex();

    // If the number of rows is greater than the number of rows beyond the
    // current index and the required number of rows.
    if ( initial.row() < 0 ) {
        // No row selected
        model()->insertRows(model()->rowCount(), rows.size() - model()->rowCount());
        initial = model()->index(0,0);
    } else if (model()->rowCount() < initial.row() + rows.size())
        model()->insertRows(model()->rowCount(), initial.row() + rows.size() - model()->rowCount());

    // Stop the model from updating the view
    model()->blockSignals(true);
    // Set the data in the rows
    for( int i = 0; i < rows.size(); ++i )
    {
        // Split the row text into cells based on tabs
        QStringList cell = rows.at(i).split(QRegExp("\\t"), QString::SkipEmptyParts);

        for( int j = 0; j < cell.size(); ++j )
        {
            //Skip if the string is empty
            if ( cell.at(j) != "" )
                model()->setData( initial.sibling( initial.row() + i, initial.column() + j),
                        cell.at(j).trimmed(), Qt::EditRole);
        }
    }
    // Re-enable the model to send signals
    model()->blockSignals(false);

    // Signal that the data needs to be changed
    dataChanged( initial.sibling( initial.row(), initial.column() ), 
            initial.sibling( initial.row() + rows.size(), model()->columnCount()));

    emit dataPasted();
}
        
void MyTableView::contextMenuEvent( QContextMenuEvent * event )
{
    m_contextMenu->popup(event->globalPos());
}
