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



#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QList>
#include <QString>
#include <QDialog>

#include <HexPlugin.h>

#include "Filter.h"


class QPushButton;
class QComboBox;


class FilterDialog : public QDialog {
	Q_OBJECT

public:
	FilterDialog(HexCursor *cur, HexTextCursor *textCur, HexEd *ed, QWidget *parent = 0);
	~FilterDialog();

	HexDocument *document() {
		return cursor()->document();
	}

	HexCursor *cursor() {
		return mCursor;
	}

	HexTextCursor *textCursor() {
		return mTextCursor;
	}

private slots:
	void filter();

	void addFilter();
	void delFilter();
	void editFilter();

private:
	HexCursor *mCursor;
	HexTextCursor *mTextCursor;

	QList<Filter> filters;
	QComboBox *filtersBox;

	QPushButton *addButton;
	QPushButton *delButton;
	QPushButton *editButton;

	QPushButton *filterButton;
	QPushButton *okButton;

	QString lastUsedFilter;

	HexEd *mEd;
};

#endif
