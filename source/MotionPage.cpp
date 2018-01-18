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

#include "MotionPage.h"

#include "CompatibleRvtMotion.h"
#include "CompatibleRvtMotionDialog.h"
#include "DepthComboBox.h"
#include "MotionLibrary.h"
#include "MotionTypeDelegate.h"
#include "MyTableView.h"
#include "RvtMotion.h"
#include "RvtMotionDialog.h"
#include "SourceTheoryRvtMotion.h"
#include "SourceTheoryRvtMotionDialog.h"
#include "SoilProfile.h"
#include "SiteResponseModel.h"
#include "TimeSeriesMotion.h"
#include "TimeSeriesMotionDialog.h"

#include <QBuffer>
#include <QDataStream>
#include <QDebug>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QTableView>
#include <QVBoxLayout>

MotionPage::MotionPage(QWidget * parent, Qt::WindowFlags f)
    : AbstractPage(parent,f), _readOnly(false)
{   
    // Set the layout of the widget
    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(createInputLocationGroupBox());
    layout->addWidget(createMotionsTableGroupBox(), 1);

    setLayout(layout);
}

void MotionPage::setModel(SiteResponseModel *model)
{
    _motionLibrary = model->motionLibrary();

    _depthComboBox->setDepth(model->siteProfile()->inputDepth());
    connect(_depthComboBox, SIGNAL(depthChanged(double)),
            model->siteProfile(), SLOT(setInputDepth(double)));

    _tableView->setModel(_motionLibrary);
    connect(_tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateButtons()));
    _tableView->resizeColumnsToContents();
    _tableView->resizeRowsToContents();

    setApproach(_motionLibrary->approach());
    connect(_motionLibrary, SIGNAL(approachChanged(int)),
            this, SLOT(setApproach(int)));
}

void MotionPage::setReadOnly(bool readOnly)
{
    _readOnly = readOnly;
    _depthComboBox->setDisabled(readOnly);
    _tableView->setReadOnly(readOnly);

    _addButton->setHidden(readOnly);
    _removeButton->setHidden(readOnly);
    _importButton->setHidden(readOnly);
    _editButton->setText(readOnly ? tr("View") : tr("Edit"));
}

void MotionPage::setApproach(int i)
{
    _tableView->setColumnHidden(
            MotionLibrary::ScaleColumn,
            i == MotionLibrary::RandomVibrationTheory);
}

QGroupBox* MotionPage::createInputLocationGroupBox()
{
    QHBoxLayout* layout = new QHBoxLayout;

    layout->addWidget(new QLabel(tr("Specify the location to input the motion(s):")));

    _depthComboBox = new DepthComboBox;
    layout->addWidget(_depthComboBox);
    layout->addStretch(1);

    _inputLocationGroupBox = new QGroupBox(tr("Motion Input Location"));
    _inputLocationGroupBox->setLayout(layout);

    return _inputLocationGroupBox;
}

QGroupBox* MotionPage::createMotionsTableGroupBox()
{
    QVBoxLayout *layout = new QVBoxLayout;
    // Create the buttons
    QHBoxLayout *buttonRow = new QHBoxLayout;

    _addButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
    connect(_addButton, SIGNAL(clicked()),
            this, SLOT(add()));
    buttonRow->addWidget(_addButton);

    _removeButton = new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
    _removeButton->setEnabled(false);
    connect(_removeButton, SIGNAL(clicked()),
            this, SLOT(remove()));
    buttonRow->addWidget(_removeButton);

    _editButton = new QPushButton;
    _editButton->setEnabled(false);
    connect(_editButton, SIGNAL(clicked()),
            this, SLOT(edit()));
    buttonRow->addWidget(_editButton);

    _importButton = new QPushButton(QIcon(":/images/document-import.svg"), tr("Load Suite"));
    connect(_importButton, SIGNAL(clicked()),
            this, SLOT(importSuite()));

    buttonRow->addStretch(1);
    buttonRow->addWidget(_importButton);

    layout->addLayout(buttonRow);

    // Create table
    _tableView = new MyTableView(this);
    _tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    _tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _tableView->setItemDelegateForColumn(2, new MotionTypeDelegate);

    connect(_tableView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(edit()));
    layout->addWidget(_tableView, 1);

    // Form the groupbox
    _motionsTableGroupBox = new QGroupBox(tr("Input Motions"));
    _motionsTableGroupBox->setLayout(layout);

    return _motionsTableGroupBox;
}

