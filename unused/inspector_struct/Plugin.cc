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
#include "Plugin.h"


HEX_DEFINE_PLUGIN(Plugin)


/////////////////////////////////////////////////////////////////////////////

LabeledCheckBox::LabeledCheckBox(QString label, bool checked, QWidget *parent)
	: QWidget(parent)
{
	mLabel = new QLabel(label);
	mCheckBox = new QCheckBox;
	mCheckBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	connect(mCheckBox, SIGNAL(stateChanged(int)), this, SLOT(stateHandler(int)));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->addStretch();
	layout->addWidget(mLabel);
	layout->addWidget(mCheckBox);
}

void LabeledCheckBox::stateHandler(int state) {
	emit stateChanged(state != Qt::Unchecked);
}

bool LabeledCheckBox::checked() {
	return mCheckBox->checkState() != Qt::Unchecked;
}

/////////////////////////////////////////////////////////////////////////////

LabeledValue::LabeledValue(QString label, QString value, QWidget *parent)
	: QWidget(parent)
{
	mLabel = new QLabel(label);
	mLineEdit = new QLineEdit(value);
	//mLineEdit->setReadOnly(true);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(mLabel);
	layout->addStretch();
	layout->addWidget(mLineEdit);
}

void LabeledValue::setValue(QString value) {
	mLineEdit->setText(value);
}


/////////////////////////////////////////////////////////////////////////////


InspectorModel::InspectorModel(QObject *parent)
		: QAbstractItemModel(parent)
{
	mCursor = 0;
	mParser = 0;

	mTypeSizes["char"] = 1;
	mTypeSizes["s1"] = 1;
	mTypeSizes["u1"] = 1;
	mTypeSizes["s2"] = 2;
	mTypeSizes["u2"] = 2;
	mTypeSizes["s4"] = 4;
	mTypeSizes["u4"] = 4;
	mTypeSizes["s8"] = 8;
	mTypeSizes["u8"] = 8;
	mTypeSizes["float"] = 4;
	mTypeSizes["double"] = 8;

	updateModel();
}

void InspectorModel::setCursor(HexCursor *cur) {
	if(cursor()) {
		document()->disconnect(this);
		cursor()->disconnect(this);
	}

	mCursor = cur;
	if(cursor()) {
		connect(document(), SIGNAL(changed()), this, SLOT(updateModel()));
		connect(cursor(), SIGNAL(changed()), this, SLOT(updateModel()));
		connect(document(), SIGNAL(destroyed(QObject * )),
			this, SLOT(documentDestroyed(QObject * )));
	}
	if(parser()) parser()->setCursor(cursor());
	updateModel();
}

void InspectorModel::setParser(MCParser *p) {
	mParser = p;
	parser()->setCursor(cursor());
	reset();
}

void InspectorModel::documentDestroyed(QObject *obj) {
	mCursor = 0; // no need to disconnect anything
	setCursor(0);
}

Node *InspectorModel::nodeFromIndex(const QModelIndex &index) const {
	if(index.isValid()) return (Node*)index.internalPointer();
	else return parser()->node(0);
}

QModelIndex InspectorModel::index(int row, int column, const QModelIndex &parent) const {
	if(!parser()) return QModelIndex();
	return createIndex(row, column, parser()->node(nodeFromIndex(parent)->children[row]));
}

QModelIndex InspectorModel::parent(const QModelIndex &index) const {
	if(!parser()) return QModelIndex();

	Node *node = nodeFromIndex(index);
	if(!node->parent) return QModelIndex();

	Node *parentNode = parser()->node(node->parent);
	Node *grandparentNode = parser()->node(parentNode->parent);
	int row = grandparentNode->children.indexOf(node->parent);
	return createIndex(row, index.column(), parentNode);
}

int InspectorModel::rowCount(const QModelIndex &parent) const {
	if(!parser()) return 0;
	return nodeFromIndex(parent)->children.size();
}

int InspectorModel::columnCount(const QModelIndex &parent) const {
	if(!parser()) return 0;
	return 2;
}

QVariant InspectorModel::data(const QModelIndex &index, int role) const {
	if(!parser() || !index.isValid()) return QVariant();
	//if(index.row() >= (int)mInspectors.size() || index.column() >= 2) return QVariant();

	if(role == (int)Qt::DisplayRole) {
		if(index.column() == 0) { // offset:type name[size]
			Node *n = nodeFromIndex(index);
			if(n->classType() == Node::STRUCT) {
				Struct *s = n->toStruct();
				return QString("%1").arg(s->offset, 0, 16) + ":" +  s->name;
			} else if(n->classType() == Node::STRUCT_ENTRY) {
				StructEntry *se = n->toStructEntry();
				QString name = QString("%1").arg(se->offset, 0, 16) + ":" +
					se->type + " " + se->name;
				if(cursor() && se->sizeExpr) { // array
					int size = node(se->sizeExpr)->toExpr()->calc().toInt();
					name += QString("[") + QString("%1").arg(size) + "]";
				}
				return name;
			}
		} else { // value
			if(!cursor()) return QVariant();
			Node *n = nodeFromIndex(index);
			if(n->classType() == Node::STRUCT_ENTRY) {
				StructEntry *se = n->toStructEntry();
				return se->value;
			}
		}
	}

	return QVariant();
}

