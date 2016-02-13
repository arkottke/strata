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
    : AbstractPage(parent,f), m_readOnly(false)
{   
    // Set the layout of the widget
    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(createInputLocationGroupBox());
    layout->addWidget(createMotionsTableGroupBox(), 1);

    setLayout(layout);
}

void MotionPage::setModel(SiteResponseModel *model)
{
    m_motionLibrary = model->motionLibrary();

    m_depthComboBox->setDepth(model->siteProfile()->inputDepth());
    connect(m_depthComboBox, SIGNAL(depthChanged(double)),
            model->siteProfile(), SLOT(setInputDepth(double)));

    m_tableView->setModel(m_motionLibrary);
    connect(m_tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateButtons()));
    m_tableView->resizeColumnsToContents();
    m_tableView->resizeRowsToContents();

    setApproach(m_motionLibrary->approach());
    connect(m_motionLibrary, SIGNAL(approachChanged(int)),
            this, SLOT(setApproach(int)));
}

void MotionPage::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    m_depthComboBox->setDisabled(readOnly);
    m_tableView->setReadOnly(readOnly);

    m_addButton->setHidden(readOnly);
    m_removeButton->setHidden(readOnly);
    m_importButton->setHidden(readOnly);
    m_editButton->setText(readOnly ? tr("View") : tr("Edit"));
}

void MotionPage::setApproach(int i)
{
    m_tableView->setColumnHidden(
            MotionLibrary::ScaleColumn,
            i == MotionLibrary::RandomVibrationTheory);
}

QGroupBox* MotionPage::createInputLocationGroupBox()
{
    QHBoxLayout* layout = new QHBoxLayout;

    layout->addWidget(new QLabel(tr("Specify the location to input the motion(s):")));

    m_depthComboBox = new DepthComboBox;
    layout->addWidget(m_depthComboBox);
    layout->addStretch(1);

    m_inputLocationGroupBox = new QGroupBox(tr("Motion Input Location"));
    m_inputLocationGroupBox->setLayout(layout);

    return m_inputLocationGroupBox;
}

QGroupBox* MotionPage::createMotionsTableGroupBox()
{
    QVBoxLayout *layout = new QVBoxLayout;
    // Create the buttons
    QHBoxLayout *buttonRow = new QHBoxLayout;

    m_addButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
    connect(m_addButton, SIGNAL(clicked()),
            this, SLOT(add()));
    buttonRow->addWidget(m_addButton);

    m_removeButton = new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
    m_removeButton->setEnabled(false);
    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(remove()));
    buttonRow->addWidget(m_removeButton);

    m_editButton = new QPushButton;
    m_editButton->setEnabled(false);
    connect(m_editButton, SIGNAL(clicked()),
            this, SLOT(edit()));
    buttonRow->addWidget(m_editButton);

    m_importButton = new QPushButton(QIcon(":/images/document-import.svg"), tr("Load Suite"));
    connect(m_importButton, SIGNAL(clicked()),
            this, SLOT(importSuite()));

    buttonRow->addStretch(1);
    buttonRow->addWidget(m_importButton);

    layout->addLayout(buttonRow);

    // Create table
    m_tableView = new MyTableView(this);
    m_tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setItemDelegateForColumn(2, new MotionTypeDelegate);

    connect(m_tableView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(edit()));
    layout->addWidget(m_tableView, 1);

    // Form the groupbox
    m_motionsTableGroupBox = new QGroupBox(tr("Input Motions"));
    m_motionsTableGroupBox->setLayout(layout);

    return m_motionsTableGroupBox;
}

