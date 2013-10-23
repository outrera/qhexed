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
#include "Plugin.h"

HEX_DEFINE_PLUGIN(Plugin)

HexPluginInfo Plugin::info() {
	return HexPluginInfo("External Filter Support Plugin", 0x00010000, "eXa");
}

bool Plugin::init(HexEd *ed) {
	mEd = ed;

	qRegisterMetaType<Filter>("Filter");
	qRegisterMetaTypeStreamOperators<Filter>("Filter");

	qRegisterMetaType<QList<Filter> >("QList<Filter>");
	qRegisterMetaTypeStreamOperators<QList<Filter> >("QList<Filter>");


	QAction *dasmAct = new QAction(tr("External &Filter"), this);
	dasmAct->setStatusTip(tr("Filter selection through external program"));
	dasmAct->setShortcut(tr("Ctrl+F"));
	connect(dasmAct, SIGNAL(triggered()), this, SLOT(filter()));
	mEd->addAction(HexToolAction, dasmAct);

	connect(mEd, SIGNAL(focusChanged(HexCursor*)),
			this, SLOT(focusChanged(HexCursor*)));

	connect(mEd, SIGNAL(focusChanged(HexTextCursor*)),
			this, SLOT(focusChanged(HexTextCursor*)));

	return true;
}

void Plugin::focusChanged(HexCursor *cur) {
	mCursor = cur;
}

void Plugin::focusChanged(HexTextCursor *cur) {
	mTextCursor = cur;
}

void Plugin::filter() {
	FilterDialog filterDialog(cursor(), textCursor(), mEd);
	filterDialog.exec();
}