QVariant InspectorModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if(role != Qt::DisplayRole)
		return QVariant();

	if(orientation == Qt::Horizontal) {
		if(section == 0) return QString("Name");
		else if(section == 1) return QString("Value");
		else return QVariant();
	} else {
		return QVariant();
	}
}

int InspectorModel::typeSize(QString type) {
	QHash<QString,int>::iterator it = mTypeSizes.find(type);
	if(it != mTypeSizes.end()) return *it;
	return 0;
}

QString InspectorModel::readType(OffType offset, QString type, int inum) {
	const int bufSize = 64;
	int num = inum;
	u1 buf[bufSize];
	QString r;
	int tsz = typeSize(type);
	int size = tsz*num;
	if(size > bufSize) {
		num = bufSize/tsz;
		size = num*tsz;
	}

	HexDocument *doc = cursor()->document();

	for(int i = 0; i < size; i++)
		buf[i] = (*doc)[offset+i];

	if(type == "char") {
		char *p = (char*)buf;
		p[num < bufSize ? num : bufSize-1] = 0;
		r += p;
	} else  if(type == "s1") {
		s1 *p = (s1*)buf;
		for(int i = 0; i < num; i++)
			r += QString(i ? ",%1" : "%1").arg(p[i], 0, 16);
	} else if(type == "u1") {
		u1 *p = (u1*)buf;
		for(int i = 0; i < num; i++)
			r += QString(i ? ",%1" : "%1").arg(p[i], 0, 16);
	} else if(type == "s2") {
		s2 *p = (s2*)buf;
		for(int i = 0; i < num; i++)
			r += QString(i ? ",%1" : "%1").arg(p[i], 0, 16);
	} else if(type == "u2") {
		u2 *p = (u2*)buf;
		for(int i = 0; i < num; i++)
			r += QString(i ? ",%1" : "%1").arg(p[i], 0, 16);
	} else if(type == "s4") {
		s4 *p = (s4*)buf;
		for(int i = 0; i < num; i++)
			r += QString(i ? ",%1" : "%1").arg(p[i], 0, 16);
	} else if(type == "u4") {
		u4 *p = (u4*)buf;
		for(int i = 0; i < num; i++)
			r += QString(i ? ",%1" : "%1").arg(p[i], 0, 16);
	} else if(type == "s8") {
		s8 *p = (s8*)buf;
		for(int i = 0; i < num; i++)
			r += QString(i ? ",%1" : "%1").arg(p[i], 0, 16);
	} else if(type == "u8") {
		u8 *p = (u8*)buf;
		for(int i = 0; i < num; i++)
			r += QString(i ? ",%1" : "%1").arg(p[i], 0, 16);
	} else if(type == "float") {
		float *p = (float*)buf;
		for(int i = 0; i < num; i++)
			r += QString(i ? ",%1" : "%1").arg(p[i]);
	} else if(type == "double") {
		double *p = (double*)buf;
		for(int i = 0; i < num; i++)
			r += QString(i ? ",%1" : "%1").arg(p[i]);
	}

	if(num != inum) r+= "...";

	return r;
}

OffType InspectorModel::rereadData(OffType off, Node *n) {
	Q_ASSERT(n != 0);

	switch(n->classType()) {
	case Node::ROOT: {
		OffType retOff = off;
		foreach(int index, n->children) {
			OffType tmpOff = rereadData(off, node(index));
			if(tmpOff > retOff) retOff = tmpOff;
		}
		off = retOff; // retOff points to end of largest structure
		break;}

	case Node::STRUCT: {
		Struct *s = n->toStruct();
		s->offset = off;
		foreach(int index, n->children) {
			off = rereadData(off, node(index));
		}
		break;}

	case Node::STRUCT_ENTRY: {
		StructEntry *se = n->toStructEntry();
		se->offset = off;
		if(se->sizeExpr) {
			int num = node(se->sizeExpr)->toExpr()->calc().toInt();
			se->value = readType(off, se->type, num);
			off += typeSize(se->type)*num;
		} else {
			se->value = readType(off, se->type, 1);
			off += typeSize(se->type);
		}
		break;}

	default:
		break;
	}
	return off;
}

void InspectorModel::updateModel() {
	if(parser() && node(0)->children.size()) {
		if(cursor()) rereadData(cursor()->position(), parser()->node(0));
		emit dataChanged(index(0, 0, QModelIndex()),
				index(parser()->node(0)->children.size()-1, 1, QModelIndex()));
	} else {
		reset();
	}
}


/////////////////////////////////////////////////////////////////////////////

