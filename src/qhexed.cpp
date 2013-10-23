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

#include "qhexed.h"



HexPluginInfo BasicFileAccess::info() {
    return HexPluginInfo("Basic File Access Plugin", 0x00010000, "eXa");
}

bool BasicFileAccess::init(HexEd *ed) {
    mEd = ed;

    newBufferAct = new QAction(QIcon(":/images/new.png"), tr("&New Buffer"), this);
    newBufferAct->setShortcut(tr("Ctrl+N"));
    newBufferAct->setStatusTip(tr("Create a new editing buffer"));
    connect(newBufferAct, SIGNAL(triggered()), this, SLOT(newBuffer()));

    newTextBufferAct = new QAction(QIcon(":/images/new.png"), tr("New &Text Buffer"), this);
    newTextBufferAct->setShortcut(tr("Ctrl+T"));
    newTextBufferAct->setStatusTip(tr("Create a new text editing buffer"));
    connect(newTextBufferAct, SIGNAL(triggered()), this, SLOT(newTextBuffer()));

    openFileAct = new QAction(QIcon(":/images/open.png"), tr("&Open File..."), this);
    openFileAct->setShortcut(tr("Ctrl+O"));
    openFileAct->setStatusTip(tr("Open file for direct editing"));
    connect(openFileAct, SIGNAL(triggered()), this, SLOT(openFile()));

    loadFileAct = new QAction(QIcon(":/images/load.png"), tr("&Load File..."), this);
    loadFileAct->setShortcut(tr("Ctrl+L"));
    loadFileAct->setStatusTip(tr("Load file into memory buffer"));
    connect(loadFileAct, SIGNAL(triggered()), this, SLOT(loadFile()));

    loadTextFileAct = new QAction(QIcon(":/images/book.png"), tr("Load Text File..."), this);
    loadTextFileAct->setStatusTip(tr("Load text file into memory buffer"));
    connect(loadTextFileAct, SIGNAL(triggered()), this, SLOT(loadTextFile()));

    mEd->addAction(HexFileAction, newBufferAct);
    mEd->addAction(HexFileAction, newTextBufferAct);
    mEd->addAction(HexFileAction, openFileAct);
    mEd->addAction(HexFileAction, loadFileAct);
    mEd->addAction(HexFileAction, loadTextFileAct);

    return true;
}

void BasicFileAccess::newBuffer() {
    HexDocument *doc = new HexDocument;
    HexWindow *win = new HexDataWindow(doc);
    doc->setParent(win);
    mEd->addWindow(win);
}

void BasicFileAccess::newTextBuffer() {
    mEd->addWindow(new HexTextWindow);
}

void BasicFileAccess::openFile() {
    QString filePath = QFileDialog::getOpenFileName(qApp->activeWindow());
    if(filePath.isEmpty()) return;

    QFile *file = new QFile(filePath);

    if(!file->open(QFile::ReadWrite) && !file->open(QFile::ReadOnly)) {
        QMessageBox::warning(qApp->activeWindow(), tr("Open failed!"),
                             tr("Cannot open file %1:\n%2.")
                             .arg(filePath)
                             .arg(file->errorString()));
        delete file;
        return;
    }

    HexDocument *doc = new HexDocument(new HexFile(file), this);
    doc->setName(QFileInfo(filePath).fileName());
    HexWindow *win = new HexDataWindow(doc);
    doc->setParent(win);
    mEd->addWindow(win);
    mEd->updateStatus(tr("File opened"));
}

void BasicFileAccess::loadFile() {
    QString filePath = QFileDialog::getOpenFileName(qApp->activeWindow());
    if(filePath.isEmpty()) return;

    QFile file(filePath);
    if(!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(qApp->activeWindow(), tr("Load failed!"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(filePath)
                             .arg(file.errorString()));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    HexDocument *doc = new HexDocument(new HexBuffer(file.readAll()));
    doc->setPath(filePath);
    HexWindow *win = new HexDataWindow(doc);
    doc->setParent(win);
    mEd->addWindow(win);
    QApplication::restoreOverrideCursor();
    mEd->updateStatus(tr("File loaded"));
}

void BasicFileAccess::loadTextFile() {
    QString filePath = QFileDialog::getOpenFileName(qApp->activeWindow());
    if(filePath.isEmpty()) return;

    HexTextWindow *win = new HexTextWindow;

    if(!win->loadFile(filePath)) {
        delete win;
        return;
    }
    mEd->addWindow(win);
    mEd->updateStatus(tr("File loaded"));
}


InspectorModel::InspectorModel(QList<InspectorFunction*> &inspectors, QObject *parent)
        : QAbstractTableModel(parent), mInspectors(inspectors) {

    //foreach(InspectorFunction *inspector, inspectors)
    //	connect(inspector, SIGNAL(updateRequested(), this SLOT(updateModel())));
    mCursor = 0;
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
    updateModel();
}

void InspectorModel::documentDestroyed(QObject *obj) {
    mCursor = 0; // no need to disconnect anything
    setCursor(0);
}

int InspectorModel::rowCount(const QModelIndex &parent) const {
    return mInspectors.size();
}

int InspectorModel::columnCount(const QModelIndex &parent) const {
    return 2;
}

QVariant InspectorModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) return QVariant();
    if(index.row() >= (int)mInspectors.size() || index.column() >= 2) return QVariant();

    if(role == (int)Qt::DisplayRole) {
        if(index.column() == 0)
            return mInspectors.at(index.row())->name();
        else {
            if(cursor()) {
                return mInspectors.at(index.row())->value(
                    document(),
                    cursor()->position()
                );
            } else {
                return QVariant();
            }
        }
    } else return QVariant();
}

QVariant InspectorModel::headerData(int section, Qt::Orientation orientation,
        int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Horizontal) {
        if(section == 0)
            return QString("Type");
        else if(section == 1)
            return QString("Value");
        else
            return QVariant();
    } else
        return QVariant();
}


void InspectorModel::updateModel() {
    emit dataChanged(index(0, 0, QModelIndex()), index(mInspectors.size()-1, 1, QModelIndex()));
}



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
    return "Basic Inspectors";
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





QString ScalarInspector::value(HexDocument *doc, OffType offset) {
    if(!doc) return "";

    uint16_t ui16;
    uint32_t ui32;

    uint8_t data[16];
    for(int i = 0; i < 16; i++)
        data[i] = (*doc)[offset+i];

    switch(mType) {
    case DispS1:
        return QString().sprintf("%d", (int)(int8_t)*data);

    case DispU1:
        if(!mHex) return QString().sprintf("%u", (unsigned int)*data);
        else return QString().sprintf("0x%02X", (unsigned int)*data);

    case DispS2:
        ui16 = LSB16(data);
        if(mOrder != Lsb) ui16 = SWAP16(ui16);
        return QString().sprintf("%d", (int)(int16_t)ui16);

    case DispU2:
        ui16 = LSB16(data);
        if(mOrder != Lsb) ui16 = SWAP16(ui16);
        if(!mHex) return QString().sprintf("%u", (unsigned int)ui16);
        else return QString().sprintf("0x%04X", (unsigned int)ui16);

    case DispS4:
        ui32 = LSB32(data);
        if(mOrder != Lsb) ui32 = SWAP32(ui32);
        return QString().sprintf("%d", (int)(int32_t)ui32);

    case DispU4:
        ui32 = LSB32(data);
        if(mOrder != Lsb) ui32 = SWAP32(ui32);
        if(!mHex) return QString().sprintf("%u", (unsigned int)ui32);
        else return QString().sprintf("0x%08X", (unsigned int)ui32);

    case DispF4:
        return QString().sprintf("%E", *(float*)data);

    case DispF8:
        return QString().sprintf("%E", *(double*)data);

    case DispOctal:
        return QString().sprintf("%03o", (unsigned int)*data);

    case DispBinary:
        return bitsToString(*data);
    }
    return "";
}





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

InspectorWidget::InspectorWidget(QWidget *parent, Qt::WindowFlags f)
        : QFrame(parent, f) {
    setFrameStyle(QFrame::Panel|QFrame::Raised);
    setLineWidth(1);

    QSettings settings;
    settings.beginGroup("BasicDataInspectorWidget");
    mLsb = settings.value("mLsb", true).toBool();
    mHex = settings.value("mHex", false).toBool();
    settings.endGroup();

    ScalarInspector::ByteOrder order = mLsb ? ScalarInspector::Lsb : ScalarInspector::Msb;

    mS1 = new ScalarInspector("int8_t", ScalarInspector::DispS1, order);
    mInspectors.append(mS1);

    mU1 = new ScalarInspector("uint8_t", ScalarInspector::DispU1, order, mHex);
    mInspectors.append(mU1);

    mS2 = new ScalarInspector("int16_t", ScalarInspector::DispS2, order);
    mInspectors.append(mS2);

    mU2 = new ScalarInspector("uint16_t", ScalarInspector::DispU2, order, mHex);
    mInspectors.append(mU2);

    mS4 = new ScalarInspector("int32_t", ScalarInspector::DispS4, order);
    mInspectors.append(mS4);

    mU4 = new ScalarInspector("uint32_t", ScalarInspector::DispU4, order, mHex);
    mInspectors.append(mU4);

    mF4 = new ScalarInspector("float", ScalarInspector::DispF4);
    mInspectors.append(mF4);

    mF8 = new ScalarInspector("double", ScalarInspector::DispF8);
    mInspectors.append(mF8);

    mOctal = new ScalarInspector("octal", ScalarInspector::DispOctal);
    mInspectors.append(mOctal);

    mBinary = new ScalarInspector("binary", ScalarInspector::DispBinary);
    mInspectors.append(mBinary);

    mModel = new InspectorModel(mInspectors, this);
    QTableView *mTableView = new QTableView(this);
    //mTableView->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    //mTableView->setColumnWidth(0, 8);
    mTableView->verticalHeader()->hide();
    mTableView->verticalScrollBar()->hide();
    mTableView->setShowGrid(false);
    mTableView->setAlternatingRowColors(true);
    mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    //mTableView->setFrameStyle(QFrame::NoFrame);
    //mTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //mTableView->QFrame::setLineWidth(0);
    mTableView->setModel(mModel);

    QFont font = mTableView->font();
    font.setPointSize(8);
    QFontMetrics fontMetrics(font);
    mTableView->setFont(font);
    mTableView->resizeColumnToContents(0);
    mTableView->resizeRowsToContents();
    //mTableView->horizontalHeader()->setMaximumHeight(mTableView->verticalHeader()->fontMetrics().height());
    mTableView->verticalHeader()->setResizeMode(QHeaderView::Custom);
    for(int i = 0; i < mInspectors.size(); i++)
        mTableView->setRowHeight(i, fontMetrics.ascent()*3/2);

    mTableView->horizontalHeader()->setResizeMode(0, QHeaderView::Interactive);
    mTableView->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    mTableView->setSelectionMode(QAbstractItemView::NoSelection);

    mCheckLsb = new LabeledCheckBox("Little Endian", mLsb);
    mCheckHex = new LabeledCheckBox("Usigned in Hex", mHex);

    mLayout = new QVBoxLayout(this);
    mLayout->setSpacing(0);
    mLayout->setMargin(0);

    mLayout->addWidget(mTableView);

    mOffsetValue = new LabeledValue("Goto:", "0", this);
    mOffsetValue->lineEdit()->setMaxLength(8);
    QRegExp regex("[0-9abcdefABCDEF]{0,16}", Qt::CaseInsensitive);
    QRegExpValidator *validator = new QRegExpValidator(regex, this);
    mOffsetValue->lineEdit()->setValidator(validator);
    connect(mOffsetValue->lineEdit(), SIGNAL(returnPressed()), this, SLOT(newOffset()));

    mLayout->addWidget(mOffsetValue);

    //mLayout->addStretch();
    mLayout->addWidget(mCheckLsb);
    mLayout->addWidget(mCheckHex);

    connect(mCheckLsb, SIGNAL(stateChanged(bool)), this, SLOT(wordFormatChanged(bool)));
    connect(mCheckHex, SIGNAL(stateChanged(bool)), this, SLOT(unsignedFormatChanged(bool)));

    focusChanged(0);
}

InspectorWidget::~InspectorWidget() {
    QSettings settings;
    settings.beginGroup("BasicDataInspectorWidget");
    settings.setValue("mLsb", mLsb);
    settings.setValue("mHex", mHex);
    settings.endGroup();
}

void InspectorWidget::wordFormatChanged(bool state) {
    mLsb = state;
    ScalarInspector::ByteOrder order = mLsb ? ScalarInspector::Lsb : ScalarInspector::Msb;
    mU1->setOrder(order);
    mU2->setOrder(order);
    mU4->setOrder(order);
    mS1->setOrder(order);
    mS2->setOrder(order);
    mS4->setOrder(order);
    mModel->updateModel();
}

void InspectorWidget::unsignedFormatChanged(bool state) {
    mHex = state;
    mU1->setHex(mHex);
    mU2->setHex(mHex);
    mU4->setHex(mHex);
    mModel->updateModel();
}

void InspectorWidget::focusChanged(HexCursor *cur) {
    mCursor = cur;
    mModel->setCursor(cursor());
}

void InspectorWidget::setOffset(OffType offset) {
    mOffsetValue->lineEdit()->setText(QString("%1").arg(offset, 0, 16));
}

void InspectorWidget::newOffset() {
    if(cursor()) {
        OffType offset = mOffsetValue->lineEdit()->text().toLongLong(0, 16);
        cursor()->setTop(offset);
    }
}







HexPluginInfo BasicInspector::info() {
    return HexPluginInfo("Basic Data Inspectors Panel", 0x00010000, "eXa");
}

