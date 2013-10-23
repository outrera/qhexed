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



#ifndef FILTEREDITDIALOG_H
#define FILTEREDITDIALOG_H

#include <QDialog>
#include <QtGui>

#include <HexPlugin.h>

#include "Filter.h"

class LabeledValue : public QWidget {
	Q_OBJECT
public:
	LabeledValue(QString label, QString value="", QWidget *parent=0);

	QLineEdit *lineEdit() {
		return mLineEdit;
	}

public slots:
	void setValue(QString value);
	void textChanged(const QString & text);

signals:
	void changed();

private:
	QLineEdit *mLineEdit;
	QLabel *mLabel;
};

class LabeledCheckBox : public QWidget {
	Q_OBJECT
public:
	LabeledCheckBox(QString label, bool checked=true, QWidget *parent=0);

	bool checked();

signals:
	void stateChanged(bool newState);
	void changed();

private slots:
	void stateHandler(int state);

private:
	QLabel *mLabel;
	QCheckBox *mCheckBox;
};


class FilterEditDialog : public QDialog {
	Q_OBJECT

public:
	FilterEditDialog(Filter *f);
	~FilterEditDialog();

private slots:
	void updateFilter();

private:
	Filter *mFilter;
	LabeledValue *nameLV;
	LabeledValue *commandLV;
	LabeledValue *commentLV;
	LabeledCheckBox *textInputLCB;
	LabeledCheckBox *textOutputLCB;
};

#endif