void MotionPage::add()
{
    QDialog *dialog = 0;
    AbstractMotion *motion = 0;

    switch (m_motionLibrary->approach()) {
    case MotionLibrary::TimeSeries:
        {
            TimeSeriesMotion *_motion = new TimeSeriesMotion(m_motionLibrary);
            _motion->setSaveData(m_motionLibrary->saveData());

            dialog = (QDialog*)(new TimeSeriesMotionDialog(_motion, m_readOnly, this));
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
                    RvtMotion *_motion = new RvtMotion(m_motionLibrary);
                    dialog = (QDialog*)(new RvtMotionDialog(_motion, m_readOnly, this));
                    motion = (AbstractMotion*)_motion;
                    break;
                }
            case 1:
                {
                    // Response Spectrum Compatible
                    CompatibleRvtMotion *_motion = new CompatibleRvtMotion(m_motionLibrary);
                    dialog = (QDialog*)(new CompatibleRvtMotionDialog(_motion, m_readOnly, this));
                    motion = (AbstractMotion*)_motion;
                    break;
                }
            case 2:
                {
                    // User defined FAS
                    SourceTheoryRvtMotion *_motion = new SourceTheoryRvtMotion(m_motionLibrary);
                    dialog = (QDialog*)(new SourceTheoryRvtMotionDialog(_motion, m_readOnly, this));
                    motion = (AbstractMotion*)_motion;
                    break;
                }
            }
        }
    }

    if (dialog->exec()) {
        m_motionLibrary->addMotion(motion);
        m_tableView->resizeColumnsToContents();
        m_tableView->resizeRowsToContents();
    } else {
        // FIXME deleting the motions causes a segfault!
        motion->deleteLater();
    }

    delete dialog;
}

void MotionPage::remove()
{
    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows();
    m_motionLibrary->removeRows(selectedRows.first().row(), selectedRows.size());
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
        const int rowCount = m_motionLibrary->rowCount();

        if (rowCount)
            m_motionLibrary->removeRows(0, rowCount);

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

            if (m_motionLibrary->approach() == MotionLibrary::TimeSeries) {
                // Load the motion
                bool successful = true;
                double scale = i.value().at(0).toDouble();
                AbstractMotion::Type type = AbstractMotion::variantToType(i.value().at(1), &successful);

                TimeSeriesMotion* tsm = new TimeSeriesMotion(i.key(), scale, type, &successful);

                if (successful)
                    m_motionLibrary->addMotion(tsm);
            } else {
                double scale = i.value().at(0).toDouble();
                AbstractRvtMotion* arm = loadRvtMotionFromTextFile(i.key(), scale);

                if (arm)
                    m_motionLibrary->addMotion(arm);
            }

            progressDialog.setValue(m_motionLibrary->rowCount());
        }
    }

    m_tableView->resizeColumnsToContents();
    m_tableView->resizeRowsToContents();
}

void MotionPage::edit()
{
    const QModelIndex& index = m_tableView->currentIndex();
    AbstractMotion *motion = m_motionLibrary->motionAt(index.row());

    // Buffer to save the state of the motion
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream dataStream(&buffer);

    // Save the current state of the motion
    dataStream << motion;

    QDialog* dialog = 0;

    if (TimeSeriesMotion* tsm = qobject_cast<TimeSeriesMotion*>(motion)) {
        dialog = new TimeSeriesMotionDialog(tsm, m_readOnly, this);
    } else if (RvtMotion* rm = qobject_cast<RvtMotion*>(motion)) {
        dialog = new RvtMotionDialog(rm, m_readOnly, this);
    } else if (CompatibleRvtMotion* crm = qobject_cast<CompatibleRvtMotion*>(motion)) {
        dialog = new CompatibleRvtMotionDialog(crm, m_readOnly, this);
    } else if (SourceTheoryRvtMotion* _motion = qobject_cast<SourceTheoryRvtMotion*>(motion)) {
        dialog = new SourceTheoryRvtMotionDialog(_motion, m_readOnly, this);
    }

    if (!dialog->exec()) {
        buffer.seek(0);
        // Revert to the previous state
        dataStream >> motion;
        m_motionLibrary->updateRow(index.row());
    }

    dialog->deleteLater();
}

void MotionPage::updateButtons()
{
    bool enabled = !m_tableView->selectionModel()->selectedRows().isEmpty();

    m_removeButton->setEnabled(enabled);
    m_editButton->setEnabled(enabled);
}