bool BasicInspector::init(HexEd *ed) {
    mEd = ed;
    Panel *panel = new Panel;

    connect(mEd, SIGNAL(focusChanged(HexCursor*)),
            panel, SLOT(focusChanged(HexCursor*)));

    mEd->addWindow(panel);

    return true;
}



ConfigDialog::ConfigDialog(QWidget *parent)
{
    QHBoxLayout *viewLayout = new QHBoxLayout;
    mOptionSelector = new QListWidget(this);
    mOptionPanel = new QStackedLayout;
    viewLayout->addWidget(mOptionSelector);
    viewLayout->addLayout(mOptionPanel);
    connect(mOptionSelector, SIGNAL(currentRowChanged(int)),
            mOptionPanel, SLOT(setCurrentIndex(int)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(viewLayout);

    QPushButton *closeButton = new QPushButton(tr("&Close"), this);
    connect(closeButton, SIGNAL(pressed()), this, SLOT(close()));
    mainLayout->addWidget(closeButton);

    setLayout(mainLayout);
    setWindowTitle(tr("Configure"));
}

ConfigDialog::~ConfigDialog() {
}


void ConfigDialog::closeEvent(QCloseEvent *event) {
    emit closed(true);
}

void ConfigDialog::addPanel(QWidget *panel, QString text, QIcon icon) {
    mOptionPanel->addWidget(panel);
    new QListWidgetItem(icon, text, mOptionSelector);
}


HexPluginInfo ConfigPlugin::info() {
    return HexPluginInfo(tr("Hex View Configurator"), 0x00010000, "Nikita Sadkov");
}

bool ConfigPlugin::init(HexEd *ed) {
    mEd = ed;
    mCfgAct = new QAction(tr("&Hex View"), this);
    connect(mCfgAct, SIGNAL(triggered()), this, SLOT(popupCfgDialog()));
    ed->addAction(HexSettingsAction, mCfgAct);
    return true;
}

void ConfigPlugin::popupCfgDialog() {
    QWidget *widget = HexWidget::configPanel();
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->setWindowTitle(tr("Hex View Configurator"));
    connect(widget, SIGNAL(destroyed()), this, SLOT(cfgDialogClosed()));
    mCfgAct->setDisabled(true);
    widget->show();
}

void ConfigPlugin::cfgDialogClosed() {
    mCfgAct->setEnabled(true);
}


HexBuffer::HexBuffer(QObject *parent)
    : HexDataModel(parent)
{
    mSize = 0;
}

HexBuffer::HexBuffer(const QByteArray &ba, QObject *parent)
    : HexDataModel(parent)
{
    mSize = 0;
    paste(0, ba);
}

void HexBuffer::write(OffType dst64, const void *srcVoid, OffType size64) {
    if(!size64) return;

    OffType start = dst64;
    OffType end = dst64+size64;
    char *src = (char*)srcVoid;

    if(start >= mSize) {
        QByteArray right(end-mSize, 0);
        memcpy(right.data()+start-mSize, src, end-start);
        mBuffer.append(right);
        mSize = end;
        return;
    }

    if(end > mSize) {
        mBuffer.append(QByteArray(src + mSize-start, end-mSize));
        qSwap(mSize, end);
    }

    QMutableLinkedListIterator<QByteArray> it(mBuffer);

    OffType offset = 0;
    for(;;) {
        assert(it.hasNext());
        QByteArray &ba = it.next();
        OffType nextOffset = offset + ba.size();
        if(start < nextOffset) {
            OffType add = offset-start;
            OffType wrsz = qMin(ba.size()-add, end-start);
            memcpy(ba.data()+add, src, wrsz);
            src += wrsz;
            start += wrsz;
            break;
        }
        offset = nextOffset;
    }

    while(start < end) {
        assert(it.hasNext());
        QByteArray &ba = it.next();
        int wrsz = qMin(ba.size(), (int)(end-start));
        memcpy(ba.data(), src, wrsz);
        src += wrsz;
        start += wrsz;
    }
}

void HexBuffer::read(void *dstVoid, OffType src64, OffType size64) {
    OffType start = src64;
    char *dst = (char*)dstVoid;

    if(start >= mSize) {
        memset(dst, 0, size64);
        return;
    }
    OffType end = start+size64;

    if(end > mSize) {
        memset(dst + mSize-start, 0, end-mSize);
        end = mSize;
    }

    QMutableLinkedListIterator<QByteArray> it(mBuffer);

    OffType offset = 0;

    for(;;) {
        assert(it.hasNext());
        QByteArray &ba = it.next();
        OffType nextOffset = offset+ba.size();
        if(start < nextOffset) {
            int add = start - offset;
            int rdsz = qMin(ba.size()-add, (int)(end-start));
            memcpy(dst, ba.data()+add, rdsz);
            dst += rdsz;
            start += rdsz;
            break;
        }
        offset = nextOffset;
    }

    while(start < end) {
        assert(it.hasNext());
        QByteArray &ba = it.next();
        int rdsz = qMin(ba.size(), (int)(end-start));
        memcpy(dst, ba.data(), rdsz);
        dst += rdsz;
        start += rdsz;
    }
}

void HexBuffer::del(OffType start, OffType end) {
    if(start >= mSize) return;
    if(end > mSize) end = mSize;

    mSize -= end-start;

    QMutableLinkedListIterator<QByteArray> it(mBuffer);

    OffType offset = 0;
    for(;;) {
        assert(it.hasNext());
        QByteArray &ba = it.next();
        OffType nextOffset = offset + ba.size();
        if(start < nextOffset) {
            if(offset < start) {
                QByteArray left(ba.data(), start-offset);
                if(nextOffset <= end) ba = left;
                else {
                    QByteArray right(ba.data() + end-offset, nextOffset-end);
                    ba = left;
                    it.insert(right);
                }
            } else {
                if(end < nextOffset)
                    ba = QByteArray(ba.data() + end-offset, nextOffset-end);
                else {
                    it.remove();
                }
            }
            offset = nextOffset;
            break;
        }
        offset = nextOffset;
    }

    while(offset < end) {
        assert(it.hasNext());
        QByteArray &ba = it.next();
        OffType nextOffset = offset + ba.size();
        if(end < nextOffset)
            ba = QByteArray(ba.data() + end-offset, nextOffset-end);
        else {
            it.remove();
        }
        offset = nextOffset;
    }
}

void HexBuffer::paste(OffType where, const QByteArray &what) {
    if(!what.size()) return;

    if(!where) {
        mBuffer.prepend(what);
        mSize += what.size();
        return;
    }

    if(where >= mSize) {
        mBuffer.append(what);
        mSize += what.size();
        return;
    }

    QMutableLinkedListIterator<QByteArray> it(mBuffer);

    OffType offset = 0;
    for(;;) {
        assert(it.hasNext());
        QByteArray &ba = it.next();
        OffType nextOffset = offset + ba.size();
        if(where < nextOffset) {
            if(where == offset) {
                it.previous();
                it.insert(what);
            } else {
                int splitPoint = where - offset;
                QByteArray right(ba.data() + splitPoint, ba.size()-splitPoint);
                ba = QByteArray(ba.data(), splitPoint);
                it.insert(what);
                it.insert(right);
            }
            break;
        }
        offset = nextOffset;
    }
    mSize += what.size();
}

OffType HexBuffer::getLength() {
    return mSize;
}

bool HexBuffer::isGrowable() {
    return true;
}

bool HexBuffer::isWriteable() {
    return true;
}

bool HexBuffer::isCuttable() {
    return true;
}

OffType HexBuffer::getPageSize() {
    return 1024;
}



#define USE_QHASH

#ifdef USE_QHASH

#include <QHash>

#else
#ifdef __GNUC__
    #if __GNUC__ < 3
        #include <hash_map.h>
        namespace Sgi { using ::hash_map; };	// inherit globals
    #else
        #include <ext/hash_map>
        #if __GNUC_MINOR__ == 0
            namespace Sgi = std;		// GCC 3.0
        #else
          namespace Sgi = ::__gnu_cxx;	// GCC 3.1 and later
        #endif
    #endif
#else	// ...  there are other compilers, right?
    namespace Sgi = std;
#endif
#endif


#ifdef USE_QHASH
    typedef QHash<OffType, HexCache::Page*> Hash;
#else
    typedef Sgi::hash_map<OffType, HexCache::Page*> Hash; // hash map for offset2page mapping
#endif

#define map (*(Hash*)hashMap)

HexCache::HexCache(HexDataModel &inDsm, int cacheSize, int inPageSize)
    : dsm(inDsm) {
    pageSize = inPageSize > 0 ? inPageSize : dsm.getPageSize();
    if(pageSize < 1) pageSize = 1;
    numPages = (cacheSize+pageSize-1)/pageSize;
    if(numPages < 3) numPages = 3;

    pages = new Page[numPages];
    for(int i = 0; i < numPages; i++) {
        pages[i].data = new uint8_t[pageSize];
        pages[i].modified = -1;
        pages[i].offset = (OffType)-1;

        if(i) pages[i].prev = &pages[i-1];
        else pages[i].prev = 0;

        if(i+1 < numPages) pages[i].next = &pages[i+1];
        else pages[i].next = 0;
    }
    leastRecentlyUsed = &pages[0];
    mostRecentlyUsed = &pages[numPages-1];

    hashMap = new Hash;
}

HexCache::~HexCache() {
    flush();
    for(int i = 0; i < numPages; i++)
        delete pages[i].data;
    delete [] pages;

    delete &map;
}

HexCache::Page *HexCache::fetchDeep(OffType pageOffset) {
    Page *page;

    Hash::iterator it = map.find(pageOffset);
    if(it != map.end())
#ifdef USE_QHASH
        page = *it;
#else
        page = (*it).second;
#endif
    else page = 0;

    if(page) {
        if(page->prev) // if not leastRecentlyUsed
            page->prev->next = page->next;
        else
            leastRecentlyUsed = page->next;
    } else {
        page = leastRecentlyUsed;
        leastRecentlyUsed = page->next;
        leastRecentlyUsed->prev = 0;
        flushPage(page);

        // rehash page to new offset
#ifdef USE_QHASH
        map.erase(map.find(page->offset));
#else
        map.erase(page->offset);
#endif
        page->offset = pageOffset;
        map[page->offset] = page;

        //fprintf(stderr, "HexCache::fetchDeep(): 0x%08X\n", page->offset*pageSize);
        dsm.read(page->data, page->offset*pageSize, pageSize);

    }
    page->next->prev = page->prev;
    page->prev = mostRecentlyUsed;
    mostRecentlyUsed->next = page;
    mostRecentlyUsed = page;
    mostRecentlyUsed->next = 0;;
    return page;
}

void HexCache::clear() {
    map.clear();
    for(int i = 0; i < numPages; i++) {
        pages[i].offset = (OffType)-1;
        pages[i].modified = -1;
    }
}

void HexCache::flush() {
    for(int i = 0; i < numPages; i++)
        flushPage(&pages[i]);
}

bool HexCache::selfTest() {
    const int testSize = 1024*512;

    uint8_t *buf0 = new uint8_t[testSize];
    uint8_t *buf1 = new uint8_t[testSize];
    HexStaticBuffer hsm0, hsm1;
    srand((int)time(0));

    for(int i = 0; i < testSize; i++)
        buf0[i] = buf1[i] = (uint8_t)(rand()%0xff);

    hsm0.setBuffer(buf0, testSize, 0);
    hsm1.setBuffer(buf1, testSize, 0);
    HexCache cache0(hsm0, testSize/10, 100);
    HexCache cache1(hsm1, testSize/10, 100);
    for(int i = 0; i < testSize*2; i++) {
        int index = rand()%testSize;
        cache0[index] = cache1[index];
    }
    for(int i = testSize-1; i >= 0; i--)
        cache1[i] = cache0[i];

    for(int i = 0; i < testSize; i++)
        cache1[i] = cache1[i];

    cache0.flush();
    cache1.flush();

    bool success = !memcmp(buf0, buf1, testSize);
    delete buf0;
    delete buf1;

    return success;
}

OffType HexCache::getLength() {
    OffType length = dsm.getLength();
    if(dsm.isGrowable()) {
        for(int i = 0; i < numPages; i++) {
            if(pages[i].modified < 0) continue;
            OffType o = pages[i].offset*pageSize + pages[i].modified+1;
            if(o > length) length = o;
        }
    }
    return length;
}

#undef map


static QByteArray getMimeData() {
    const QMimeData *mimeData = qApp->clipboard()->mimeData();
    if(mimeData->hasFormat("application/octet-stream"))
        return mimeData->data("application/octet-stream");
    else if(mimeData->hasFormat("text/plain"))
        return mimeData->data("text/plain");
    return QByteArray();
}

HexCursor::HexCursor(HexDocument *doc,  QObject *parent)
    : QObject(parent)
{
    mAnchor = 0;
    mPosition = 0;
    mTop = 0;
    mDocument = 0;
    setDocument(doc);
}

HexCursor::HexCursor(HexDocument *doc, OffType anchor, OffType position, QObject *parent)
    : QObject(parent)
{
    mAnchor = 0;
    mPosition = 0;
    mTop = 0;
    mDocument = 0;
    setDocument(doc);
    setCursor(anchor, position);
}

HexCursor::~HexCursor() {
    document()->disconnect(this);
    document()->release();
}

QByteArray HexCursor::selectedData() {
    return document()->copy(selectionStart(), selectionEnd());
}

void HexCursor::setDocument(HexDocument *doc) {
    assert(doc != 0);

    doc->reference();
    if(document()) {
        document()->disconnect(this);
        document()->release();
    }
    mDocument = doc;
    connect(document(), SIGNAL(changed()), this, SLOT(documentChanged()));
    mAnchor = 0;
    mPosition = 0;
    emit changed();
}

void HexCursor::setCursor(OffType newAnchor, OffType newPosition) {
    OffType len = document()->length();
    newAnchor = qMax((OffType)0, qMin(len, newAnchor));
    newPosition = qMax((OffType)0, qMin(len, newPosition));
    if(newAnchor != mAnchor || newPosition != mPosition) {
        mAnchor = newAnchor;
        mPosition = newPosition;
        emit changed();
    }
}

void HexCursor::setTop(OffType newTop) {
    newTop = qMin(newTop, document()->length());
    newTop -= newTop&0xf;

    if(newTop != mTop) {
        mTop = newTop;
        emit topChanged();
    }
}

void HexCursor::toStart(bool moveAnchor) {
    setPosition(0, moveAnchor);
}

void HexCursor::toEnd(bool moveAnchor) {
    setPosition(document()->length(), moveAnchor);
}

bool HexCursor::atStart() {
    return position() == 0;
}

bool HexCursor::atEnd() {
    return position() == document()->length();
}

void HexCursor::HexCursor::selectAll() {
    setCursor(0, document()->length());
}

void HexCursor::HexCursor::clearSelection() {
    setCursor(position(), position());
}

void HexCursor::moveSelection(OffType pos) {
    OffType selStart = selectionStart();
    OffType selEnd = selectionEnd();
    OffType newPosition;
    OffType newAnchor;

    // resize vector carefully without breaking direction
    if(pos >= selEnd) {
        newAnchor = selStart;
        newPosition = pos;
    } else if(pos < selStart) {
        newAnchor = selEnd;
        newPosition = pos;
    } else if(pos-selStart >= (selEnd-selStart)/2) {
        newAnchor = selStart;
        newPosition = pos;
    } else {
        newAnchor = selEnd;
        newPosition = pos;
    }
    setCursor(newAnchor, newPosition);
}

void HexCursor::move(int delta, bool moveAnchor) {
    OffType newPosition = position();

    if(delta < 0)
        newPosition -= qMin(newPosition, (OffType)-delta);
    else
        newPosition += qMin(document()->length()-newPosition, (OffType)delta);

    OffType newAnchor = moveAnchor ? position() : anchor();
    setCursor(newAnchor, newPosition);
}

void HexCursor::setPosition(OffType newPosition, bool moveAnchor) {
    OffType newAnchor = moveAnchor ? newPosition : mAnchor;
    setCursor(newAnchor, newPosition);
}

void HexCursor::documentChanged() {
    // make sure we wont get outside of document
    setCursor(anchor(), position());
}

void HexCursor::del() {
    document()->pushCommand(new HexDel(selectionStart(), selectionEnd()));
}

void HexCursor::cut() {
    document()->pushCommand(new HexCut(selectionStart(), selectionEnd()));
}

void HexCursor::copy() {
    document()->copy(selectionStart(), selectionEnd());
}

void HexCursor::copyAsText() {
    document()->copyAsText(selectionStart(), selectionEnd());
}

void HexCursor::paste() {
    document()->pushCommand(new HexPaste(position(), getMimeData()));
}

void HexCursor::pasteOver() {
    document()->pushCommand(new HexPasteOver(position(), getMimeData()));
}

void HexCursor::redo() {
    document()->redo();
}

void HexCursor::undo() {
    document()->undo();
}

HexDataModel::HexDataModel(QObject *parent)
    : QObject(parent)
{
}

HexDataModel::~HexDataModel() {
}


HexDataWindow::HexDataWindow(HexDocument *document) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mHexWidget = new HexWidget(document, this);
    mHexWidget->setEditable(true);
    mainLayout->setMargin(0);
    mainLayout->addWidget(mHexWidget);
    setLayout(mainLayout);
    setWindowTitle(document->name());
    connect(mHexWidget->document(), SIGNAL(nameChanged()), this, SLOT(updateTitle()));
}

void HexDataWindow::updateTitle() {
    setWindowTitle(mHexWidget->document()->name());
}

QList<class QAction *> HexDataWindow::fileActions() {
    return mHexWidget->fileActions();
}

QList<class QAction *> HexDataWindow::editActions() {
    return mHexWidget->editActions();
}

QString HexDataWindow::name() {
    return mHexWidget->document()->name();
}

HexCursor * HexDataWindow::dataCursor() {
    return hexWidget()->cursor();
}


HexDocument::HexDocument(QObject *parent)
    : QObject(parent)
{
    mBuffer = true;
    initFrom(new HexBuffer(this));

    static int sequenceNumber = 1;
    setName(tr("buffer%1").arg(sequenceNumber++));
}

HexDocument::HexDocument(HexDataModel *model, QObject *parent)
    : QObject(parent)
{

    mBuffer = false;

    if(!model->parent())
        model->setParent(this);

    initFrom(model);
}

HexDocument::~HexDocument() {
    delete mCache;
}

void HexDocument::initFrom(HexDataModel *model) {
    mModified = false;
    mModel = model;
    mReadOnly = !mModel->isWriteable();
    mCache = new HexCache(*mModel);
    mCursor = new HexCursor(this);
    mUndoStack = new QUndoStack(this);
    mRefs = 0; // this is last cuz our mCursor also references us
}

bool HexDocument::maybeSave() {
    if(mBuffer && mModified) {
        int ret;
        ret = QMessageBox::warning(qApp->activeWindow(), tr("Save Changes?"),
                                   tr("'%1' has been modified.\n"
                                      "Do you want to save your changes?")
                                   .arg(userFriendlyName()),
                                   QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
        if (ret == QMessageBox::Yes)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    } else {
        mCache->flush();
    }
    return true;
}

bool HexDocument::save() {
    bool ret;
    if(mPath.isEmpty()) {
        if(mBuffer)
            ret = saveAs();
        else {
            mCache->flush();
            ret = true;
            setModified(false);
        }
    } else {
        ret = saveFile(mPath);
    }
    return ret;
}

bool HexDocument::saveAs() {
    QString fileName = QFileDialog::getSaveFileName(qApp->activeWindow(), tr("Save As"), mName);
    if(fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool HexDocument::saveFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(qApp->activeWindow(), tr("Save failed!"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    int length = mCache->getLength();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    for(int i = 0; i < length; i++)
        file.putChar((*mCache)[i]);
    QApplication::restoreOverrideCursor();

    setPath(fileName);
    setModified(false);

    return true;
}

void HexDocument::setModified(bool state) {
    if(!state && mModified) {
        mModified = false;
        emit saved();
    } else {
        mModified = state;
        if(mModified)
            emit changed();
    }
}

QString HexDocument::name() {
    return mName;
}

void HexDocument::setName(QString name) {
    if(mName != name) {
        mName = name;
        emit nameChanged();
    }
}

QString HexDocument::path() {
    return mPath;
}

void HexDocument::setPath(QString path) {
    if(mPath != path) {
        mPath = path;
        mName = QFileInfo(mPath).fileName();
        emit nameChanged();
    }
}

QString HexDocument::userFriendlyName() {
    return mName;

}

void HexDocument::putByte(OffType offset, uint8_t byte) {
    if(mReadOnly) {
        QMessageBox::warning(qApp->activeWindow(), tr("Editing is disabled!"),
            tr("Document you are trying to edit is readonly."));
        return;
    }
    pushCommand(new HexReplaceByte(offset, byte));
}

void HexDocument::pushCommand(HexUndoCommand *cmd) {
    cmd->setDocument(this);
    mUndoStack->push(cmd);
}

QAction *HexDocument::createRedoAction() {
    return mUndoStack->createRedoAction(mUndoStack, tr("&Redo"));
}

QAction *HexDocument::createUndoAction() {
    return mUndoStack->createUndoAction(mUndoStack, tr("&Undo"));
}

void HexDocument::del() {
    cursor()->del();
}

void HexDocument::cut() {
    cursor()->cut();
}

void HexDocument::copy() {
    cursor()->copy();
}

void HexDocument::copyAsText() {
    cursor()->copyAsText();
}

void HexDocument::paste() {
    cursor()->paste();
}

void HexDocument::pasteOver() {
    cursor()->pasteOver();
}

void HexDocument::redo() {
    mUndoStack->redo();
}

void HexDocument::undo() {
    mUndoStack->undo();
}

void HexDocument::paste(OffType where, const QByteArray &what) {
    if(mReadOnly) {
        QMessageBox::warning(qApp->activeWindow(), tr("Editing is disabled!"),
            tr("Document you are trying to edit is readonly."));
        return;
    }

    if(what.size()) {
        mCache->flush();
        mCache->clear();
        if(cursor()->selectionSize())
            del();
        mModel->paste(where, what);
        setModified(true);
    }
}

void HexDocument::pasteOver(OffType where, const QByteArray &what) {
    if(mReadOnly) {
        QMessageBox::warning(qApp->activeWindow(), tr("Editing is disabled!"),
            tr("Document you are trying to edit is readonly."));
        return;
    }

    if(what.size()) {
        mCache->flush();
        mCache->clear();
        mModel->pasteOver(where, what);
        setModified(true);
        cursor()->clearSelection();
    }
}

void HexDocument::del(OffType start, OffType end) {
    if(start == end) return;

    if(mReadOnly) {
        QMessageBox::warning(qApp->activeWindow(), tr("Editing is disabled!"),
            tr("Document you are trying to edit is readonly."));
        return;
    }

    mModel->del(start, end);
    setModified(true);
    cursor()->clearSelection();
}

QByteArray HexDocument::cut(OffType start, OffType end) {

    if(mReadOnly) {
        QMessageBox::warning(qApp->activeWindow(), tr("Editing is disabled!"),
            tr("Document you are trying to edit is readonly."));
        return QByteArray();
    }

    if(start == end) return QByteArray();

    mCache->flush();
    if(start >= mModel->getLength()) return QByteArray();
    mCache->clear();

    QMimeData *mimeData = new QMimeData;
    QByteArray ret = mModel->cut(start, end);
    mimeData->setData("application/octet-stream", ret);
    qApp->clipboard()->setMimeData(mimeData);
    setModified(true);
    cursor()->clearSelection();
    return ret;
}

QByteArray HexDocument::copy(OffType start, OffType end) {
    mCache->flush();
    QMimeData *mimeData = new QMimeData;
    QByteArray ret = mModel->copy(start, end);
    mimeData->setData("application/octet-stream", ret);
    qApp->clipboard()->setMimeData(mimeData);
    return ret;
}

QByteArray HexDocument::copyAsText(OffType start, OffType end) {
    mCache->flush();
    QMimeData *mimeData = new QMimeData;
    QByteArray ret = mModel->copy(start, end);
    mimeData->setData("text/plain", ret);
    qApp->clipboard()->setMimeData(mimeData);
    return ret;
}

uint8_t HexDocument::replaceByte(OffType where, uint8_t with) {
    uint8_t what = (*cache())[where];
    (*cache())[where] = with;
    setModified(true);
    cursor()->clearSelection();
    return what;
}


HexTextWindow::HexTextWindow(QString text) {
    static int sequenceNumber = 1;
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mTextEdit = new QTextEdit(this);
    mainLayout->addWidget(mTextEdit);
    setLayout(mainLayout);
    setWindowTitle(name());
    setName(tr("TextBuf%1").arg(sequenceNumber++));
    mTextCursor = new HexTextCursor(textEdit(), this);

    textEdit()->setPlainText(text);

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save changes"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    mFileActions.append(saveAct);

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
    mFileActions.append(saveAsAct);

    undoAct = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last editing action"));
    connect(undoAct, SIGNAL(triggered()), document(), SLOT(redo()));
    connect(textEdit(), SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)));
    undoAct->setEnabled(document()->isUndoAvailable());
    mEditActions.append(undoAct);

    redoAct = new QAction(QIcon(":/images/redo.png"), tr("&Redo"), this);
    redoAct->setShortcut(tr("Ctrl+R"));
    redoAct->setStatusTip(tr("Redo last editing action"));
    connect(redoAct, SIGNAL(triggered()), document(), SLOT(redo()));
    connect(textEdit(), SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)));
    redoAct->setEnabled(document()->isRedoAvailable());
    mEditActions.append(redoAct);

    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcut(tr("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), textEdit(), SLOT(cut()));
    mEditActions.append(cutAct);

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit(), SLOT(copy()));
    mEditActions.append(copyAct);

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcut(tr("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste clipboard's contents into current selection"));
    connect(pasteAct, SIGNAL(triggered()), textEdit(), SLOT(paste()));
    mEditActions.append(pasteAct);

    connect(document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));

#ifdef __linux__
    QFont font("monospace");
    font.setPointSize(10);
#else
        // other systems may not have this nice font, but most likely have some courier...
    QFont font("courier");
    font.setWeight(QFont::Bold);
    font.setPointSize(10);
#endif
    setFont(font);
}


void HexTextWindow::updateTitle() {
    setWindowTitle(name() + "[*]");
}

void HexTextWindow::documentWasModified() {
    setWindowModified(document()->isModified());
}

QList<class QAction *> HexTextWindow::fileActions() {
    return mFileActions;
}

QList<class QAction *> HexTextWindow::editActions() {
    return mEditActions;
}

QString HexTextWindow::name() {
    return mName;
}

void HexTextWindow::setName(const QString &name) {
    mName = name;
    updateTitle();
}

QString HexTextWindow::path() {
    return mName;
}

void HexTextWindow::setPath(const QString &name) {
    mPath = name;
    setName(QFileInfo(path()).fileName());
}

QString HexTextWindow::text() {
    return textEdit()->toPlainText();
}

void HexTextWindow::setText(const QString &text) {
    textEdit()->setPlainText(text);
}

void HexTextWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool HexTextWindow::loadFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Load failed!"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    textEdit()->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setPath(fileName);
    setWindowModified(0);

    return true;
}