InspectorWidget::InspectorWidget(QWidget *parent, Qt::WindowFlags f)
		: QFrame(parent, f) {
	setFrameStyle(QFrame::Panel|QFrame::Raised);
	setLineWidth(1);

	QSettings settings;
	settings.beginGroup("StructureInspectorWidget");
	mStructSrc = settings.value("mStructSrc", QString()).toString();
	settings.endGroup();

	mLoadButton = new QPushButton(tr("Load"), this);
	connect(mLoadButton, SIGNAL(clicked()), this, SLOT(load()));

	mStoreButton = new QPushButton(tr("Store"), this);
	connect(mStoreButton, SIGNAL(clicked()), this, SLOT(store()));

	mTextCursor = 0;

	QHBoxLayout *buttonsLay = new QHBoxLayout;
	buttonsLay->addWidget(mLoadButton);
	buttonsLay->addWidget(mStoreButton);

	mParser = new MCParser;
	mParser->parse(mStructSrc);

	mModel = new InspectorModel(this);
	mModel->setParser(mParser);


	mView = new QNodeView(this);
	mView->setModel(mModel);
	mView->setAlternatingRowColors(true);
	connect(mView, SIGNAL(expanded(const QModelIndex &)),
			this, SLOT(updateCols(const QModelIndex &)));
	connect(mView, SIGNAL(collapsed(const QModelIndex &)),
			this, SLOT(updateCols(const QModelIndex &)));



	mView->header()->setStretchLastSection(false);
	mView->header()->setResizeMode(QHeaderView::Interactive);
	//mView->header()->setResizeMode(QHeaderView::Stretch);
	//mView->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	//mView->setColumnWidth(0, 8);
	//mView->setSelectionBehavior(QAbstractItemView::SelectRows);
	//mView->setFrameStyle(QFrame::NoFrame);
	//mView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//mView->QFrame::setLineWidth(0);

	QFont font = mView->font();
	font.setPointSize(8);
	QFontMetrics fontMetrics(font);
	mView->setFont(font);
	mView->resizeColumnToContents(0);
	//mTableView->horizontalHeader()->setMaximumHeight(mTableView->verticalHeader()->fontMetrics().height());
	//mView->verticalHeader()->setResizeMode(QHeaderView::Custom);
	//for(int i = 0; i < mInspectors.size(); i++)
	//	mTableView->setRowHeight(i, fontMetrics.ascent()*3/2);

	mView->setSelectionMode(QAbstractItemView::NoSelection);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setMargin(0);

	layout->addWidget(mView);
	layout->addLayout(buttonsLay);

	focusChanged((HexCursor*)0);
	focusChanged((HexTextCursor*)0);
}

InspectorWidget::~InspectorWidget() {
	delete mParser;
}


void InspectorWidget::updateCols(const QModelIndex &) {
	mView->resizeColumnToContents(0);
	mView->resizeColumnToContents(1);
}

void InspectorWidget::focusChanged(HexCursor *cur) {
	mCursor = cur;
	mModel->setCursor(cursor());
}

void InspectorWidget::focusChanged(HexTextCursor *cur) {
	mTextCursor = cur;
	mLoadButton->setEnabled(textCursor() ? true : false);
	mStoreButton->setEnabled(textCursor() ? true : false);
}

void InspectorWidget::load() {
	if(!textCursor()) return;
	QString src = textCursor()->textEdit()->toPlainText();
	MCParser *newParser = new MCParser;
	if(newParser->parse(src)) {
		mStructSrc = src;
		QSettings settings;
		settings.beginGroup("StructureInspectorWidget");
		settings.setValue("mStructSrc", mStructSrc);
		settings.endGroup();
		delete mParser;
		mParser = newParser;
		mModel->setParser(parser());
	} else {
		QMessageBox::warning(this, tr("Compilation failed!"),
							 tr("Parse error at line %1:\n%2")
							 .arg(newParser->errorLine())
							 .arg(newParser->errorMessage()));
		delete newParser;
	}
}

void InspectorWidget::store() {
	if(!textCursor()) return;
	textCursor()->textEdit()->setPlainText(mStructSrc);
}

/*void InspectorWidget::setOffset(OffType off) {
	if(cursor()) {
		cursor()->setTop(off);
	}
}*/

/////////////////////////////////////////////////////////////////////////////

Panel::Panel() {
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mInspectorWidget = new InspectorWidget(this);
	mainLayout->addWidget(mInspectorWidget);
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	setLayout(mainLayout);
	setWindowTitle(name());
}

QString Panel::name() {
	return "Data Inspector";
}

bool Panel::docked() {
	return true;
}

Qt::DockWidgetArea Panel::dockArea() {
	return Qt::RightDockWidgetArea;
}

void Panel::focusChanged(HexCursor *cur) {
	mInspectorWidget->focusChanged(cur);
}

void Panel::focusChanged(HexTextCursor *cur) {
	mInspectorWidget->focusChanged(cur);
}

/////////////////////////////////////////////////////////////////////////////

HexPluginInfo Plugin::info() {
	return HexPluginInfo("Data Structure Inspector Panel", 0x00010000, "eXa");
}

bool Plugin::init(HexEd *ed) {
	mEd = ed;
	Panel *panel = new Panel;
 
	connect(mEd, SIGNAL(focusChanged(HexCursor*)), panel, SLOT(focusChanged(HexCursor*)));
	connect(mEd, SIGNAL(focusChanged(HexTextCursor*)), panel, SLOT(focusChanged(HexTextCursor*)));

	mEd->addWindow(panel);

	return true;
}

