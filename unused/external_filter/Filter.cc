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

#include <stdlib.h>

#include <HexPlugin.h>
#include <HexFile.h>

#include "Filter.h"
#include "FilterEditDialog.h"

QDataStream &operator<<(QDataStream &out, const Filter &filter) {
	out << filter.name;
	out << filter.command;
	out << filter.comment;
	out << filter.textInput;
	out << filter.textOutput;
	return out;
}

QDataStream &operator>>(QDataStream &in, Filter &filter) {
	in >> filter.name;
	in >> filter.command;
	in >> filter.comment;
	in >> filter.textInput;
	in >> filter.textOutput;
	return in;
}

QDataStream &operator<<(QDataStream &out, const QList<Filter> &filters) {
	out << filters.size();
	foreach(Filter filter, filters) {
		out << filter;
	}
	return out;
}

QDataStream &operator>>(QDataStream &in, QList<Filter> &filters) {
	int size;
	in >> size;
	while(size-- > 0) {
		Filter filter;
		in >> filter;
		filters.append(filter);
	}
	return in;
}

Filter::Filter() {
	textInput = 0;
	textOutput = 0;
}

Filter::~Filter() {
}

Filter::Filter(const Filter &filter) {
	name = filter.name;
	command = filter.command;
	comment = filter.comment;
	textInput = filter.textInput;
	textOutput = filter.textOutput;
}


void Filter::execute(HexCursor *cur, HexTextCursor *textCur, HexEd *ed) {
	QTemporaryFile inTmp, outTmp;

	QString outName, inName;

	QApplication::setOverrideCursor(Qt::WaitCursor);


	outTmp.setFileTemplate("qhexedExtFltOutXXXXXX");
	inTmp.setFileTemplate("qhexedExtFltInXXXXXX");

	inTmp.open();
	inName = inTmp.fileName();
	outTmp.open();
	outName = outTmp.fileName();
	inTmp.remove();
	outTmp.remove();

	QFile outFile(outName), inFile(inName);
	QString cmd = command.arg(inName).arg(outName);

	inFile.open(QIODevice::WriteOnly);

	if(textInput) {
		QTextStream in(&inFile);
		in << textCur->selectedText();
	} else {
		QByteArray ba = cur->selectedData();
		inFile.write(ba.data(), ba.size());
	}

	inFile.close();


	int ret = system(cmd.toAscii().data());
	if(ret < 0) {
		QMessageBox::warning(
			qApp->activeWindow(),
			"Failed to Executing Filter!",
			QString("Failed to execute filter command:\n%1").arg(cmd)
		);
		inFile.remove();
		return;
	}

	outFile.open(QIODevice::ReadOnly);
	if(textOutput) {

		QTextStream out(&outFile);
		QString s = out.readAll();
		//fprintf(stderr, "%s\n", s.toAscii().data());
		HexTextWindow *win = new HexTextWindow(s);
		win->setName(name + " out");
		ed->addWindow(win);
	} else {
		HexDocument *doc = new HexDocument(new HexBuffer(outFile.readAll()));
		doc->setName(name + " out");
		HexWindow *win = new HexDataWindow(doc);
		doc->setParent(win);
		ed->addWindow(win);
	}

	outFile.remove();
	inFile.remove();

	QApplication::restoreOverrideCursor();
}

int Filter::popupEditDialog() {
	Filter filter(*this);
	FilterEditDialog filterEdit(&filter);
	int ret = filterEdit.exec();
	if(ret != filterEdit.Rejected)
		*this = filter;

	return ret;
}