bool HexTextWindow::maybeSave() {
    if(document()->isModified()) {
        int ret;
        ret = QMessageBox::warning(qApp->activeWindow(), tr("Save Changes?"),
                                   tr("'%1' has been modified.\n"
                                      "Do you want to save your changes?")
                                   .arg(name()),
                                   QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
        if (ret == QMessageBox::Yes)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

bool HexTextWindow::saveFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(qApp->activeWindow(), tr("Save failed!"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEdit()->toPlainText();
    QApplication::restoreOverrideCursor();

    setPath(fileName);

    return true;
}

bool HexTextWindow::save() {
    bool ret;
    if(mPath.isEmpty()) {
        ret = saveAs();
    } else {
        ret = saveFile(mPath);
    }
    return ret;
}

bool HexTextWindow::saveAs() {
    QString fileName = QFileDialog::getSaveFileName(qApp->activeWindow(), tr("Save As"), mName);
    if(fileName.isEmpty())
        return false;

    return saveFile(fileName);
}





#define d ((HexWidgetPrivate*)mPrivate)
#define q ((HexWidget*)mPublic)

#define LOAD_INT(name,default) name = settings.value(#name, default).toInt();
#define LOAD_FONT(name,default) name = settings.value(#name, default).value<QFont>();
#define LOAD_COLOR(name, default) name = settings.value(#name, default).value<QColor>();

#define SAVE(name) settings.setValue(#name, name)


HexWidget::HexWidget(HexDocument *doc, QWidget *parent)
    : QWidget(parent)
{
    mPrivate = new HexWidgetPrivate(this, doc);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(d);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    connect(d, SIGNAL(offsetChanged(OffType)), this, SIGNAL(offsetChanged(OffType)));
}

HexWidget::~HexWidget() {
}

HexDocument *HexWidget::document() {return d->document();}
bool HexWidget::isEditable() {return d->isEditable();}
void HexWidget::setEditable(bool state) {d->setEditable(state);}
HexCursor *HexWidget::cursor() {return d->cursor();}
QWidget *HexWidget::configPanel() {return HexWidgetPrivate::configPanel();}

QList<class QAction *> HexWidget::fileActions() {return d->fileActions();}
QList<class QAction *> HexWidget::editActions() {return d->editActions();};

void HexWidget::cut() {d->cut();}
void HexWidget::copy() {d->copy();}
void HexWidget::copyAsText() {d->copyAsText();}
void HexWidget::paste() {d->paste();}
void HexWidget::pasteOver() {d->pasteOver();}
bool HexWidget::save() {return d->save();}
bool HexWidget::saveAs() {return d->saveAs();}


//////////////////////////////////// Private Stuff ///////////////////////////////////////


static inline int binToHex(int b, bool upperCase=true) {
    if(upperCase) return b < 10 ? b+'0' : b-10+'A';
    else return b < 10 ? b+'0' : b-10+'a';
}


static inline int hexToBin(int b) {
    if('0' <= b && b <= '9') return b-'0';
    else if('A' <= b && b <= 'F') return b-'A'+10;
    else if('a' <= b && b <= 'f') return b-'a'+10;
    return 0;
}

static inline QString RectToString(QRect r) {
    return QString("(%1,%2) -> (%3,%4)").arg(r.left()).arg(r.top()).
           arg(r.width()).arg(r.height());
}

uint qHash(const HexCellDesc &cd) {
    return cd.hash();
}


HexWidgetPrivate::HexWidgetPrivate(HexWidget *pub, HexDocument *doc)
    : QFrame(pub)
{
    mPublic = pub;

    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    mCacheSize = 512;

    pixes = new CharPix[mCacheSize];
    for(int i = 0; i < mCacheSize; i++) {
        if(i) pixes[i].prev = &pixes[i-1];
        else pixes[i].prev = 0;

        if(i+1 < mCacheSize) pixes[i].next = &pixes[i+1];
        else pixes[i].next = 0;
    }
    lru = &pixes[0];
    mru = &pixes[mCacheSize-1];

    mCellSide = -1;
    readSettings();

    mEditable = false;
    mInstanceList.append(this);

    if(!doc) doc = new HexDocument;

    doc->reference();
    mDocument = doc;

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save changes"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    mFileActions.append(saveAct);

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
    mFileActions.append(saveAsAct);

    undoAct = document()->createUndoAction();
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setIcon(QIcon(":/images/undo.png"));
    undoAct->setStatusTip(tr("Undo last editing action"));
    undoAct->setParent(this);
    mEditActions.append(undoAct);

    redoAct = document()->createRedoAction();
    redoAct->setShortcut(tr("Ctrl+R"));
    redoAct->setIcon(QIcon(":/images/redo.png"));
    redoAct->setStatusTip(tr("Redo last editing action"));
    redoAct->setParent(this);
    mEditActions.append(redoAct);

    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcut(tr("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));
    mEditActions.append(cutAct);

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));
    mEditActions.append(copyAct);

    copyAsTextAct = new QAction(QIcon(":/images/text.png"), tr("Copy &As Text"), this);
    copyAsTextAct->setShortcut(tr("Ctrl+T"));
    copyAsTextAct->setStatusTip(tr("Copy current selection's contents as text"));
    connect(copyAsTextAct, SIGNAL(triggered()), this, SLOT(copyAsText()));
    mEditActions.append(copyAsTextAct);

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcut(tr("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste clipboard's contents into current selection"));
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));
    mEditActions.append(pasteAct);

    pasteOverAct = new QAction(QIcon(":/images/paste_over.png"), tr("Paste &Over"), this);
    pasteOverAct->setShortcut(tr("Ctrl+O"));
    pasteOverAct->setStatusTip(tr("Paste clipboard's contents over data under cursor"));
    connect(pasteOverAct, SIGNAL(triggered()), this, SLOT(pasteOver()));
    mEditActions.append(pasteOverAct);

    // we use our own private cursor shared between all views
    mCursor = new HexCursor(doc, this);

    connect(cursor(), SIGNAL(changed()), this, SLOT(updateActions()));
    connect(HexSettings::instance(), SIGNAL(changed()), this, SLOT(readSettings()));
    connect(document(), SIGNAL(changed()), this, SLOT(documentChanged()));
    connect(document(), SIGNAL(saved()), this, SLOT(documentSaved()));
    connect(document(), SIGNAL(nameChanged()), this, SLOT(updateTitle()));

    updateActions();
    updateTitle();

    setWindowModified(document()->isModified());

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(frameWidth());
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    mOffsetView = new HexOffsetView(this);
    mDataView = new HexDataView(this);
    mTextView = new HexTextView(this);

    layout->addWidget(new Column(mOffsetView));
    layout->addWidget(new Column(mDataView));
    layout->addWidget(new Column(mTextView));

    //QSplitter *splitter = new QSplitter(this);
    //splitter->addWidget(hexOffsetView);
    //splitter->addWidget(hexDataView);
    //splitter->addWidget(hexTextView);
    //layout->addWidget(splitter);

    mOffsetView->adjustSize();
}

HexWidgetPrivate::~HexWidgetPrivate() {
    delete [] pixes;
    mInstanceList.removeAll(this);
    document()->release();
}

bool HexWidgetPrivate::isEditable() {
    return mEditable && document()->isEditable();
}

void HexWidgetPrivate::setEditable(bool state) {
    mEditable = state;
    updateActions();
}

bool HexWidgetPrivate::event(QEvent * e) {
    if (e->type() == QEvent::FontChange) {
        mOffsetView->setFont(font());
        mDataView->setFont(font());
        mTextView->setFont(font());
        return true;
    }

    return QWidget::event(e);
}

HexDocument *HexWidgetPrivate::document() {
    return mDocument;
}

void HexWidgetPrivate::updateActions() {
    if(cursor()->hasSelection()) {
        cutAct->setEnabled(isEditable() && document()->isCuttable());
        copyAct->setEnabled(true);
        copyAsTextAct->setEnabled(true);
        pasteAct->setEnabled(isEditable() && document()->isCuttable());
        pasteOverAct->setEnabled(isEditable());
    } else {
        cutAct->setEnabled(false);
        copyAct->setEnabled(false);
        copyAsTextAct->setEnabled(false);
        pasteAct->setEnabled(isEditable() && document()->isCuttable());
        pasteOverAct->setEnabled(isEditable());
    }
}

bool HexWidgetPrivate::save() {
    return document()->save();
}

bool HexWidgetPrivate::saveAs() {
    return document()->saveAs();
}

void HexWidgetPrivate::closeEvent(QCloseEvent *event) {
    int refs = 0;

    foreach(HexWidgetPrivate *mdi, mInstanceList)
        if(document() == mdi->document())
            refs++;

    if(refs > 1) event->accept();
    else if(document()->maybeSave()) event->accept();
    else event->ignore();
}

void HexWidgetPrivate::documentChanged() {
    setWindowModified(document()->isModified());
}

void HexWidgetPrivate::documentSaved() {
    setWindowModified(document()->isModified());
}

void HexWidgetPrivate::updateTitle() {
    setWindowTitle(document()->userFriendlyName()+"[*]");
}

void HexWidgetPrivate::cut() {
    cursor()->cut();
}

void HexWidgetPrivate::copy() {
    cursor()->copy();
}

void HexWidgetPrivate::copyAsText() {
    cursor()->copyAsText();
}

void HexWidgetPrivate::paste() {
    cursor()->paste();
}

void HexWidgetPrivate::pasteOver() {
    cursor()->pasteOver();
}

QWidget *HexWidgetPrivate::configPanel() {
    return HexSettings::instance()->configPanel();
}



void HexWidgetPrivate::readSettings() {
    QSettings settings;
    settings.beginGroup("HexWidget");

    QFont newFont = settings.value("mFont", HexSettings::defFont()).value<QFont>();

    LOAD_INT(mCols, HexSettings::defCols());
    LOAD_INT(mGroupCols, HexSettings::defGroupCols());

    LOAD_COLOR(mOffsetFg,	HexSettings::defOffsetFg());
    LOAD_COLOR(mOffsetBg,	HexSettings::defOffsetBg());
    LOAD_COLOR(mDataFgEven,	HexSettings::defDataFgEven());
    LOAD_COLOR(mDataFgOdd,	HexSettings::defDataFgOdd());
    LOAD_COLOR(mDataBgEven,	HexSettings::defDataBgEven());
    LOAD_COLOR(mDataBgOdd,	HexSettings::defDataBgOdd());
    LOAD_COLOR(mTextFgEven,	HexSettings::defTextFgEven());
    LOAD_COLOR(mTextBgEven,	HexSettings::defTextBgEven());
    LOAD_COLOR(mTextFgOdd,	HexSettings::defTextFgOdd());
    LOAD_COLOR(mTextBgOdd,	HexSettings::defTextBgOdd());
    LOAD_COLOR(mSelFg,		HexSettings::defSelFg());
    LOAD_COLOR(mSelBg,		HexSettings::defSelBg());
    LOAD_COLOR(mCursorFg,	HexSettings::defCursorFg());
    LOAD_COLOR(mCursorBg,	HexSettings::defCursorBg());

    settings.endGroup();

    static char hexChars[] = "0123456789ABCDEF";

    QFontMetrics fontMetrics(newFont);
    int newCellSide = fontMetrics.ascent();
    for(int i = 0; i < 16; i++) {
        int width = fontMetrics.width(hexChars[i])
            /*+ fontMetrics.leftBearing(ch)
            + fontMetrics.rightBearing(ch)*/;
        if(width > newCellSide)
            newCellSide = width;
    }

    mCellMargin = (int)(newCellSide/1.5);
    newCellSide += mCellMargin; // additional margin for readability
    newCellSide += newCellSide&1; // make even

    if(newCellSide != mCellSide || newFont != mFont) {
        mCellSide = newCellSide;
        mFont = newFont;
        clearFontCache();
        adjustSize();
        update();
        emit cellFontChanged();
    }
    update();
}


void HexWidgetPrivate::clearFontCache() {
    mPixHash.clear();
    for(int i = 0; i < mCacheSize; i++)
        if(pixes[i].cd.isValid()) {
            pixes[i].cd.makeInvalid();
            pixes[i].pix = QPixmap();
        }
}

QPixmap HexWidgetPrivate::fetchCell(const HexCellDesc &cd) {
    Q_ASSERT(cd.isValid());

    if(mru->cd == cd) return mru->pix;

    CharPix *pix;
    QHash<HexCellDesc, CharPix*>::iterator it = mPixHash.find(cd);
    if(it != mPixHash.end()) {
        pix = *it;
        if(pix->prev) // if not leastRecentlyUsed
            pix->prev->next = pix->next;
        else
            lru = pix->next;
    } else {
        pix = lru;
        lru = pix->next;
        lru->prev = 0;
        mPixHash.erase(mPixHash.find(pix->cd));
        pix->cd = cd;
        mPixHash[pix->cd] = pix;
        pix->pix = drawFontCell(pix->cd);
    }
    pix->next->prev = pix->prev;
    pix->prev = mru;
    mru->next = pix;
    mru = pix;
    mru->next = 0;
    return pix->pix;
}

QPixmap HexWidgetPrivate::drawFontCell(const HexCellDesc &cd) {
    QColor fg, bg;
    int ch = cd.value();
    int width = charWidth();

    int flags;
    switch(cd.alignment()) {
    case HexCellDesc::LEFT:
        flags = Qt::AlignRight;
        break;
    case HexCellDesc::RIGHT:
        flags = Qt::AlignLeft;
        break;
    case HexCellDesc::CENTER:
        flags = Qt::AlignCenter;
        break;
    default:
        flags = 0;
        ;
    }

    QPixmap pix(width, mCellSide);
    QPainter painter(&pix);
    painter.fillRect(pix.rect(), cd.bg());

    QFontMetrics fontMetrics(mFont);
    if(ch != ' ' && fontMetrics.inFont(ch)) {
        painter.setFont(mFont);
        painter.setPen(cd.fg());
        painter.drawText(QRect(0, 0, width-1, mCellSide), flags, QString(ch));
    }

    if(cd.frameEnabled()) {
        // non-focus cursor frame
        painter.setPen(mCursorBg);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(0,0,width-1,mCellSide-1);
    }

    return pix;
}


//////////////////////////////// HexView Related Stuff ////////////////////////////

HexView::HexView(HexWidgetPrivate *p, Qt::WindowFlags f)
        : 	mParent(p)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    connect(parent(), SIGNAL(cellFontChanged()), this, SLOT(updateSize()));
    connect(cursor(), SIGNAL(changed()), this, SLOT(cursorChanged()));
    connect(cursor(), SIGNAL(topChanged()), this, SLOT(update()));
    connect(document(), SIGNAL(changed()), this, SLOT(updateView()));
}

HexView::~HexView() {
}

HexDocument *HexView::document() const {
    return parent()->document();
}

HexCursor *HexView::cursor() const {
    return parent()->cursor();
}

void HexView::updateSize() {
    setMinimumWidth(widgetWidth());
    adjustSize();
    updateView();
}

void HexView::updateView() {
    update();
}

void HexView::cursorChanged() {
    OffType selStart = cursor()->selectionStart();
    OffType selEnd = cursor()->selectionEnd();

    bool newSelectionVisible = inView(selStart) || inView(selEnd);

    if (newSelectionVisible || newSelectionVisible != mSelectionVisible)
        update();

    mSelectionVisible = newSelectionVisible;
}

QSize HexView::sizeHint() const {
    return QSize(widgetWidth(), qMax(32,rows())*parent()->charHeight());
}

void HexView::resizeEvent(QResizeEvent * event) {
    //fprintf(stderr, "%d\n", event->size().height());
    //updateSize();
}

bool HexView::event(QEvent * e) {
    /*if (e->type() == QEvent::FontChange) {
        parent()->changeFont(font());
        return true;
    }*/

    return QWidget::event(e);
}

void HexView::wheelEvent(QWheelEvent *e) {
    int numDegrees = e->delta() / 8;
    int numSteps = numDegrees / 15;

    if (e->orientation() == Qt::Horizontal) {
        bool moveAnchor = (e->modifiers()&Qt::ShiftModifier) == 0;
        cursor()->move(numSteps, moveAnchor);
    } else {
        scroll(-numSteps*qApp->wheelScrollLines());
    }
}

void HexView::scroll(int deltaInt) {
    OffType delta = deltaInt*cols();
    OffType newTop = cursor()->top();

    if (delta > 0) newTop += delta;
    else newTop -= qMin(newTop, (OffType)-delta);

    cursor()->setTop(newTop);
}

void HexView::moveCursor(int col, int row, bool moveAnchor) {
    if (row < 0) row = -qMin(cursor()->position()/cols(), (OffType)-row);
    OffType delta = (OffType)row*cols() + col;
    cursor()->setPosition(cursor()->position()+delta, moveAnchor);
}

void HexView::keyPressEvent(QKeyEvent * e) {
    if (!parent()->isEditable()) {
        switch (e->key()) {
        case Qt::Key_Down:
            scroll(1);
            break;
        case Qt::Key_Up:
            scroll(-1);
            break;
        case Qt::Key_PageDown:
            scroll(rows());
            break;
        case Qt::Key_PageUp:
            scroll(-rows());
            break;
        default:
            QWidget::keyPressEvent(e);
        }
        return;
    }

    bool moveAnchor = (e->modifiers()&Qt::ShiftModifier) == 0;
    switch (e->key()) {
    case Qt::Key_Right:
        moveCursor( 1,  0,      moveAnchor);
        break;
    case Qt::Key_Left:
        moveCursor(-1,  0,      moveAnchor);
        break;
    case Qt::Key_Down:
        moveCursor( 0,  1,      moveAnchor);
        break;
    case Qt::Key_Up:
        moveCursor( 0, -1,      moveAnchor);
        break;
    case Qt::Key_PageDown:
        moveCursor( 0, rows(),  moveAnchor);
        break;
    case Qt::Key_PageUp:
        moveCursor( 0, -rows(), moveAnchor);
        break;
    case Qt::Key_Home:
        if (e->modifiers()&Qt::ControlModifier)
            cursor()->toStart(moveAnchor);
        else
            cursor()->setCursor(cursor()->position()/cols()*cols(), moveAnchor);
        break;

    case Qt::Key_End:
        if (e->modifiers()&Qt::ControlModifier)
            cursor()->toEnd(moveAnchor);
        else
            cursor()->setCursor(cursor()->position()
                    + cols()-1-cursor()->position()%cols(), moveAnchor);
        break;

    case Qt::Key_Alt:
        break;

    case Qt::Key_Shift:
        break;

    default:
        QString text = e->text();
        if (!text.isEmpty())
            input(((QChar)text[0]).toAscii());
        else
            QWidget::keyPressEvent(e);
    }
}

void HexView::contextMenuEvent(QContextMenuEvent *event) {
    QMenu editMenu;

    QList<QAction *> actions = parent()->editActions();
    foreach(QAction *action, actions)
    editMenu.addAction(action);

    editMenu.exec(event->globalPos());
}









HexOffsetView::HexOffsetView(HexWidgetPrivate *p, Qt::WindowFlags f)
    : HexView(p, f)
{
    connect(document(), SIGNAL(changed()), this, SLOT(documentChanged()));
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    updateSize();
}


void HexOffsetView::documentChanged() {
    mSelectionTracing = false;
}

void HexOffsetView::input(char ch) {
}

OffType HexOffsetView::screenToRelative(QPoint point) {
    int x = point.x();
    int y = point.y();

    if(x < 0) x = 0;
    else if(x > widgetWidth()) x = widgetWidth();

    // reverse hex area pixel coord to column and row with round-up

    int row = y/parent()->charHeight();
    return row*cols();
}

OffType HexOffsetView::relativeToGlobal(int rel) {
    OffType offset = start();
    if (rel < 0) offset -= qMin(offset, (OffType)-rel);
    else offset += rel;
    return offset;
}

void HexOffsetView::mousePressEvent(QMouseEvent *e) {
    if(e->button() & Qt::LeftButton) {
        OffType offset = relativeToGlobal(screenToRelative(e->pos()));
        offset -= offset%cols();
        OffType anchor = cursor()->anchor();
        OffType position = cursor()->position();

        if(e->modifiers()&Qt::ShiftModifier) {
            if(anchor < position) {
                if(offset < anchor || offset-anchor < (position-anchor)/2)
                    anchor = offset;
                else
                    position = offset+cols();
            } else {
                if(offset < position || offset-position < (anchor-position)/2)
                    position = offset;
                else
                    anchor = offset+cols();
            }
        } else if(e->modifiers()&Qt::ControlModifier) {
            if(anchor < position) {
                if(anchor <= offset) position = offset + cols();
                else {
                    position = offset - offset%cols();
                    anchor += cols();
                }
            } else {
                if(offset < anchor) position = offset;
                else {
                    position = offset + cols();
                    anchor -= cols();
                }
            }
        } else {
            anchor = offset - offset%cols();
            position = anchor + cols();
            mSelInverted = false;
            mSelectionTracing = true;
        }
        cursor()->setCursor(anchor, position);

        e->accept();
    } else {
        QWidget::mousePressEvent(e);
    }
}

void HexOffsetView::mouseReleaseEvent(QMouseEvent *e) {
    if(mSelectionTracing && (e->button() & Qt::LeftButton)) {
        mSelectionTracing = false;
        e->accept();
    } else {
        QWidget::mouseReleaseEvent(e);
    }
}

void HexOffsetView::mouseMoveEvent(QMouseEvent *e) {
    if(mSelectionTracing) {
        OffType position = relativeToGlobal(screenToRelative(e->pos()));
        OffType anchor = cursor()->anchor();
        if(anchor < position)  {
            position = (position+cols()-1)/cols()*cols();
            if(mSelInverted) {
                anchor -= cols();
                mSelInverted = false;
            }
        } else
            if(!mSelInverted) {
                anchor += cols();
                mSelInverted = true;
            }

        cursor()->setCursor(anchor, position);
        e->accept();
    } else {
        QWidget::mouseMoveEvent(e);
    }
}

int HexOffsetView::offsetSize() const {
    int size;
    return 8;
    OffType offset = end();

    if(offset <= 0xff) size = 2;
    else if(offset <= 0xffffLL) size = 4;
    else if(offset <= 0xffffffLL) size = 6;
    else if(offset <= 0xffffffffLL) size = 8;
    else if(offset <= 0xffffffffffLL) size = 10;
    else if(offset <= 0xffffffffffffLL) size = 12;
    else if(offset <= 0xffffffffffffffLL) size = 14;
    else size = 16;
    return size;
}

void HexOffsetView::paintEvent(QPaintEvent * event) {
    OffType selStart = 1;
    OffType selEnd = 0;

    if(cursor()->hasSelection()) {
        selStart = cursor()->selectionStart();
        selEnd = cursor()->selectionEnd();
    }

    QPainter painter(this);

    int cols = HexView::cols();
    int offsz = offsetSize();
    OffType endOffset = end();
    int y = 0;


    for(OffType offset = start(); offset < endOffset; offset += cols) {
        int x = 0;
        for(int i = offsz-1; i >= 0; i--) {
            if(i != offsz-1 && !((i+1)%4)) {
                painter.drawPixmap(x, y, parent()->fetchCell(
                            HexCellDesc(':', parent()->offsetFg(), parent()->offsetBg())));
                x += charWidth();
            }
            char digit = binToHex((int)((offset >> (i*4))&0xf));
            painter.drawPixmap(x, y, parent()->fetchCell(
                        HexCellDesc(digit,  parent()->offsetFg(), parent()->offsetBg())));
            x += charWidth();
        }
        y += parent()->charHeight();
    }
}




HexDataView::HexDataView(HexWidgetPrivate *p, Qt::WindowFlags f)
    : HexView(p, f)
{
    mCursorFlashTimer = new QTimer(this);
    connect(mCursorFlashTimer, SIGNAL(timeout()), this, SLOT(cursorFlash()));
    connect(cursor(), SIGNAL(changed()), this, SLOT(updateCursor()));

    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));

    mSelectionTracing = false;
    mCursorFlash = false;
    mCursorSubPos = 0;

    updateSize();
}


