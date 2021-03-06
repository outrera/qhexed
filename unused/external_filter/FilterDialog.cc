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
#include "FilterDialog.h"
#include "FilterEditDialog.h"


FilterDialog::FilterDialog(HexCursor *cur, HexTextCursor *textCur, HexEd *ed, QWidget *parent)
	: QDialog(parent), mCursor(cur), mTextCursor(textCur), mEd(ed)
{
	setWindowTitle(tr("Filter Setup"));

	filtersBox = new QComboBox(this);

	QSettings settings("exanode", "ExternalFilterPlugin");
	settings.beginGroup("ExternalFilters");
	filters = settings.value("filterList", QVariant::fromValue(QList<Filter>()) ).value<QList<Filter> >();
	lastUsedFilter = settings.value("lastUsedFilter", QString()).toString();
	settings.endGroup();

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(filtersBox);

	////////////////////////////////
	QHBoxLayout *filterListButtons = new QHBoxLayout;
	mainLayout->addLayout(filterListButtons);
	filterListButtons->addStretch();
	addButton = new QPushButton(tr("&Add"), this);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addFilter()));
	filterListButtons->addWidget(addButton);

	editButton = new QPushButton(tr("&Edit"), this);
	connect(editButton, SIGNAL(clicked()), this, SLOT(editFilter()));
	filterListButtons->addWidget(editButton);
	filterListButtons->addStretch();


	QHBoxLayout *delButtonLayout = new QHBoxLayout;
	mainLayout->addLayout(delButtonLayout);
	delButtonLayout->addStretch();
	delButton = new QPushButton(tr("&Delete"), this);
	connect(delButton, SIGNAL(clicked()), this, SLOT(delFilter()));
	delButtonLayout->addWidget(delButton);
	delButtonLayout->addStretch();

	////////////////////////////////
	mainLayout->addStretch();

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	mainLayout->addLayout(buttonLayout);
	buttonLayout->addStretch();

	filterButton = new QPushButton(tr("&Filter"), this);
	connect(filterButton, SIGNAL(clicked()), this, SLOT(filter()));
	buttonLayout->addWidget(filterButton);

	QPushButton *cancelButton = new QPushButton(tr("&Cancel"), this);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	buttonLayout->addWidget(cancelButton);

	buttonLayout->addStretch();

	int index = 0;
	foreach(Filter filter, filters) {
		filtersBox->addItem(filter.name);
		if(filter.name == lastUsedFilter)
			filtersBox->setCurrentIndex(index);
		index++;
	}
	
}

FilterDialog::~FilterDialog() {
	if(filtersBox->count())
		lastUsedFilter = filtersBox->currentText();
	else
		lastUsedFilter = "";

	QSettings settings("exanode", "ExternalFilterPlugin");
	settings.beginGroup("ExternalFilters");
	settings.setValue("filterList", QVariant::fromValue(filters));
	settings.setValue("lastUsedFilter", lastUsedFilter);
	settings.endGroup();
}

void FilterDialog::filter() {
	if(filtersBox->currentIndex() >= filtersBox->count()) {
		QMessageBox::warning(
			qApp->activeWindow(),
			tr("No Filter Selected!"),
			tr("You should first select existing filter or add a new one.")
		);
		return;
	}

	QString name = filtersBox->currentText();
	Filter filter;

	foreach(filter, filters) {
		if(filter.name == name)
			break;
	}

	if(!cursor() && !textCursor()) {
		QMessageBox::warning(
			qApp->activeWindow(),
			tr("Nothing to filter!"),
			tr("No underlaying document in focus.")
		);
		return;
	}

	if(!textCursor() && filter.textInput) {
		QMessageBox::warning(
			qApp->activeWindow(),
			tr("Invalid filter!"),
			tr("Selected filter requires text input.")
		);
		return;
	}

	if(!cursor() && !filter.textInput) {
		QMessageBox::warning(
			qApp->activeWindow(),
			tr("Invalid filter!"),
			tr("Selected filter requires binary input.")
		);
		return;
	}

	filter.execute(cursor(), textCursor(), mEd);
	accept();
}

void FilterDialog::addFilter() {
	Filter filter;

again:
	if(filter.popupEditDialog() == Rejected)
		return;

	foreach(Filter f, filters) {
		if(filter.name == f.name) {
			QMessageBox::warning(
				qApp->activeWindow(),
				tr("Invalid name!"),
				tr("Filter with this name already exist.")
			);
			goto again;
		}
	}

	filters.append(filter);
	filtersBox->addItem(filter.name);
}

void FilterDialog::editFilter() {
	for(int i = 0; i < filters.size(); i++)
		if(filters[i].name == filtersBox->currentText()) {
			filters[i].popupEditDialog();
			break;
		}
}

void FilterDialog::delFilter() {
	for(int i = 0; i < filters.size(); i++)
		if(filters[i].name == filtersBox->currentText()) {
			filters.removeAt(i);
			filtersBox->removeItem(filtersBox->currentIndex());
			break;
		}
}
