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

#include "Plugin.h"
#include "Panel.h"

HEX_DEFINE_PLUGIN(Plugin)

HexPluginInfo Plugin::info() {
	return HexPluginInfo("X86 Quick Disassembler Panel", 0x00010000, "eXa");
}

bool Plugin::init(HexEd *ed) {
	mEd = ed;
	Panel *panel = new Panel(ed);
 
	connect(mEd, SIGNAL(focusChanged(HexCursor*)),
			panel, SLOT(focusChanged(HexCursor*)));

	mEd->addWindow(panel);

	return true;
}