void HexDataView::cursorFlash() {
    if (QApplication::focusWidget() == this) {
        mCursorFlash = !mCursorFlash;
        update();
    } else {
        mCursorFlash = 0;
    }
}

void HexDataView::setCursorFlash(bool state) {
    if (state) {
        mCursorFlashTimer->start(qApp->cursorFlashTime());
    } else {
        mCursorFlashTimer->stop();
        mCursorFlash = false;
    }
}

void HexDataView::updateCursor() {
    mCursorSubPos = 0;
    if (QApplication::focusWidget() == this) {
        setCursorFlash(false);
        setCursorFlash(true);

        OffType position = cursor()->position();
        OffType end = start()+(height()/parent()->charHeight()-1)*cols();
        if(position > end)
            cursor()->setTop(start()+position-end);
        else if(position < start())
            cursor()->setTop(position);
    }
}

OffType HexDataView::screenToRelative(QPoint point) {
    int x = point.x();
    int y = point.y();

    if (x < 0) x = 0;
    else if (x > widgetWidth()) x = widgetWidth();

    // reverse hex area pixel coord to column and row with round-up
    int col = x/(charWidth()*2);

    int j = 0;
    for (int i = 0; i < col; i++, j++) {
        if (j == parent()->groupCols()) {
            j = 0;
            col--;
        }
    }
    if (j == groupCols()) col--;

    int row = y/parent()->charHeight();

    return row*cols() + col;
}

