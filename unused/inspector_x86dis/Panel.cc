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


#include <QVBoxLayout>
#include "Panel.h"

Panel::Panel(HexEd *ed) {
	mInspectorWidget = new InspectorWidget(ed, this);
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	mainLayout->addWidget(mInspectorWidget);
	setLayout(mainLayout);
	setWindowTitle(name());
}

QString Panel::name() {
	return "X86 Disassembler";
}

bool Panel::docked() {
	return true;
}

Qt::DockWidgetArea Panel::dockArea() {
	return Qt::BottomDockWidgetArea;
}

void Panel::focusChanged(HexCursor *cur) {
	mInspectorWidget->focusChanged(cur);
}
