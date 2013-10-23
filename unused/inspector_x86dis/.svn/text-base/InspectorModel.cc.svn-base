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

#include "InspectorModel.h"

#include "x86.h"

InspectorModel::InspectorModel(QObject *parent)
		: QAbstractTableModel(parent) {
	

	mValidRows = 0;
	mRows = 20;
	m32bitMode = 0;
	mCursor = 0;
	mDisasmList.resize(mRows);
	updateModel();
}

void InspectorModel::setCursor(HexCursor *cur) {
	if(cursor()) {
		document()->disconnect(this);
		cursor()->disconnect(this);
	}

	mCursor = cur;
	if(cursor()) {
		connect(document(), SIGNAL(changed()),
				this, SLOT(updateModel()));
		connect(cursor(), SIGNAL(changed()),
				this, SLOT(updateModel()));
		connect(document(), SIGNAL(destroyed(QObject * )),
				this, SLOT(documentDestroyed(QObject * )));
	}
	updateModel();
}

void InspectorModel::documentDestroyed(QObject *obj) {
	mCursor = 0;
	setCursor(0);
}

int InspectorModel::rowCount(const QModelIndex &parent) const {
	return mRows;
}

int InspectorModel::columnCount(const QModelIndex &parent) const {
	return 4;
}

QVariant InspectorModel::data(const QModelIndex &index, int role) const {
	if(index.isValid() && role == (int)Qt::DisplayRole && index.row() < mValidRows) {
		if(index.column() == 0)
			return mDisasmList[index.row()].offset;
		else if(index.column() == 1)
			return mDisasmList[index.row()].size;
		else if(index.column() == 2)
			return mDisasmList[index.row()].code;
		else if(index.column() == 3)
			return mDisasmList[index.row()].mnemonic;
		else
			return QVariant();
	} else {
		return QVariant();
	}
}

QVariant InspectorModel::headerData(
	int section,
	Qt::Orientation orientation,
	int role
) const {
	if (role != Qt::DisplayRole)
		return QVariant();

	if(orientation == Qt::Horizontal) {
		if(section == 0)
			return QString("Offset");
		else if(section == 1)
			return QString("Size");
		else if(section == 2)
			return QString("Code");
		else if(section == 3)
			return QString("Mnemonic");
		else
			return QVariant();
	} else
		return QVariant();
}


void InspectorModel::updateModel() {
	int i;
	mValidRows = 0;

	if(cursor()) {
		uint8_t data[X86_MAXCMDSIZE];
		HexDocument *doc = document(); 
		OffType offset = cursor()->position();

		for(; mValidRows < mRows; mValidRows++) {
			mDisasmList[mValidRows].offset = "0x"+QString("%1").arg(offset, 8, 16, QChar('0'));

			for(i = 0; i < X86_MAXCMDSIZE; i++)
				data[i] = (*doc)[offset+i];

			X86_Instr inst;
			inst.usr.ip = offset;
			inst.usr.mode = m32bitMode ? X86_MODE_IA32 : X86_MODE_IA16;

			if(!X86_Dec(&inst, data)) {
				mDisasmList[mValidRows].size = "<failed>";
				mDisasmList[mValidRows].code = "<failed>";
				mDisasmList[mValidRows].mnemonic = "<failed>";
				mValidRows++;
				break;
			} else {
				char mnemo[256];
				memset(mnemo, 0, 256);
				X86_Dis(mnemo, &inst);
				//fprintf(stderr, "%3i  %-24s  %-24s\n", inst.size, inst.dump, mnemo);
				mDisasmList[mValidRows].size = QString("%1").arg(inst.size);
				mDisasmList[mValidRows].code = inst.dump;
				mDisasmList[mValidRows].mnemonic = mnemo;
				offset += inst.size;
			}
		}
	}

	emit dataChanged(index(0, 0, QModelIndex()), index(mRows, 3, QModelIndex()));
}