OffType HexDataView::relativeToGlobal(int rel) {
    OffType offset = start();
    if (rel < 0) offset -= qMin(offset, (OffType)-rel);
    else offset += rel;
    return offset;
}

void HexDataView::input(char ch) {
    //fprintf(stderr, "%c\n", ch);

    uint8_t byte;

    if (cursor()->position() < length())
        byte = (*document())[cursor()->position()];
    else
        byte = 0;

    uint8_t in = hexToBin(ch);
    if (mCursorSubPos) {
        (*document())[cursor()->position()] = (byte&0xf0) | in;
        cursor()->move(1, true);
    } else {
        (*document())[cursor()->position()] = (byte&0xf) | in<<4;
        mCursorSubPos = 1;
        setCursorFlash(false);
        setCursorFlash(true);
        update();
    }
}

void HexDataView::paintEvent(QPaintEvent * event) {
    OffType selStart = 1;
    OffType selEnd = 0;

    if (cursor()->hasSelection()) {
        selStart = cursor()->selectionStart();
        selEnd = cursor()->selectionEnd();
    }

    QPainter painter(this);

    int cols = HexView::cols();
    OffType endOffset = end();
    OffType len = length();
    bool haveFocus = QApplication::focusWidget() == this;
    int y = 0;

    for (OffType offset = start(); offset < endOffset; offset += cols) {
        QColor bg((offset/cols)%2 ? parent()->dataBgOdd() : parent()->dataBgEven());

        int x = 0;
        for (int i = 0; i < cols; i++) {
            QColor fg(i%2 ? parent()->dataFgOdd(): parent()->dataFgEven());
            HexCellDesc l, r;
            l.setAlignment(l.LEFT);
            r.setAlignment(r.RIGHT);

            if (selStart <= offset+i && offset+i < selEnd) {
                l.setFgBg(parent()->selFg(), parent()->selBg());
                r.setFgBg(parent()->selFg(), parent()->selBg());
            } else {
                l.setFgBg(fg, bg);
                r.setFgBg(fg, bg);
            }

            if (!(i % groupCols()) && i) { // draw group separator?
                HexCellDesc scd(l);
                scd.setValue(' ');

                OffType prevOff = offset+i-1;
                if (!prevOff || selStart > prevOff || prevOff > selEnd)
                    scd.setFgBg(fg, bg);

                QPixmap pix = parent()->fetchCell(scd);
                painter.drawPixmap(x, y, pix);
                x += charWidth();
                painter.drawPixmap(x, y, pix);
                x += charWidth();
            }

            if (offset+i < len) {
                uint8_t byte = (*document())[offset+i];
                l.setValue(binToHex(byte>> 4));
                r.setValue(binToHex(byte&0xf));
            } else {
                l.setValue(' ');
                r.setValue(' ');
            }

            if (offset+i == cursor()->position() && parent()->isEditable()) {
                HexCellDesc &cd = mCursorSubPos ? r : l;

                if (!haveFocus || mCursorFlash) cd.enableFrame(true);
                else cd.setFgBg(parent()->cursorFg(), parent()->cursorBg());
            }

            painter.drawPixmap(x, y, parent()->fetchCell(l));
            x += charWidth();
            painter.drawPixmap(x, y, parent()->fetchCell(r));
            x += charWidth();
        }

        y += parent()->charHeight();
    }
}


