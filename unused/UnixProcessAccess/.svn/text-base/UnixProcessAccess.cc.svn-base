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

#include <HexDocument.h>
#include <HexPlugin.h>
#include <HexDataWindow.h>

#include "OpenProcessDialog.h"
#include "PTraceModel.h"
#include "UnixProcessAccess.h"

HEX_DEFINE_PLUGIN(UnixProcessAccess)

HexPluginInfo UnixProcessAccess::info() {
	return HexPluginInfo("Unix Process Access Plugin", 0x00010000, "eXa");
}

bool UnixProcessAccess::init(HexEd *ed) {
	mEd = ed;

	openAct = new QAction(QIcon(":/images/process.png"), tr("Open &Process..."), this);
	openAct->setShortcut(tr("Ctrl+P"));
	openAct->setStatusTip(tr("Open process memory for direct access"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	mEd->addAction(HexFileAction, openAct);
	return true;
}

void UnixProcessAccess::open() {
	QString pid = OpenProcessDialog::ask(tr("Choice Process to Open"));
	if(pid.isEmpty()) return;

	PTraceModel *pm = new PTraceModel;
	if(!pm->open(pid)) {
		QMessageBox::warning(qApp->activeWindow(), tr("Open failed!"),
							 tr("Cannot open process %1:\n%2.")
							 .arg(pid)
							 .arg(pm->errorString()));
		delete pm;
		return;
	}
	HexDocument *doc = new HexDocument(pm);
	doc->setName("PID: " + pid);
	mEd->addWindow(new HexDataWindow(doc));
}
