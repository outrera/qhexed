/*
 * QHexEd - Simple Qt Based Hex Editor
 *
 * Copyright (C) 2007 Nikita Sadkov
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This code is free software.  You may redistribute copies of it under the terms of
 * the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.
 * There is NO WARRANTY, to the extent permitted by law.
*/


#include <QtGui>

#include <HexPlugin.h>

#include "InspectorModel.h"
#include "InspectorWidget.h"

InspectorWidget::InspectorWidget(HexEd *ed, QWidget *parent, Qt::WindowFlags f)
		: QFrame(parent, f) {
	setFrameStyle(QFrame::Panel|QFrame::Raised);
	setLineWidth(1);

	QSettings settings;
	settings.beginGroup("X86QuickDisassemblerPanel");
	m32bitMode = settings.value("m32bitMode", true).toBool();
	settings.endGroup();

	mModel = new InspectorModel(this);
	QTableView *mTableView = new QTableView(this);
	//mTableView->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	//mTableView->setColumnWidth(0, 8);
	mTableView->verticalHeader()->hide();
	mTableView->verticalScrollBar()->hide();
	//mTableView->horizontalHeader()->hide();
	//mTableView->horizontalScrollBar()->hide();
	mTableView->setShowGrid(false);
	mTableView->setAlternatingRowColors(true);
	mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	//mTableView->setFrameStyle(QFrame::NoFrame);
	//mTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//mTableView->QFrame::setLineWidth(0);
	mTableView->setModel(mModel);


	QFont font = mTableView->font();
	font.setPointSize(8);
	QFontMetrics fontMetrics(font);
	mTableView->setFont(font);
	mTableView->resizeColumnToContents(1);
	mTableView->resizeRowsToContents();
	//mTableView->horizontalHeader()->setMaximumHeight(mTableView->verticalHeader()->fontMetrics().height());
	mTableView->verticalHeader()->setResizeMode(QHeaderView::Custom);
	for(int i = 0; i < 20; i++)
		mTableView->setRowHeight(i, fontMetrics.ascent()*3/2);

	mTableView->horizontalHeader()->setResizeMode(0, QHeaderView::Interactive);
	mTableView->horizontalHeader()->setResizeMode(1, QHeaderView::Interactive);
	mTableView->horizontalHeader()->setResizeMode(2, QHeaderView::Interactive);
	mTableView->horizontalHeader()->setResizeMode(3, QHeaderView::Stretch);
	mTableView->setSelectionMode(QAbstractItemView::NoSelection);

	mToggleModeAction = new QAction("X86QuickDis 32bit Mode", this);
	mToggleModeAction->setCheckable(true);
	mToggleModeAction->setChecked(m32bitMode);
	connect(mToggleModeAction, SIGNAL(toggled(bool)), this, SLOT(disModeChanged(bool)));
	ed->addAction(HexToolAction, mToggleModeAction);
	mModel->set32bitMode(m32bitMode);

	mLayout = new QVBoxLayout(this);
	mLayout->addWidget(mTableView);
	mLayout->setSpacing(0);
	mLayout->setMargin(0);

	focusChanged(0);
}

InspectorWidget::~InspectorWidget() {
	QSettings settings;
	settings.beginGroup("X86QuickDisassemblerPanel");
	settings.setValue("m32bitMode", m32bitMode);
	settings.endGroup();
}

void InspectorWidget::disModeChanged(bool state) {
	m32bitMode = state;
	mModel->set32bitMode(m32bitMode);
	mModel->updateModel();
}

void InspectorWidget::focusChanged(HexCursor *cur) {
	mCursor = cur;
	mModel->setCursor(cursor());
	//mTableView->resizeRowsToContents();
}