void HexDataView::mousePressEvent(QMouseEvent *e) {
    if (e->button() & Qt::LeftButton) {
        OffType offset = relativeToGlobal(screenToRelative(e->pos()));

        if (cursor()->selectionSize() && e->modifiers()&Qt::ShiftModifier)
            cursor()->moveSelection(offset);
        else {
            bool moveAnchor = !(e->modifiers()&Qt::ControlModifier);
            cursor()->setPosition(offset, moveAnchor);
            mSelectionTracing = true;
        }

        e->accept();
    } else {
        QWidget::mousePressEvent(e);
    }
}

void HexDataView::mouseReleaseEvent(QMouseEvent *e) {
    if (mSelectionTracing && (e->button() & Qt::LeftButton)) {
        mSelectionTracing = false;
        e->accept();
    } else {
        QWidget::mouseReleaseEvent(e);
    }
}

void HexDataView::mouseMoveEvent(QMouseEvent *e) {
    if (mSelectionTracing) {
        OffType offset = relativeToGlobal(screenToRelative(e->pos()));
        cursor()->setPosition(offset, false);
        e->accept();
    } else {
        QWidget::mouseMoveEvent(e);
    }

}




HexTextView::HexTextView(HexWidgetPrivate *parent, Qt::WindowFlags f)
    : HexView(parent, f)
{
    mCursorFlashTimer = new QTimer(this);
    connect(mCursorFlashTimer, SIGNAL(timeout()), this, SLOT(cursorFlash()));

    connect(cursor(), SIGNAL(changed()), this, SLOT(updateCursor()));

    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding));
    mSelectionTracing = false;
    mCursorFlash = false;
}


void HexTextView::cursorFlash() {
    if (QApplication::focusWidget() == this) {
        mCursorFlash = !mCursorFlash;
        update();
    } else {
        mCursorFlash = 0;
    }
}

void HexTextView::setCursorFlash(bool state) {
    if (state) {
        mCursorFlashTimer->start(qApp->cursorFlashTime());
    } else {
        mCursorFlashTimer->stop();
        mCursorFlash = false;
    }
}

void HexTextView::updateCursor() {
    if (QApplication::focusWidget() == this) {
        setCursorFlash(false);
        setCursorFlash(true);

        OffType position = cursor()->position();
        OffType end = start()+(height()/parent()->charHeight()-1)*cols();
        if(position > end)
            cursor()->setTop(start()+position-end);
        else if(position < start())
            cursor()->setTop(position);
    }
}

void HexTextView::input(char ch) {
    //fprintf(stderr, "%c\n", ch);

    (*document())[cursor()->position()] = ch;
    cursor()->move(1, true);
}

OffType HexTextView::screenToRelative(QPoint point) {
    int x = point.x();
    int y = point.y();

    if(x < 0) x = 0;
    else if(x > widgetWidth()) x = widgetWidth();

    // reverse hex area pixel coord to column and row with round-up
    int col = x/charWidth();

    int row = y/parent()->charHeight();

    return row*cols() + col;
}


OffType HexTextView::relativeToGlobal(int rel) {
    OffType offset = start();
    if (rel < 0) offset -= qMin(offset, (OffType)-rel);
    else offset += rel;
    return offset;
}

void HexTextView::paintEvent(QPaintEvent * event) {
    OffType selStart = 1;
    OffType selEnd = 0;

    if(cursor()->hasSelection()) {
        selStart = cursor()->selectionStart();
        selEnd = cursor()->selectionEnd();
    }

    QPainter painter(this);

    int cols = HexView::cols();
    OffType len = length();
    OffType endOffset = end();
    bool haveFocus = QApplication::focusWidget() == this;
    int y = 0;

    //fprintf(stderr, "%d\n", width());
    for(OffType offset = start(); offset < endOffset; offset += cols) {
        QColor bg((offset/cols)%2 ? parent()->textBgOdd() : parent()->textBgEven());

        int x = 0;
        for(int i = 0; i < cols; i++) {
            QColor fg(i%2 ? parent()->textFgOdd(): parent()->textFgEven());
            HexCellDesc cd;

            if(selStart <= offset+i && offset+i < selEnd)
                cd.setFgBg(parent()->selFg(), parent()->selBg());
            else
                cd.setFgBg(fg, bg);

            if(offset+i < len) {
                cd.setValue((uint8_t)(*document())[offset+i]);
            } else {
                cd.setValue(' ');
            }

            if(offset+i == cursor()->position() && parent()->isEditable()) {
                if(!haveFocus || mCursorFlash) cd.enableFrame(true);
                else cd.setFgBg(parent()->cursorFg(), parent()->cursorBg());
            }

            painter.drawPixmap(x, y, parent()->fetchCell(cd));
            x += charWidth();
        }
        while(x < width()) {
            painter.drawPixmap(x, y, parent()->fetchCell(HexCellDesc(' ', QColor(), bg)));
            x += charWidth();
        }
        y += parent()->charHeight();
    }
}


void HexTextView::mousePressEvent(QMouseEvent *e) {
    if (e->button() & Qt::LeftButton) {
        OffType offset = relativeToGlobal(screenToRelative(e->pos()));

        if (cursor()->selectionSize() && e->modifiers()&Qt::ShiftModifier)
            cursor()->moveSelection(offset);
        else {
            bool moveAnchor = !(e->modifiers()&Qt::ControlModifier);
            cursor()->setPosition(offset, moveAnchor);
            mSelectionTracing = true;
        }

        e->accept();
    } else {
        QWidget::mousePressEvent(e);
    }
}

void HexTextView::mouseReleaseEvent(QMouseEvent *e) {
    if (mSelectionTracing && (e->button() & Qt::LeftButton)) {
        mSelectionTracing = false;
        e->accept();
    } else {
        QWidget::mouseReleaseEvent(e);
    }
}

void HexTextView::mouseMoveEvent(QMouseEvent *e) {
    if (mSelectionTracing) {
        OffType offset = relativeToGlobal(screenToRelative(e->pos()));
        cursor()->setPosition(offset, false);
        e->accept();
    } else {
        QWidget::mouseMoveEvent(e);
    }
}


#undef q
#undef d

//////////////////////////////// HexSettings //////////////////////////////////////
HexSettings *HexSettings::mInstance = 0;


HexSettings::HexSettings(QObject *parent)
    : QObject(parent)
{
}

HexSettings::~HexSettings() {
}

HexSettings *HexSettings::instance() {
    if(!mInstance) mInstance = new HexSettings(qApp);
    return mInstance;
}

QWidget *HexSettings::configPanel() {
    return new HexSettingsPanel;
}

void HexSettings::emitChanged() {
    emit changed();
}



int QLuminancePicker::y2val(int y)
{
    int d = height() - 2*coff - 1;
    return 255 - (y - coff)*255/d;
}

int QLuminancePicker::val2y(int v)
{
    int d = height() - 2*coff - 1;
    return coff + (255-v)*d/255;
}

QLuminancePicker::QLuminancePicker(QWidget* parent)
    :QWidget(parent)
{
    hue = 100; val = 100; sat = 100;
    pix = 0;
    //    setAttribute(WA_NoErase, true);
}

QLuminancePicker::~QLuminancePicker()
{
    delete pix;
}

void QLuminancePicker::mouseMoveEvent(QMouseEvent *m)
{
    setVal(y2val(m->y()));
}
void QLuminancePicker::mousePressEvent(QMouseEvent *m)
{
    setVal(y2val(m->y()));
}

void QLuminancePicker::setVal(int v)
{
    if (val == v)
        return;
    val = qMax(0, qMin(v,255));
    delete pix; pix=0;
    repaint();
    emit newHsv(hue, sat, val);
}

//receives from a hue,sat chooser and relays.
void QLuminancePicker::setCol(int h, int s)
{
    setCol(h, s, val);
    emit newHsv(h, s, val);
}

void QLuminancePicker::paintEvent(QPaintEvent *)
{
    int w = width() - 5;

    QRect r(0, foff, w, height() - 2*foff);
    int wi = r.width() - 2;
    int hi = r.height() - 2;
    if (!pix || pix->height() != hi || pix->width() != wi) {
        delete pix;
        QImage img(wi, hi, QImage::Format_RGB32);
        int y;
        for (y = 0; y < hi; y++) {
            QColor c;
            c.setHsv(hue, sat, y2val(y+coff));
            QRgb r = c.rgb();
            int x;
            for (x = 0; x < wi; x++)
                img.setPixel(x, y, r);
        }
        pix = new QPixmap(QPixmap::fromImage(img));
    }
    QPainter p(this);
    p.drawPixmap(1, coff, *pix);
    const QPalette &g = palette();
    qDrawShadePanel(&p, r, g, true);
    p.setPen(g.foreground().color());
    p.setBrush(g.foreground());
    QPolygon a;
    int y = val2y(val);
    a.setPoints(3, w, y, w+5, y+5, w+5, y-5);
    p.eraseRect(w, 0, 5, height());
    p.drawPolygon(a);
}

void QLuminancePicker::setCol(int h, int s , int v)
{
    val = v;
    hue = h;
    sat = s;
    delete pix; pix=0;
    repaint();
}

QPoint QColorPicker::colPt()
{ return QPoint((360-hue)*(pWidth-1)/360, (255-sat)*(pHeight-1)/255); }
int QColorPicker::huePt(const QPoint &pt)
{ return 360 - pt.x()*360/(pWidth-1); }
int QColorPicker::satPt(const QPoint &pt)
{ return 255 - pt.y()*255/(pHeight-1) ; }
void QColorPicker::setCol(const QPoint &pt)
{ setCol(huePt(pt), satPt(pt)); }

QColorPicker::QColorPicker(QWidget* parent)
    : QFrame(parent)
{
    hue = 0; sat = 0;
    setCol(150, 255);

    QImage img(pWidth, pHeight, QImage::Format_RGB32);
    int x,y;
    for (y = 0; y < pHeight; y++)
        for (x = 0; x < pWidth; x++) {
            QPoint p(x, y);
            QColor c;
            c.setHsv(huePt(p), satPt(p), 200);
            img.setPixel(x, y, c.rgb());
        }
    pix = new QPixmap(QPixmap::fromImage(img));
    setAttribute(Qt::WA_NoSystemBackground);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
}

QColorPicker::~QColorPicker()
{
    delete pix;
}

QSize QColorPicker::sizeHint() const
{
    return QSize(pWidth + 2*frameWidth(), pHeight + 2*frameWidth());
}

static QRect unite(const QRect &a, const QRect &b)
{
    if (a.isNull())
        return b;
    if (b.isNull())
        return a;
    QRect r1 = a.normalized();
    QRect r2 = b.normalized();
    QRect tmp;
    tmp.setX(qMin(r1.x(), r2.x()));
    tmp.setRight(qMax(r1.right(), r2.right()));
    tmp.setY(qMin(r1.y(), r2.y()));
    tmp.setBottom(qMax(r1.bottom(), r2.bottom()));
    return tmp;
}

void QColorPicker::setCol(int h, int s)
{
    int nhue = qMin(qMax(0,h), 359);
    int nsat = qMin(qMax(0,s), 255);
    if (nhue == hue && nsat == sat)
        return;
    QRect r(colPt(), QSize(20,20));
    hue = nhue; sat = nsat;
    r = unite(r, QRect(colPt(), QSize(20,20)));
    r.translate(contentsRect().x()-9, contentsRect().y()-9);
    //    update(r);
    repaint(r);
}

void QColorPicker::mouseMoveEvent(QMouseEvent *m)
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol(p);
    emit newCol(hue, sat);
}

void QColorPicker::mousePressEvent(QMouseEvent *m)
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol(p);
    emit newCol(hue, sat);
}

