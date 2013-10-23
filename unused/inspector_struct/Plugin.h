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


#ifndef PLUGIN_H
#define PLUGIN_H


#include <QtGui>
#include <HexPlugin.h>
#include <HexDocument.h>
#include <HexWidget.h>

#include "lexer.h"

using namespace NAMESPACE;

class LabeledValue : public QWidget {
	Q_OBJECT
public:
	LabeledValue(QString label, QString value="", QWidget *parent=0);

	QLineEdit *lineEdit() {
		return mLineEdit;
	}

public slots:
	void setValue(QString value);

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

private slots:
	void stateHandler(int state);

private:
	QLabel *mLabel;
	QCheckBox *mCheckBox;
};

class InspectorModel : public QAbstractItemModel {
	Q_OBJECT

public:
	InspectorModel(QObject *parent = 0);

	QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole) const;

	HexDocument *document() const {
		return cursor()->document();
	}

	HexCursor *cursor() const {
		return mCursor;
	}

	MCParser *parser() const {
		return mParser;
	}

	void setCursor(HexCursor *cur);
	void setParser(MCParser *p);

public slots:
	void updateModel();

private slots:
	void documentDestroyed(QObject *obj);

private:
	Node *node(int index) const {
		Q_ASSERT(parser() != 0);
		return parser()->node(index);
	}

	QString readType(OffType offset, QString type, int num);
	int typeSize(QString type);
	OffType rereadData(OffType off, Node *n);
	Node *nodeFromIndex(const QModelIndex &index) const;

	MCParser *mParser;
	HexCursor *mCursor;
	QHash<QString,int> mTypeSizes;
};


class InspectorWidget : public QFrame {
	Q_OBJECT

public:
	InspectorWidget(QWidget *parent=0, Qt::WindowFlags f=0);
	~InspectorWidget();


	MCParser *parser() const {
		return mParser;
	}

	HexCursor *cursor() const {
		return mCursor;
	}

	HexTextCursor *textCursor() const {
		return mTextCursor;
	}

	HexDocument *document() const {
		return cursor()->document();
	}

public slots:
	void focusChanged(HexCursor *cur);
	void focusChanged(HexTextCursor *cur);

signals:
	void offsetChanged(OffType offset);

private slots:
	void newOffset();
	void load();
	void store();
	void updateCols(const QModelIndex &index);

private:

	HexCursor *mCursor;
	HexTextCursor *mTextCursor;

	QString mStructSrc;
	QPushButton *mLoadButton;
	QPushButton *mStoreButton;
	//LabeledCheckBox *mAnchor;
	//LabeledValue *mOffsetValue;
	InspectorModel *mModel;
	QNodeView *mView;
	MCParser *mParser;
};

class Panel : public HexWindow {
	Q_OBJECT

public:
	Panel();
	QString name();

	bool docked();
	Qt::DockWidgetArea dockArea();

public slots:
	void focusChanged(HexCursor *cur);
	void focusChanged(HexTextCursor *cur);

private:
	InspectorWidget *mInspectorWidget;
};

class Plugin : public HexPlugin {
	Q_OBJECT

public:
	bool init(HexEd *);
	HexPluginInfo info();

private:
	HexEd *mEd;
};

#endif