void MotionPage::add()
{
    QDialog *dialog = 0;
    AbstractMotion *motion = 0;

    switch (_motionLibrary->approach()) {
    case MotionLibrary::TimeSeries:
        {
            TimeSeriesMotion *_motion = new TimeSeriesMotion(_motionLibrary);
            _motion->setSaveData(_motionLibrary->saveData());

            dialog = (QDialog*)(new TimeSeriesMotionDialog(_motion, _readOnly, this));
            motion = (AbstractMotion*)_motion;
            break;
        }
    case MotionLibrary::RandomVibrationTheory:
        {
            // Prompt for type of motions
            QStringList items = QStringList()
                                << tr("User Defined FAS")
                                << tr("Response Spectrum Compatible")
                                << tr("Seismological Source Theory");
            bool ok;
            QString item = QInputDialog::getItem(this, tr("RVT Motion Type"),
                                                 tr("Select method for defining RVT motion:"), items, 0, false, &ok);

            if (!ok) {
                return;
            }

            switch (items.indexOf(item)) {
            case 0:
                {
                    // User defined FAS
                    RvtMotion *_motion = new RvtMotion(_motionLibrary);
                    dialog = (QDialog*)(new RvtMotionDialog(_motion, _readOnly, this));
                    motion = (AbstractMotion*)_motion;
                    break;
                }
            case 1:
                {
                    // Response Spectrum Compatible
                    CompatibleRvtMotion *_motion = new CompatibleRvtMotion(_motionLibrary);
                    dialog = (QDialog*)(new CompatibleRvtMotionDialog(_motion, _readOnly, this));
                    motion = (AbstractMotion*)_motion;
                    break;
                }
            case 2:
                {
                    // User defined FAS
                    SourceTheoryRvtMotion *_motion = new SourceTheoryRvtMotion(_motionLibrary);
                    dialog = (QDialog*)(new SourceTheoryRvtMotionDialog(_motion, _readOnly, this));
                    motion = (AbstractMotion*)_motion;
                    break;
                }
            }
        }
    }

    if (dialog->exec()) {
        _motionLibrary->addMotion(motion);
        _tableView->resizeColumnsToContents();
        _tableView->resizeRowsToContents();
    } else {
        // FIXME deleting the motions causes a segfault!
        motion->deleteLater();
    }

    delete dialog;
}

void MotionPage::remove()
{
    QModelIndexList selectedRows = _tableView->selectionModel()->selectedRows();
    _motionLibrary->removeRows(selectedRows.first().row(), selectedRows.size());
}