void QColorPicker::paintEvent(QPaintEvent* )
{
    QPainter p(this);
    drawFrame(&p);
    QRect r = contentsRect();

    p.drawPixmap(r.topLeft(), *pix);
    QPoint pt = colPt() + r.topLeft();
    p.setPen(Qt::black);

    p.fillRect(pt.x()-9, pt.y(), 20, 2, Qt::black);
    p.fillRect(pt.x(), pt.y()-9, 2, 20, Qt::black);

}


ColorPicker::ColorPicker(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mColorPicker = new QColorPicker(this);
    mLuminancePicker = new QLuminancePicker(this);
    connect(mColorPicker, SIGNAL(newCol(int,int)), mLuminancePicker, SLOT(setCol(int,int)));
    connect(mLuminancePicker, SIGNAL(newHsv(int,int,int)), this, SLOT(setHsv(int,int,int)));

    mColorPicker->setFrameStyle(QFrame::Panel + QFrame::Sunken);
    mLuminancePicker->setFixedWidth(20);

    mainLayout->addWidget(mColorPicker);
    mainLayout->addWidget(mLuminancePicker);
    setLayout(mainLayout);
}

void ColorPicker::setHsv(int h, int s, int v) {
    QColor c;
    c.setHsv(h,s,v);
    setColor(c);
}


void ColorPicker::setColor(QColor c) {
    if(mColor != c) {
        mColor = c;
        int h, s ,v;
        c.getHsv(&h, &s, &v);
        mLuminancePicker->setCol(h, s, v);
        mColorPicker->setCol(h, s);
        emit newColor(c);
    }
    update();
}
HexSettingsPanel::HexSettingsPanel(QWidget *parent)
    : QWidget(parent)
{
    readSettings();

    //QPushButton *colorEditButton = new QPushButton(tr("Edit color"), this);
    //connect(colorEditButton, SIGNAL(clicked()), this, SLOT(editColor()));

    mColorPicker = new ColorPicker(this);
    mColors.append(ColorSetup(tr("offset foreground"), &mOffsetFg));
    mColors.append(ColorSetup(tr("offset background"), &mOffsetBg));
    mColors.append(ColorSetup(tr("hex even cell foreground"), &mDataFgEven));
    mColors.append(ColorSetup(tr("hex odd cell foreground"), &mDataFgOdd));
    mColors.append(ColorSetup(tr("hex even line background"), &mDataBgEven));
    mColors.append(ColorSetup(tr("hex odd line background"), &mDataBgOdd));
    mColors.append(ColorSetup(tr("ascii even cell foreground"), &mTextFgEven));
    mColors.append(ColorSetup(tr("ascii odd cell foreground"), &mTextFgOdd));
    mColors.append(ColorSetup(tr("ascii even line background"), &mTextBgEven));
    mColors.append(ColorSetup(tr("ascii odd line background"), &mTextBgOdd));
    mColors.append(ColorSetup(tr("selection foreground"), &mSelFg));
    mColors.append(ColorSetup(tr("selection background"), &mSelBg));
    mColors.append(ColorSetup(tr("cursor foreground"), &mCursorFg));
    mColors.append(ColorSetup(tr("cursor background"), &mCursorBg));

    mColorBox = new QComboBox(this);
    connect(mColorBox, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)));

    foreach(ColorSetup cs, mColors) {
        mColorBox->addItem(iconForColor(*cs.color), cs.text);
    }

    mColorPicker->setColor(mOffsetFg);
    connect(mColorPicker, SIGNAL(newColor(QColor)), this, SLOT(setColor(QColor)));

    QHBoxLayout *colorLay = new QHBoxLayout;
    colorLay->addWidget(mColorBox);
    colorLay->addWidget(mColorPicker);

    QPushButton *fontButton = new QPushButton(tr("change"), this);
    connect(fontButton, SIGNAL(clicked()), this, SLOT(editFont()));
    mFontLabel = new QLabel(tr("Font: ") + mFont.family(), this);

    QHBoxLayout *fontLay = new QHBoxLayout;
    fontLay->addWidget(mFontLabel);
    fontLay->addWidget(fontButton);


    QLabel *colsLabel = new QLabel(tr("Columns: "), this);
    QSpinBox *colsSpin = new QSpinBox(this);
    colsSpin->setValue(mCols);
    connect(colsSpin, SIGNAL(valueChanged(int)), this, SLOT(setCols(int)));
    QHBoxLayout *colsLay = new QHBoxLayout;
    colsLay->addWidget(colsLabel);
    colsLay->addWidget(colsSpin);

    QLabel *grpColsLabel = new QLabel(tr("Columns per group: "), this);
    QSpinBox *grpColsSpin = new QSpinBox(this);
    grpColsSpin->setValue(mGroupCols);
    connect(grpColsSpin, SIGNAL(valueChanged(int)), this, SLOT(setGroupCols(int)));
    QHBoxLayout *grpColsLay = new QHBoxLayout;
    grpColsLay->addWidget(grpColsLabel);
    grpColsLay->addWidget(grpColsSpin);

    //QPushButton *applyButton = new QPushButton(tr("&Apply"), this);
    //connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
    QPushButton *doneButton = new QPushButton(tr("&Done"), this);
    connect(doneButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *buttonLay = new QHBoxLayout;
    buttonLay->addWidget(doneButton);
    //buttonLay->addWidget(applyButton);

    QVBoxLayout *mainLay = new QVBoxLayout;
    mainLay->addLayout(colsLay);
    mainLay->addLayout(grpColsLay);
    mainLay->addLayout(fontLay);
    mainLay->addLayout(colorLay);
    mainLay->addLayout(buttonLay);

    setLayout(mainLay);
}

HexSettingsPanel::~HexSettingsPanel() {
}

void HexSettingsPanel::readSettings() {
    QSettings settings;
    settings.beginGroup("HexWidget");


    LOAD_FONT(mFont, HexSettings::defFont());

    LOAD_INT(mCols, HexSettings::defCols());
    LOAD_INT(mGroupCols, HexSettings::defGroupCols());

    LOAD_COLOR(mOffsetFg,	HexSettings::defOffsetFg());
    LOAD_COLOR(mOffsetBg,	HexSettings::defOffsetBg());
    LOAD_COLOR(mDataFgEven,	HexSettings::defDataFgEven());
    LOAD_COLOR(mDataFgOdd,	HexSettings::defDataFgOdd());
    LOAD_COLOR(mDataBgEven,	HexSettings::defDataBgEven());
    LOAD_COLOR(mDataBgOdd,	HexSettings::defDataBgOdd());
    LOAD_COLOR(mTextFgEven,	HexSettings::defTextFgEven());
    LOAD_COLOR(mTextBgEven,	HexSettings::defTextBgEven());
    LOAD_COLOR(mTextFgOdd,	HexSettings::defTextFgOdd());
    LOAD_COLOR(mTextBgOdd,	HexSettings::defTextBgOdd());
    LOAD_COLOR(mSelFg,		HexSettings::defSelFg());
    LOAD_COLOR(mSelBg,		HexSettings::defSelBg());
    LOAD_COLOR(mCursorFg,	HexSettings::defCursorFg());
    LOAD_COLOR(mCursorBg,	HexSettings::defCursorBg());

    settings.endGroup();
}

void HexSettingsPanel::apply() {
    QSettings settings;
    settings.beginGroup("HexWidget");

    SAVE(mFont);
    SAVE(mCols);
    SAVE(mGroupCols);

    SAVE(mOffsetFg);
    SAVE(mOffsetBg);
    SAVE(mDataFgEven);
    SAVE(mDataFgOdd);
    SAVE(mDataBgEven);
    SAVE(mDataBgOdd);
    SAVE(mTextFgEven);
    SAVE(mTextBgEven);
    SAVE(mTextFgOdd);
    SAVE(mTextBgOdd);
    SAVE(mSelFg);
    SAVE(mSelBg);
    SAVE(mCursorFg);
    SAVE(mCursorBg);
    settings.endGroup();

    HexSettings::instance()->emitChanged();
}

void HexSettingsPanel::setCols(int cols) {
    if(cols != mCols) {
        mCols = cols;
        apply();
    }
}


void HexSettingsPanel::setGroupCols(int groupCols) {
    if(groupCols != mGroupCols) {
        mGroupCols = groupCols;
        apply();
    }
}

void HexSettingsPanel::indexChanged(int index) {
    mColorPicker->setColor(*mColors.at(index).color);
}

void HexSettingsPanel::setColor(QColor newColor) {
    int index = mColorBox->currentIndex();
    if(*mColors.at(index).color != newColor) {
        *mColors.at(index).color = newColor;
        mColorBox->setItemIcon(index, iconForColor(newColor));
        apply();
    }
}


QIcon HexSettingsPanel::iconForColor(const QColor &c) {
    QPixmap pix(QSize(10,10));
    pix.fill(c);
    QPainter p(&pix);
    p.drawRect(QRect(0,0,9,9));
    p.end();
    return QIcon(pix);
}

void HexSettingsPanel::editFont() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, mFont, this);
    if(ok) {
        mFont = font;
        mFontLabel->setText(tr("Font: ")+mFont.family());
        apply();
    }
}

///////////////////////////////////////// Column //////////////////////////////////////////////

Column::Column(QWidget *child, QWidget *parent)
        : QFrame(parent), mChild(child) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setSpacing(0);
    layout->setMargin(frameWidth());
    layout->addWidget(child);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
}


QList<HexWindow*> HexWindow::windowList;

HexWindow::HexWindow() {
    windowList.append(this);
}

HexWindow::~HexWindow() {
    windowList.removeAll(this);
}



PluginDialog::PluginDialog(
    const QString &path,
    const QVector<HexPluginInfo> &plugins,
    QWidget *parent
) :
    QDialog(parent),
    label(new QLabel),
    mPlugins(plugins),
    treeWidget(new QTreeWidget(this)),
    okButton(new QPushButton(tr("OK"), this))
{
    int i;
    treeWidget->setAlternatingRowColors(false);
    treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    treeWidget->setColumnCount(1);
    treeWidget->header()->hide();

    okButton->setDefault(true);

    connect(okButton, SIGNAL(clicked()), this, SLOT(close()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->addWidget(label, 0, 0, 1, 3);
    mainLayout->addWidget(treeWidget, 1, 0, 1, 3);
    mainLayout->addWidget(okButton, 2, 1);
    setLayout(mainLayout);

    pluginIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),
                            QIcon::Normal, QIcon::On);
    pluginIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon),
                            QIcon::Normal, QIcon::Off);
    infoIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));

    setWindowTitle(tr("Plugin Information"));

    label->setText(tr("following plugins were found\n"
                      "(looked in %1):")
                   .arg(path));
                   //.arg(QDir::toNativeSeparators(path)));



    /************ populate our tree... **************/


    //foreach (QObject *plugin, QPluginLoader::staticInstances())
    //    populateTreeWidget(plugin, tr("%1 (Static Plugin)")
    //                               .arg(plugin->metaObject()->className()));

#if 0
    QSignalMapper *signalMapper = new QSignalMapper(this);
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(configure(int)));
#endif

    for(i = 0; i < mPlugins.size(); i++) {
        const HexPluginInfo &plugin = mPlugins.at(i);

        QTreeWidgetItem *pluginItem = new QTreeWidgetItem(treeWidget);
        pluginItem->setText(0, plugin.fileName());
        pluginItem->setIcon(0, pluginIcon);
        treeWidget->setItemExpanded(pluginItem, true);

        QFont boldFont = pluginItem->font(0);
        boldFont.setBold(true);
        pluginItem->setFont(0, boldFont);

        int major = (plugin.version()>>24)&0xff;
        int minor = (plugin.version()>>16)&0xff;
        int patch = plugin.version()&0xffff;

        QString version = QString("%1.%2.%3%4%5%6")
            .arg((major>>4)*10 + (major&0xf))
            .arg((minor>>4)*10 + (minor&0xf))
            .arg((patch>>12)&0xf)
            .arg((patch>>8)&0xf)
            .arg((patch>>4)&0xf)
            .arg(plugin.version()&0xf)
        ;

        addItem(pluginItem, QString("Name: ")+plugin.name());
        addItem(pluginItem, QString("Version: ")+version);
        addItem(pluginItem, QString("Author: ")+plugin.author());
        //addItem(pluginItem, plugin.name() + " v" +version+" by " + plugin.author());

#if 0
        QPushButton *confButton = 0;
        if(plugin.handle()->hasCfgWidget()) {
            QTreeWidgetItem *confItem = addItem(pluginItem, QString());
            confButton = new QPushButton(tr("Configure"), this);
            connect(confButton, SIGNAL(clicked()), signalMapper, SLOT(map()));
            signalMapper->setMapping(confButton, i);
            treeWidget->setItemWidget(confItem, 0, confButton);
        }
        mConfButtons.append(confButton);
#endif
    }
}

QTreeWidgetItem *PluginDialog::addItem(
    QTreeWidgetItem *pluginItem,
    QString info
) {
    if(info.endsWith("..."))
        info.chop(3);
    QTreeWidgetItem *infoItem = new QTreeWidgetItem(pluginItem);
    infoItem->setText(0, info);
    infoItem->setIcon(0, infoIcon);
    return infoItem;
}

#if 0
void PluginDialog::configure(int i) {
    const HexPluginInfo &plugin = mPlugins.at(i);

    QWidget *widget = plugin.handle()->cfgWidget();
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->setWindowTitle(plugin.name());

    mConfButtons.at(i)->setEnabled(false);
    connect(this, SIGNAL(closed()), widget, SLOT(close()));
    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(configureClosed(QObject*)));
    mObjToIndex[widget] = i;
    widget->show();
}

void PluginDialog::closeEvent(QCloseEvent * event) {
    Q_UNUSED(event)
    emit closed();
}

