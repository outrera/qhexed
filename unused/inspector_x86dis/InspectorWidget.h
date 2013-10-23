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


#ifndef INSPECTORWIDGET_H
#define INSPECTORWIDGET_H

#include <QFrame>
#include <HexPlugin.h>
#include <HexWidget.h>
#include <HexDataWindow.h>

class QVBoxLayout;
class QCheckBox;
class QLineEdit;
class QLabel;


class InspectorWidget : public QFrame {
	Q_OBJECT

public:
	InspectorWidget(HexEd *ed, QWidget *parent=0, Qt::WindowFlags f=0);
	~InspectorWidget();

	HexDocument *document() {
		return cursor()->document();
	}

	HexCursor *cursor() {
		return mCursor;
	}


public slots:
	void focusChanged(HexCursor *cur);

private slots:
	void disModeChanged(bool state);

private:
	HexCursor *mCursor;

	QVBoxLayout *mLayout;
	class InspectorModel *mModel;
	class QTableView *mTableView;

	bool m32bitMode;
	QAction *mToggleModeAction;
};

#endif