void MotionPage::importSuite()
{
    QSettings settings;
    // Prompt for the file to load
    QString fileName =  QFileDialog::getOpenFileName(
            this,
            tr("Select suite file... - Strata"),
            settings.value("suiteDirectory",
                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                           ).toString(),
            "Strata Suite File (*.csv)");


    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        QDir dir = fileInfo.dir();

        // Save the path
        settings.setValue("suiteDirectory", fileInfo.filePath());

        // Load the file
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() <<  "Unable to open file:" << fileName;
            return;
        }

        // Clear the previous motions
        const int rowCount = _motionLibrary->rowCount();

        if (rowCount)
            _motionLibrary->removeRows(0, rowCount);

        // Load each of the filename/scale pairs in a map
        QMap<QString, QStringList> map;

        QTextStream stream(&file);
        QString line = stream.readLine();
        while (!line.isNull()) {
            if (!line.startsWith("#")) {
                QStringList args = line.split(",");
                if (args.size() < 1 || args.size() > 3) {
                    qCritical() <<  tr("Invalid file format on line:") << line;
                    return;
                }

                const QString _fileName = args.takeFirst();

                if (args.size() > 0) {
                    // First argument is the scale that is applied to the motion
                    // Test to see if we can parse the scale
                    bool ok = false;
                    double d = args.at(0).toDouble(&ok);
                    Q_UNUSED(d);

                    if (!ok) {
                        qCritical() << "Error converting" << args.at(0) << "to a number!\n"
                                << "Assuming a scale of 1.0.";

                        args[0] = "1.";
                    }
                } else {
                    // Assume a scale of 1.
                    args << "1.";
                }

                if (args.size() > 1) {
                    bool ok;

                    AbstractMotion::Type type = AbstractMotion::variantToType(args.at(1), &ok);
                    Q_UNUSED(type);

                    if (!ok) {
                        qCritical() << "Unknown type specified:" << args.at(2)
                                << ". Valid options are 'Outcrop'"
                                << "\nAssuming 'Outcrop'";
                        args[1] = AbstractMotion::typeList().first();
                    }
                } else {
                    // Assume Outcrop
                    args <<  AbstractMotion::typeList().first();
                }

                map.insertMulti(dir.absoluteFilePath(_fileName), args);
            }
            line = stream.readLine();
        }

        QProgressDialog progressDialog(tr("Loading motion suite..."), tr("Abort"), 1, map.size(), this);
        progressDialog.setWindowModality(Qt::WindowModal);

        QMapIterator<QString, QStringList> i(map);
        while (i.hasNext() && !progressDialog.wasCanceled()) {
            i.next();

            if (_motionLibrary->approach() == MotionLibrary::TimeSeries) {
                // Load the motion
                bool successful = true;
                double scale = i.value().at(0).toDouble();
                AbstractMotion::Type type = AbstractMotion::variantToType(i.value().at(1), &successful);

                TimeSeriesMotion* tsm = new TimeSeriesMotion(i.key(), scale, type, &successful);

                if (successful)
                    _motionLibrary->addMotion(tsm);
            } else {
                double scale = i.value().at(0).toDouble();
                AbstractRvtMotion* arm = loadRvtMotionFromTextFile(i.key(), scale);

                if (arm)
                    _motionLibrary->addMotion(arm);
            }

            progressDialog.setValue(_motionLibrary->rowCount());
        }
    }

    _tableView->resizeColumnsToContents();
    _tableView->resizeRowsToContents();
}

void MotionPage::edit()
{
    const QModelIndex& index = _tableView->currentIndex();
    AbstractMotion *motion = _motionLibrary->motionAt(index.row());

    // Buffer to save the state of the motion
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream dataStream(&buffer);

    // Save the current state of the motion
    dataStream << motion;

    QDialog* dialog = 0;

    if (TimeSeriesMotion* tsm = qobject_cast<TimeSeriesMotion*>(motion)) {
        dialog = new TimeSeriesMotionDialog(tsm, _readOnly, this);
    } else if (RvtMotion* rm = qobject_cast<RvtMotion*>(motion)) {
        dialog = new RvtMotionDialog(rm, _readOnly, this);
    } else if (CompatibleRvtMotion* crm = qobject_cast<CompatibleRvtMotion*>(motion)) {
        dialog = new CompatibleRvtMotionDialog(crm, _readOnly, this);
    } else if (SourceTheoryRvtMotion* _motion = qobject_cast<SourceTheoryRvtMotion*>(motion)) {
        dialog = new SourceTheoryRvtMotionDialog(_motion, _readOnly, this);
    }

    if (!dialog->exec()) {
        buffer.seek(0);
        // Revert to the previous state
        dataStream >> motion;
        _motionLibrary->updateRow(index.row());
    }

    dialog->deleteLater();
}

void MotionPage::updateButtons()
{
    bool enabled = !_tableView->selectionModel()->selectedRows().isEmpty();

    _removeButton->setEnabled(enabled);
    _editButton->setEnabled(enabled);
}
