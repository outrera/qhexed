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


#ifndef INSPECTORMODEL_H
#define INSPECTORMODEL_H


#include <QString>
#include <QList>
#include <QAbstractTableModel>

#include <HexPlugin.h>

class HexDocument;

class InspectorModel : public QAbstractTableModel {
	Q_OBJECT

public:
	InspectorModel(QObject *parent = 0);

	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole) const;


	HexDocument *document() {
		return cursor()->document();
	}

	HexCursor *cursor() {
		return mCursor;
	}


	void setCursor(HexCursor *cur);

	void set32bitMode(bool state) {
		m32bitMode = state;
	}

public slots:
	void updateModel();

private slots:
	void documentDestroyed(QObject *obj);

private:
	struct CacheEntry {
		QString offset;
		QString size;
		QString code;
		QString mnemonic;
	};
	bool m32bitMode;
	int mRows;		// how many instructions to disassemble
	int mValidRows;

	HexCursor *mCursor;
	QVector<CacheEntry> mDisasmList;
};

#endif