void PluginDialog::configureClosed(QObject *widgetObj) {
    mConfButtons.at(mObjToIndex[widgetObj])->setEnabled(true);
}
#endif

HexTextCursor::HexTextCursor(QTextEdit *te, QObject *parent)
    : QObject(parent), mTextEdit(te)
{
}

TabbedMdi::TabbedMdi(QWidget *parent)
    : QWidget(parent)
{
    mIndex = 0;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mTabBar = new QTabBar(this);
    mMdiArea = new QMdiArea(this);
    mainLayout->addWidget(mTabBar);
    mainLayout->addWidget(mMdiArea);
    setLayout(mainLayout);
    connect(mTabBar,SIGNAL(currentChanged(int)), this, SLOT(handleTabChange(int)));
}

void TabbedMdi::add(QWidget *widget, QString text) {
    QHash<QWidget*,int>::iterator it = mWidgetToIndex.find(widget);

    if(it != mWidgetToIndex.end()) {
        activate(widget);
        return;
    }

    int tabIndex = tabBar()->addTab(text);
    //tabBar()->setCurrentIndex(tabIndex);
    mIndexToWidget[mIndex] = widget;
    mWidgetToIndex[widget] = mIndex;
    tabBar()->setTabData(tabIndex, mIndex);
    mIndex++;
    emit activated(widget);
}

void TabbedMdi::activate(QWidget *widget) {
    for(int i = 0; i < tabBar()->count(); i++) {
        int index = tabBar()->tabData(i).toInt();
        if(widget == mIndexToWidget[index]) {
            if(i != tabBar()->currentIndex())
                tabBar()->setCurrentIndex(i);
            return;
        }
    }
}

void TabbedMdi::remove(QWidget *widget) {
    QHash<QWidget*, int>::iterator it = mWidgetToIndex.find(widget);
    if(it == mWidgetToIndex.end()) return;

    int index = *it;
    mWidgetToIndex.remove(widget);
    mIndexToWidget.remove(index);
    for(int i = 0; i < tabBar()->count(); i++)
        if(index == tabBar()->tabData(i).toInt()) {
            tabBar()->removeTab(i);
            return;
        }
}

void TabbedMdi::handleTabChange(int index) {
    QHash<int, QWidget*>::iterator it = mIndexToWidget.find(tabBar()->tabData(index).toInt());
    if(it != mIndexToWidget.end()) {
        emit activated(*it);
    }
}

MainWindow::MainWindow() {
    mTabbedMdi = new TabbedMdi(this);
    setCentralWidget(mTabbedMdi);
    connect(mdiArea(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(updateMenus()));
    connect(mdiArea(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(activateTab(QMdiSubWindow*)));

    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget *)),
            this, SLOT(setActiveSubWindow(QWidget *)));

    connect(mTabbedMdi, SIGNAL(activated(QWidget*)),
            this, SLOT(tabActivated(QWidget*)));

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    setWindowTitle(tr("QHexEd by Nikita Sadkov"));
    // following requires Qt 4.3
    //setDockOptions(AllowNestedDocks|AllowTabbedDocks|VerticalTabs);

    // assign corners
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
}

MainWindow::~MainWindow() {
}

void MainWindow::closeEvent(QCloseEvent *event) {
    mdiArea()->closeAllSubWindows();
    if (activeMdiChild()) {
        event->ignore();
    } else {
        saveSettings();
        event->accept();
    }
}

void MainWindow::about() {
    QMessageBox::about(this,
        tr("About QHexEd"),
        tr("<b>QHexEd</b> Copyright (C) 2007 Nikita Sadkov<br>"
        "<br>"
        "This is free software.  You may redistribute copies of it under the terms of"
        "the GNU General Public License (<i>http://www.gnu.org/licenses/gpl.html</i>)."
        "There is NO WARRANTY, to the extent permitted by law.")
    );
}

void MainWindow::updateMenus() {
    HexWindow * mdiChild = activeMdiChild();
    emit focusChanged(mdiChild ? mdiChild->dataCursor() : 0);
    emit focusChanged(mdiChild ? mdiChild->textCursor() : 0);

    closeAct->setEnabled(mdiChild != 0);
    closeAllAct->setEnabled(mdiChild != 0);
    tileAct->setEnabled(mdiChild != 0);
    cascadeAct->setEnabled(mdiChild != 0);
    nextAct->setEnabled(mdiChild != 0);
    previousAct->setEnabled(mdiChild != 0);
    separatorAct->setVisible(mdiChild != 0);

    fileMenu->clear();
    fileToolBar->clear();
    editMenu->clear();
    editToolBar->clear();
    toolMenu->clear();
    settingsMenu->clear();
    helpMenu->clear();

    foreach(QAction *action, fileActions) {
        fileMenu->addAction(action);
        fileToolBar->addAction(action);
    }

    foreach(QAction *action, editActions) {
        editMenu->addAction(action);
    }

    foreach(QAction *action, settingsActions) {
        settingsMenu->addAction(action);
    }

    foreach(QAction *action, toolActions) {
        toolMenu->addAction(action);
    }
    //toolMenu->addSeparator();
    //toolMenu->addAction(configAct);

    foreach(QAction *action, helpActions) {
        helpMenu->addAction(action);
    }

    if(mdiChild) {
        QList<QAction *> childEditActions = mdiChild->editActions();
        foreach(QAction *action, childEditActions) {
            editMenu->addAction(action);
            editToolBar->addAction(action);
        }
        fileMenu->addSeparator();
        QList<class QAction *> childFileActions = mdiChild->fileActions();
        foreach(QAction *action, childFileActions) {
            fileMenu->addAction(action);
            if(action->text() != tr("Save &As..."))
                fileToolBar->addAction(action);
        }
    }
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::updateWindowMenu() {
    windowMenu->clear();
    windowMenu->addAction(closeAct);
    windowMenu->addAction(closeAllAct);
    windowMenu->addSeparator();
    windowMenu->addAction(tileAct);
    windowMenu->addAction(cascadeAct);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAct);
    windowMenu->addAction(previousAct);
    windowMenu->addAction(separatorAct);

    QList<QMdiSubWindow *> windows = mdiArea()->subWindowList();
    separatorAct->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i) {
        HexWindow *child = qobject_cast<HexWindow *>(windows.at(i)->widget());

        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1).arg(child->name());
        } else {
            text = tr("%1 %2").arg(i + 1).arg(child->name());
        }
        QAction *action = windowMenu->addAction(text);
        action->setCheckable(true);
        action->setChecked(child == activeMdiChild());
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action, windows.at(i));
    }
}

void MainWindow::createActions() {
/*	saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save changes"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));*/

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    //configAct = new QAction(tr("&Configure"), this);
    //configAct->setShortcut(tr("Ctrl+C"));
    //configAct->setStatusTip(tr("Configure program"));
    //connect(configAct, SIGNAL(triggered()), this, SLOT(configure()));

    closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setShortcut(tr("Ctrl+F4"));
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, SIGNAL(triggered()),
            mdiArea(), SLOT(closeActiveSubWindow()));

    closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, SIGNAL(triggered()),
            mdiArea(), SLOT(closeAllSubWindows()));

    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, SIGNAL(triggered()), mdiArea(), SLOT(tileSubWindows()));

    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, SIGNAL(triggered()), mdiArea(), SLOT(cascadeSubWindows()));

    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, SIGNAL(triggered()),
            mdiArea(), SLOT(activateNextSubWindow()));

    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));
    connect(previousAct, SIGNAL(triggered()),
            mdiArea(), SLOT(activatePreviousSubWindow()));

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    editMenu = menuBar()->addMenu(tr("&Edit"));
    toolMenu = menuBar()->addMenu(tr("&Tools"));
    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    viewMenu = menuBar()->addMenu(tr("&View"));
    windowMenu = menuBar()->addMenu(tr("&Window"));

    updateWindowMenu();
    connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));

}

void MainWindow::createToolBars() {
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName(tr("File Toolbar"));

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName(tr("Edit Toolbar"));
}

void MainWindow::createStatusBar() {
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::loadSettings() {
    QSettings settings;
    settings.beginGroup("MainWindow");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    QByteArray state = settings.value("state", saveState()).toByteArray();
    settings.endGroup();
    move(pos);
    resize(size);
    restoreState(state);

    updateMenus();
}

void MainWindow::saveSettings() {
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("state", saveState());
    settings.endGroup();
}

HexWindow *MainWindow::activeMdiChild() {
    if (QMdiSubWindow *activeSubWindow = mdiArea()->activeSubWindow())
        return qobject_cast<HexWindow *>(activeSubWindow->widget());
    return 0;
}

void MainWindow::activateTab(QMdiSubWindow *sw) {
    mTabbedMdi->activate(sw->widget());
}

void MainWindow::tabActivated(QWidget *window) {
    QList<QMdiSubWindow *> list = mdiArea()->subWindowList();
    foreach(QMdiSubWindow *sw, list) {
        if(sw->widget() == window) {
            mdiArea()->setActiveSubWindow(sw);
            return;
        }
    }
}

void MainWindow::setActiveSubWindow(QWidget *window) {
    if(!window) return;
    mdiArea()->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::subWindowClosed(QObject *window) {
    if(!activeMdiChild()) { // last window?
        emit focusChanged((HexCursor*)0);
        emit focusChanged((HexTextCursor*)0);
    }

    mTabbedMdi->remove(qobject_cast<QWidget*>(window));
}

void MainWindow::addWindow(HexWindow *window) {
    window->setAttribute(Qt::WA_DeleteOnClose);

    if(!window->docked()) {
        mdiArea()->addSubWindow(window);
        mTabbedMdi->add(window, window->windowTitle());
        connect(window, SIGNAL(destroyed(QObject*)), this, SLOT(subWindowClosed(QObject*)));
        window->show();
    } else {
        QDockWidget *dock = new QDockWidget(window->windowTitle(), this);
        dock->setObjectName(window->windowTitle() + " Dock");
        dock->setWidget(window);
        addDockWidget(window->dockArea(), dock);
        viewMenu->addAction(dock->toggleViewAction());
    }
}

void MainWindow::addAction(HexActionType type, QAction *act) {
    switch(type) {
    case HexFileAction:
        fileActions.append(act);
        break;
    case HexEditAction:
        editActions.append(act);
        break;
    case HexToolAction:
        toolActions.append(act);
        break;
    case HexSettingsAction:
        settingsActions.append(act);
        break;
    case HexHelpAction:
        helpActions.append(act);
        break;
    }
}

void MainWindow::updateStatus(QString status) {
    statusBar()->showMessage(status);
}



HexEdImpl::HexEdImpl(int argc, char **argv) {
    pluginsDialog = 0;
    mainWindow = new MainWindow();
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(mainWindow, SIGNAL(focusChanged(HexCursor *)),
            this, SIGNAL(focusChanged(HexCursor *)));
    connect(mainWindow, SIGNAL(focusChanged(HexTextCursor *)),
            this, SIGNAL(focusChanged(HexTextCursor *)));
    connect(mainWindow, SIGNAL(destroyed(QObject *)), qApp, SLOT(quit()));
}

HexEdImpl::~HexEdImpl() {
}

int HexEdImpl::exec() {
    loadPlugins();
    mainWindow->loadSettings();
    mainWindow->show();
    return qApp->exec();
}

#define REGISTER_PLUGIN(ClassName)  do { \
    HexPlugin *plugin = (HexPlugin*)(new ClassName); \
    HexPluginInfo pluginInfo = plugin->info(); \
    pluginInfo.setFileName(#ClassName); \
    pluginInfo.setHandle(plugin); \
    loadedPlugins.append(pluginInfo); \
} while (0)

void HexEdImpl::loadPlugins() {
    REGISTER_PLUGIN(BasicFileAccess);
    REGISTER_PLUGIN(BasicInspector);
    REGISTER_PLUGIN(ConfigPlugin);

    foreach(HexPluginInfo plugin, loadedPlugins) {
        if(!plugin.handle()->init(this)) {
            qDebug() << "failed to init: " << plugin.fileName();
        }
    }

    pluginsAct = new QAction(tr("About &Plugins"), this);
    pluginsAct->setStatusTip(tr("Loaded plugins"));
    connect(pluginsAct, SIGNAL(triggered()), this, SLOT(showPluginsDialog()));
    mainWindow->addAction(HexHelpAction, pluginsAct);
}



void HexEdImpl::showPluginsDialog() {
    pluginsAct->setDisabled(true);
    pluginsDialog = new PluginDialog(pluginsDir.path(), loadedPlugins, mainWindow);
    pluginsDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(pluginsDialog, SIGNAL(destroyed(QObject*)), this, SLOT(pluginsDialogDone()));
    connect(mainWindow, SIGNAL(destroyed(QObject *)), pluginsDialog, SLOT(close()));
    pluginsDialog->show();
}
void HexEdImpl::pluginsDialogDone() {
    pluginsAct->setEnabled(true);
    pluginsDialog = 0;
}

void HexEdImpl::addWindow(HexWindow *window) {
    mainWindow->addWindow(window);
}

void HexEdImpl::addAction(HexActionType type, QAction *act) {
    mainWindow->addAction(type, act);
}

void HexEdImpl::updateStatus(QString status) {
    mainWindow->updateStatus(status);
}

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(resources);

    QApplication *app = new QApplication(argc, argv);
    app->setOrganizationName("Nikita Sadkov");
    app->setApplicationName("qhexed");
    app->setWindowIcon(QIcon(":images/deadbeef.png"));

#ifdef Q_OS_WIN
    // cuz under wine strange things happen - Qt choices comic sans!
    QFont font("arial");
    font.setPointSize(8);
    app->setFont(font);
#endif

    return HexEdImpl(argc, argv).exec();
}

