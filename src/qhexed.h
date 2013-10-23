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


#ifndef HEXED_H
#define HEXED_H

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <QtGui>
#include <QFile>




#ifndef __cplusplus
typedef enum {false, true} bool;
#endif //__cplusplus

typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

typedef int64_t OffType;

#define auto(var,expr) typeof(expr) var = (expr)
#define MIN(a,b) ({auto(_a,(a)); auto(_b,(b)); _a < _b ? _a : _b})
#define MAX(a,b) ({auto(_a,(a)); auto(_b,(b)); _a > _b ? _a : _b})

#define SWAP16(v) ((((v)&0xff00)>>8)|(((v)&0xff)<<8))
#define SWAP32(v) ((((v)&0xff000000ul)>>24) | (((v)&0xff0000ul)>>8) \
    |(((v)&0xff00ul)<<8)|(((v)&0xfful)<<24))

#define LSB16(v) (*(uint16_t*)(v))
#define LSB32(v) (*(uint32_t*)(v))
#define LSB64(v) (*(OffType*)(v))

#define Deb fprintf(stderr, "debug\n");



class HexDocument;

class HexCursor : public QObject {
    Q_OBJECT

public:
    HexCursor(HexDocument *doc, QObject *parent=0);
    HexCursor(HexDocument *doc, OffType anchor, OffType position, QObject *parent=0);
    ~HexCursor();


    HexDocument *document() const {
        return mDocument;
    }

    void setDocument(HexDocument *doc);

    OffType anchor() const {
        return mAnchor;
    }

    OffType position() const {
        return mPosition;
    }

    OffType top() const {
        return mTop;
    }

    void setCursor(OffType newAnchor, OffType newCursor);
    void setPosition(OffType position, bool moveAnchor=true);
    void setTop(OffType top);

    void moveSelection(OffType position);
    void toStart(bool moveAnchor=true);
    void toEnd(bool moveAnchor=true);
    void move(int delta, bool moveAnchor=true);

    bool atStart();
    bool atEnd();
    bool hasSelection() {return anchor() != position();}

    OffType selectionStart() {return qMin(anchor(), position());}
    OffType selectionEnd() {return qMax(anchor(), position());}
    OffType selectionSize() {return selectionEnd()-selectionStart();}
    QByteArray selectedData();

public slots:
    void del();
    void cut();
    void copy();
    void copyAsText();
    void paste();
    void pasteOver();
    void selectAll();
    void clearSelection();
    void undo();
    void redo();

signals:
    void changed();
    void topChanged();

private slots:
    void documentChanged();

private:
    HexDocument *mDocument;
    OffType mTop;		// top-left HexWidget-corner offset
    OffType mAnchor;
    OffType mPosition;
    int mRefs;
};

class HexTextCursor : public QObject {
    Q_OBJECT

public:
    HexTextCursor(QTextEdit *te, QObject *parent = 0);

    QTextEdit *textEdit() {
        return mTextEdit;
    }

    QString selectedText() {
        return mTextEdit->textCursor().selectedText();
    }

private:
    QTextEdit *mTextEdit;
};


class HexCursor;
class HexDocument;
class HexWidget;
class QTextCursor;

class HexWindow : public QFrame {
    Q_OBJECT

public:
    HexWindow();
    ~HexWindow();

    virtual QList<class QAction *> fileActions() {
        return QList<class QAction *>();
    }

    virtual QList<class QAction *> editActions() {
        return QList<class QAction *>();
    }

    virtual bool docked() {
        return false;
    }

    virtual Qt::DockWidgetArea dockArea() {
        return Qt::RightDockWidgetArea;
    }

    virtual QString name() {
        return "unnamed";
    }

    virtual HexCursor *dataCursor() {
        return 0;
    }


    virtual HexTextCursor *textCursor() {
        return 0;
    }


private:
    static QList<HexWindow*> windowList;
};


enum HexActionType {
	HexFileAction,
	HexEditAction,
	HexToolAction,
	HexSettingsAction,
	HexHelpAction,
};

class HexEd : public QObject {
	Q_OBJECT

public:
	virtual void addWindow(HexWindow *window) = 0;
	virtual void addAction(HexActionType type, class QAction *act) = 0;
	virtual void updateStatus(QString status) = 0;

signals:
	// signaled when user changes focus to window with different HexCursor
	// if cur == 0 then no HexCursor in focus 
	void focusChanged(HexCursor *cur);
	void focusChanged(HexTextCursor *cur);
};



class HexDataModel : public QObject {
    Q_OBJECT

public:
    HexDataModel(QObject *parent = 0);
    virtual ~HexDataModel();

    virtual void write(OffType dst, const void *src, OffType size) = 0;
    virtual void read(void *dst, OffType src, OffType size) = 0;
    virtual OffType getLength() = 0;
    virtual bool isGrowable() = 0;
    virtual bool isWriteable() = 0;

    virtual void del(OffType start, OffType end) {
    }

    virtual QByteArray cut(OffType start, OffType end) {
        QByteArray ret = copy(start, end);
        del(start, end);
        return ret;
    }

    QByteArray copy(OffType start, OffType end) {
        OffType size = end-start;
        QByteArray ret(size, 0);
        read(ret.data(), start, size);
        return ret;
    }

    virtual void paste(OffType where, const QByteArray &what) {
    }

    void pasteOver(OffType where, const QByteArray &what) {
        write(where, what.data(), what.size());
    }

    virtual OffType getPageSize() {
        return 1024;
    }

    virtual bool isCuttable() {
        return false;
    }

};


class HexCache {
public:
    class Reference {
    public:
        Reference(HexCache &inHexCache, int inOffset)
           : cache(inHexCache), offset(inOffset) {
        }

        Reference(Reference const &r)
            : cache(r.cache), offset(r.offset) {
        }

        inline operator int() {
            return cache.getByte(offset);
        }

        inline void operator=(int val) {
            cache.putByte(offset, (uint8_t)val);
        }

        inline void operator=(Reference &val) {
            cache.putByte(offset, (int)val);
        }


    private:
        HexCache &cache;
        int offset;
    };

    struct Page {
        Page *prev, *next;
        OffType offset; // line offset and hash key
        OffType modified;	// number of bytes modified or -1 if page isn't modified
        // note: alternatively we can implement range by using startOfModified, but this will
        //       complicate code, without any perfomance increasion in general case
        uint8_t *data;
    };

    HexCache(HexDataModel &inDsm, int cacheSize=100*1024, int inPageSize=-1);
    ~HexCache();

    inline Reference operator[](OffType offset) {
        return Reference(*this, offset);
    }

    void prefetch(OffType offset) {
        fetchDeep(offset/pageSize);
    }

    void refetch() {
        for(int i = 0; i < numPages; i++)
            if(pages[i].offset != (OffType)-1) {
                dsm.read(pages[i].data, pages[i].offset, pageSize);
                pages[i].modified = -1;
            }
    }

    // call clear() when unerlaying data model makes current cache invalid
    void clear();
    void flush();

    inline uint8_t getByte(OffType offset) {
        return fetch(offset)->data[offset%pageSize];
    }

    inline void putByte(OffType offset, uint8_t val) {
        Page *page = fetch(offset);
        offset %= pageSize;

        page->data[offset] = val;
        if(page->modified < (int)offset)
            page->modified = offset;
    }

    OffType getLength();

    // perform simple tests to catch common implemetation errors
    static bool selfTest();

private:
    Page *fetch(OffType offset) {
        OffType pageOffset = offset / pageSize;
        Page *page = mostRecentlyUsed;
        if(pageOffset == page->offset) return page;
        else if(pageOffset == (page = page->prev)->offset) return page;
        //else if(pageOffset == (page = page->prev)->offset) return page;
        //else if(pageOffset == (page = page->prev)->offset) return page;
        else return fetchDeep(pageOffset);
    }

    Page *fetchDeep(OffType pageOffset);

    void flushPage(Page *page) {
        if(page->modified >= 0) {
            dsm.write(page->offset*pageSize, page->data, page->modified+1);
            page->modified = -1;
        }
    }

    HexDataModel &dsm;
    Page *mostRecentlyUsed;
    Page *leastRecentlyUsed;
    Page *pages;
    int pageSize;
    int numPages;

    void *hashMap;
};




class QAction;
class HexUndoCommand;

class HexDocument : public QObject {
    Q_OBJECT

private:

    void putByte(OffType offset, uint8_t byte);

public:

    class Reference {
    public:
        Reference(HexDocument &doc, int offset)
           : mDoc(doc), mOffset(offset) {
        }

        Reference(Reference const &r)
            : mDoc(r.mDoc), mOffset(r.mOffset) {
        }

        inline operator int() {
            return mDoc.mCache->getByte(mOffset);
        }

        inline void operator=(int val) {
            mDoc.putByte(mOffset, (uint8_t)val);
        }

        inline void operator=(Reference &val) {
            mDoc.putByte(mOffset, (int)val);
        }


    private:
        HexDocument &mDoc;
        int mOffset;
    };
    friend class Reference;

    HexDocument(QObject *parent=0);
    HexDocument(HexDataModel *model, QObject *parent=0);
    ~HexDocument();

    bool maybeSave();
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);

    void pushCommand(HexUndoCommand *cmd);
    QByteArray copy(OffType start, OffType end);
    QByteArray copyAsText(OffType start, OffType end);

    QAction *createRedoAction();
    QAction *createUndoAction();

    HexCursor *cursor() {
        return mCursor;
    }

    HexCache *cache() {
        return mCache;
    }

    OffType length() {
        return mCache->getLength();
    }

    int refs() {
        return mRefs;
    }

    void reference() {
        ++mRefs;
    }

    void release() {
        if(mRefs <= 1) deleteLater();
        else --mRefs;
    }

    int numRefs() {
         return mRefs;
    }

    bool isModified() {
        return mModified;
    }

    bool isEditable() {
        return !mReadOnly;
    }

    bool isCuttable() {
        return mModel->isCuttable();
    }

    void setModified(bool state);
    void setPath(QString name);
    void setName(QString name);
    QString name();
    QString path();
    QString userFriendlyName();

    Reference operator[](OffType offset) {
        return Reference(*this, offset);
    }

    void pasteByte(uint8_t byte) {
        return paste(cursor()->position(), QByteArray((char*)&byte, 1));
    }

    void pasteArray(const QByteArray &ba) {
        return paste(cursor()->position(), ba);
    }

public slots:
    void del();
    void cut();
    void copy();
    void copyAsText();
    void paste();
    void pasteOver();
    void undo();
    void redo();

signals:
    // signaled when there is no need to save changes anymore
    void nameChanged();
    void changed();
    void saved();

private:
    friend class HexUndoCommand;

    // following function are for ours and our friend's convenience
    void del(OffType start, OffType end);
    QByteArray cut(OffType start, OffType end);
    void paste(OffType where, const QByteArray &what);
    void pasteOver(OffType where, const QByteArray &what);
    uint8_t replaceByte(OffType where, uint8_t what);

    void initFrom(HexDataModel *model);

    int mRefs;
    QString mName;
    QString mPath;
    bool mReadOnly;
    bool mModified;			// true if content is modified and unsaved
    bool mBuffer;
    bool mFreeModel;
    HexCache *mCache;
    HexDataModel *mModel;
    HexCursor *mCursor;

    class QUndoStack *mUndoStack;
};



class QAction;

class HexWidgetPrivate;
class HexDocument;
class HexCursor;

class HexWidget : public QWidget {
    Q_OBJECT

public:

    HexWidget(HexDocument *document=0, QWidget *parent=0);
    ~HexWidget();

    HexDocument *document();
    bool isEditable();
    void setEditable(bool state);
    HexCursor *cursor();
    static QWidget *configPanel();


    QList<class QAction *> fileActions();
    QList<class QAction *> editActions();

public slots:
    void cut();
    void copy();
    void copyAsText();
    void paste();
    void pasteOver();
    bool save();
    bool saveAs();

signals:
    void offsetChanged(OffType offset);

private:
    Q_DISABLE_COPY(HexWidget)

    void *mPrivate;
};


class HexDataWindow : public HexWindow {
    Q_OBJECT

public:
    HexDataWindow(HexDocument *document);
    QList<class QAction *> fileActions();
    QList<class QAction *> editActions();
    QString name();

    HexWidget *hexWidget() {
        return mHexWidget;
    }

    HexCursor *dataCursor();

private slots:
    void updateTitle();

private:
    HexWidget *mHexWidget;
};


class HexTextWindow : public HexWindow {
    Q_OBJECT

public:
    HexTextWindow(QString text = QString());

    bool maybeSave();
    bool saveFile(const QString &fileName);
    bool loadFile(const QString &fileName);

    QList<class QAction *> fileActions();
    QList<class QAction *> editActions();

    QString name();
    void setName(const QString &name);

    QString path();
    void setPath(const QString &path);

    QString text();
    void setText(const QString &text);

    QTextDocument *document() {
        return textEdit()->document();
    }

    QTextEdit *textEdit() {
        return mTextEdit;
    }

    HexTextCursor *textCursor() {
        return mTextCursor;
    }

    void closeEvent(QCloseEvent *event);

private slots:
    void updateTitle();
    void documentWasModified();

    bool save();
    bool saveAs();

private:

    QString mName;
    QString mPath;

    QTextEdit *mTextEdit;
    HexTextCursor *mTextCursor;

    QList<class QAction *> mFileActions;
    QList<class QAction *> mEditActions;

    class QAction
        *saveAct,
        *saveAsAct,
        *cutAct,
        *copyAct,
        *copyAsTextAct,
        *pasteAct,
        *pasteOverAct,
        *undoAct,
        *redoAct,
    ;
};


class HexFile : public HexDataModel {
    Q_OBJECT

public:
    HexFile(QFile *file, QObject *parent = 0)
        : HexDataModel(parent), mFile(file)
    {
        forbidExpansion = false;
        length = mFile->size();

        if(!mFile->parent())
            mFile->setParent(this);
    }

    virtual ~HexFile() {
    }

    void write(OffType dst, const void *src, OffType size) {

        mFile->seek(dst);
        int written = mFile->write((char*)src, size);
        if(written > 0) {
            size = written;
            if(dst+size > length)
                length = dst+size;
        }
    }

    void read(void *dst, OffType src, OffType size) {
        if(src+size > length) {
            if(src > length) return;
            memset(dst, length-src, size-(length-src));
            size = length-src;
        }

        mFile->seek(src);
        int readed = (int)mFile->read((char*)dst, size);
        if(readed < 0)
            memset(dst, 0, size);
        else if(readed != (int)size)
            memset((uint8_t*)dst+readed, 0, size-readed);
    }

    void setForbidExpansion(bool val) {
        forbidExpansion = val;
    }

    OffType getLength() {
        return length;
    }

    bool isGrowable() {
        return mFile->isWritable();
    }

    bool isWriteable() {
        return mFile->isWritable();
    }

private:
    OffType length;
    QFile *mFile;
    bool forbidExpansion;
};



class HexBuffer : public HexDataModel {
    Q_OBJECT

public:
    HexBuffer(QObject *parent = 0);
    HexBuffer(const QByteArray &ba, QObject *parent = 0);

    void write(OffType dst, const void *src, OffType size);
    void read(void *dst, OffType src, OffType size);
    OffType getPageSize();

    void del(OffType start, OffType end);
    void paste(OffType where, const QByteArray &what);


    OffType getLength();

    bool isGrowable();
    bool isWriteable();
    bool isCuttable();


private:

    QLinkedList<QByteArray> mBuffer;
    OffType mSize;		// total size
};


class HexStaticBuffer : public HexDataModel {
    Q_OBJECT

public:
    HexStaticBuffer(uint8_t *buf = 0, OffType size = 0, OffType offset = 0, QObject *parent = 0)
        : HexDataModel(parent), buffer(buf), bufferSize(size), startOffset(offset)
    {
    }

    void write(OffType dst, const void *src, OffType size) {
        dst -= startOffset;

        if(dst+size > bufferSize) size = bufferSize-dst;

        uint8_t *p = (uint8_t*)src;
        OffType end = dst+size;
        while(dst < end) buffer[dst++] = *p++;
    }

    void read(void *dst, OffType src, OffType size) {
        memset(dst, 0, size);
        src -= startOffset;

        if(src+size > bufferSize) size = bufferSize-src;

        uint8_t *p = (uint8_t*)dst;
        OffType end = src+size;
        while(src < end) *p++ = buffer[src++];
    }

    void setBuffer(uint8_t *ibuffer, OffType ibufferSize, OffType istartOffset) {
        buffer = ibuffer;
        bufferSize = ibufferSize;
        startOffset = istartOffset;
    }

    OffType getLength() {
        return bufferSize;
    }
    virtual bool isGrowable() {
        return false;
    }
    virtual bool isWriteable() {
        return true;
    }

    // page size cache system should use for reading

private:
    uint8_t *buffer;
    OffType bufferSize;
    OffType startOffset;
};



#define HEX_VERSION 0x00010000

class QWidget;
class QString;
class QAction;

class HexEd;
class HexDocument;
class HexPlugin;

class HexPluginInfo {
public:

    HexPluginInfo() {}

    HexPluginInfo(
        const QString &name,
        int version,
        const QString author
    ) :
        mName(name),
        mVersion(version),
        mAuthor(author),
        mHandle(0)
    {
    }


    HexPluginInfo(const HexPluginInfo &pluginInfo) {
        *this = pluginInfo;
    }

    HexPluginInfo &operator=(const HexPluginInfo &pluginInfo) {
        mName = pluginInfo.name();
        mIcon = pluginInfo.icon();
        mVersion = pluginInfo.version();
        mAuthor = pluginInfo.author();
        mFileName = pluginInfo.fileName();
        mHandle = pluginInfo.handle();
        return *this;
    }

    QString name() const {return mName;}
    int version() const {return mVersion;}
    QIcon icon() const {return mIcon;}
    QString author() const {return mAuthor;}

    // for internal use
    QString fileName() const {return mFileName;}
    void setFileName(QString fileName) {mFileName = fileName;}
    HexPlugin *handle() const {return mHandle;}
    void setHandle(HexPlugin *handle) {mHandle = handle;}

private:
    QString mName;
    QIcon mIcon;
    int mVersion;
    QString mAuthor;
    QString mFileName;
    HexPlugin *mHandle;
};

class HexPlugin : public QObject {
    Q_OBJECT

public:
    virtual ~HexPlugin() {}

    virtual bool init(HexEd *) = 0;

    uint hexVersion() {
        return HEX_VERSION;
    }

    virtual HexPluginInfo info() {
        return HexPluginInfo("unnamed", 0x00000000, "anonymous");
    }
};

class QLabel;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class PluginDialog : public QDialog {
    Q_OBJECT

public:
    PluginDialog(
        const QString &path,
        const QVector<HexPluginInfo> &loadedPluginsInfo,
        QWidget *parent = 0
    );


signals:
    //void closed();

private slots:
    //void configure(int index);
    //void configureClosed(QObject *);

protected:
    //void closeEvent(QCloseEvent * event);

private:
    QTreeWidgetItem *addItem(QTreeWidgetItem *pluginItem, QString info);

    QLabel *label;
    QIcon pluginIcon;
    QIcon infoIcon;
    QVector<HexPluginInfo> mPlugins;
    QTreeWidget *treeWidget;
    QPushButton *okButton;
#if 0
    QVector<QPushButton*> mConfButtons;
    QMap<QObject*,int> mObjToIndex;
#endif
};



class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QCheckBox;
class QLineEdit;
class QLabel;
class QComboBox;
class QFontComboBox;

class HexView;
class HexDataModel;
class HexCursor;


class HexCellDesc {
public:

    enum {
        CENTER		= 0x000000,
        LEFT		= 0x000100,
        RIGHT		= 0x000200,
        FRAME		= 0x001000,
        INVALID		= 0x002000,
    };

    HexCellDesc()
        : mParam(INVALID)
    {
    }

    HexCellDesc(const HexCellDesc &cd)
        : mParam(cd.mParam), mFG(cd.mFG), mBG(cd.mBG)
    {
    }

    HexCellDesc(uint param, const QColor &fg, const QColor &bg)
        : mParam(param), mFG(fg), mBG(bg)
    {
    }

    HexCellDesc &operator=(const HexCellDesc &cd) {
        mParam = cd.mParam;
        mFG = cd.mFG;
        mBG = cd.mBG;
        return *this;
    }

    bool operator==(const HexCellDesc &cd) const {
        return mParam == cd.mParam && mFG == cd.mFG && mBG == cd.mBG;
    }

    uint hash() const {
        uint h = mParam + mFG.rgba() + mBG.rgba();
        return h;
    }

    QColor fg() const {
        return mFG;
    }

    QColor bg() const {
        return mBG;
    }

    void setFg(const QColor &c) {
        mFG = c;
    }

    void setBg(const QColor &c) {
        mBG = c;
    }
    void setFgBg(const QColor &fg, const QColor &bg) {
        mFG = fg;
        mBG = bg;
    }


    int value() const {
        return mParam&0xff;
    }

    void setValue(int v) {
        mParam = (mParam&~(0xff|INVALID)) | v;
    }

    int alignment() const {
        return mParam&0x300;
    }

    void setAlignment(int a) {
        mParam = (mParam&~0x300) | a;
    }

    void enableFrame(bool state) {
        if(state) mParam |= FRAME;
        else mParam &= ~FRAME;
    }

    bool frameEnabled() const {
        return (mParam&FRAME) != 0;
    }

    void makeInvalid() {
        mParam = INVALID;
    }

    bool isValid() const {
        return (mParam&INVALID) == 0;
    }


private:
    int mParam;
    QColor mFG;
    QColor mBG;
};

uint qHash(const HexCellDesc &cd);

/////////////////////// HexWidgetPrivate /////////////////////////////

class HexWidgetPrivate : public QFrame {
    Q_OBJECT

public:

    HexWidgetPrivate(HexWidget *pub, HexDocument *document);
    ~HexWidgetPrivate();

    bool isEditable();
    void setEditable(bool state);

    HexDocument *document();

    void setCursor(HexCursor *cursor);

    HexCursor *cursor() {
        return mCursor;
    }

    static QWidget *configPanel();


    QList<class QAction *> fileActions() {
        return mFileActions;
    }

    QList<class QAction *> editActions() {
        return mEditActions;
    }

    bool event(QEvent * e);

    int charHeight() {return mCellSide;}
    int charWidth() {return mCellSide/2;}
    QPixmap fetchCell(const HexCellDesc &cd);
    int cols() {return mCols;}
    int groupCols() {return mGroupCols;}

    QColor offsetFg()	{return mOffsetFg;}
    QColor offsetBg()	{return mOffsetBg;}
    QColor dataFgEven()	{return mDataFgEven;}
    QColor dataBgEven()	{return mDataBgEven;}
    QColor dataFgOdd()	{return mDataFgOdd;}
    QColor dataBgOdd()	{return mDataBgOdd;}
    QColor textFgEven()	{return mTextFgEven;}
    QColor textBgEven()	{return mTextBgEven;}
    QColor textFgOdd()	{return mTextFgOdd;}
    QColor textBgOdd()	{return mTextBgOdd;}
    QColor selFg()		{return mSelFg;}
    QColor selBg()		{return mSelBg;}
    QColor cursorFg()	{return mCursorFg;}
    QColor cursorBg()	{return mCursorBg;}


public slots:
    void cut();
    void copy();
    void copyAsText();
    void paste();
    void pasteOver();

    bool save();
    bool saveAs();

signals:
    void cellFontChanged();
    void offsetChanged(OffType offset);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void documentChanged();
    void documentSaved();
    void updateTitle();
    void updateActions();
    void readSettings();

private:
    HexWidget *mPublic;

    struct CharPix {
        CharPix *prev, *next;
        HexCellDesc cd; // hash key
        QPixmap pix;
    };


    void clearFontCache();
    QPixmap drawFontCell(const HexCellDesc &cd);

    bool mEditable;

    HexCursor *mCursor;
    HexDocument *mDocument;

    QList<QAction *> mFileActions;
    QList<QAction *> mEditActions;

    QList<HexWidgetPrivate *> mInstanceList;

    class QAction
        *saveAct,
        *saveAsAct,
        *cutAct,
        *copyAct,
        *copyAsTextAct,
        *pasteAct,
        *pasteOverAct,
        *undoAct,
        *redoAct,
    ;


    HexView *mOffsetView;
    HexView *mDataView;
    HexView *mTextView;

    QFont mFont;		// font to use for drawing all stuff
    int mCellMargin;	// border margin in byte cell
    int mCellSide;		// size in pixels of byte cell side
    int mCols;			// number of bytes per line
    int mGroupCols;		// number of bytes in group

    QColor
        mOffsetFg,
        mOffsetBg,
        mDataFgEven,
        mDataFgOdd,
        mDataBgEven,
        mDataBgOdd,
        mTextFgEven,
        mTextBgEven,
        mTextFgOdd,
        mTextBgOdd,
        mSelFg,
        mSelBg,
        mCursorFg,
        mCursorBg,
    ;

    CharPix *mru;
    CharPix *lru;
    CharPix *pixes;
    int mCacheSize; // number of font cells to cache

    QHash<HexCellDesc, CharPix*> mPixHash;
};


//////////////////////////////// HexView Related Stuff //////////////////////////////////////
class HexView : public QWidget {
    Q_OBJECT

public:
    HexView(HexWidgetPrivate *parent, Qt::WindowFlags f=0);
    virtual ~HexView();

    QSize sizeHint() const;

    HexWidgetPrivate *parent() const {return mParent;}
    HexDocument *document() const;
    HexCursor *cursor() const;

    bool inView(OffType offset) {return start() <= offset && offset < end();}

    OffType start() const {return cursor()->top();}
    OffType end() const {return start()+cols()*rows();}
    OffType length() const {return document()->length();}
    int rows() const {return ((height()+parent()->charHeight()-1)/parent()->charHeight());}
    int cols() const {return parent()->cols();}
    int groupCols() {return parent()->groupCols();}
    int charWidth() const {return parent()->charWidth();}


public slots:
    void scroll(int delta);

protected slots:
    void updateSize(); // update widget size

private slots:
    void updateView();
    void cursorChanged();

protected:
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *e);
    void keyPressEvent(QKeyEvent * event);
    void contextMenuEvent(QContextMenuEvent *event);

    virtual void input(char ch) = 0;
    virtual int widgetWidth() const = 0;

    void setCursorBlinking(bool state);
    void moveCursor(int col, int row, bool moveAnchor=true);

    QRect rangeToRect(OffType start, OffType end);
    QPair<OffType,OffType> rectToRange(QRect rect);

private:
    bool mSelectionVisible;
    bool mCursorVisible;
    bool mEditable;
    HexWidgetPrivate *mParent;
};



class HexOffsetView : public HexView {
    Q_OBJECT

public:
    HexOffsetView(HexWidgetPrivate *parent, Qt::WindowFlags f=0);

    void paintEvent(QPaintEvent * event);

    int widgetWidth() const {
        return charWidth()*offsetSize() + (offsetSize()-1)/4*charWidth();
    }

    void input(char ch);

    // converts screen coordinates to offset relative to start()
    OffType screenToRelative(QPoint point);

    // converts start() relative offset to global offset
    OffType relativeToGlobal(int rel);

public slots:
    //void setStart(OffType offset);

private slots:
    void documentChanged();

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);


private:
    int offsetSize() const;
    bool mSelInverted;
    bool mSelectionTracing;
};


class HexDataView : public HexView {
    Q_OBJECT

public:
    HexDataView(HexWidgetPrivate *parent, Qt::WindowFlags=0);

    void paintEvent(QPaintEvent * event);

    int widgetWidth() const {
        return 2*charWidth()*(cols() + (cols()-1)/parent()->groupCols());
    }

    void input(char ch);

    // converts screen coordinates to offset relative to start()
    OffType screenToRelative(QPoint point);

    // converts start() relative offset to global offset
    OffType relativeToGlobal(int rel);

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);


private slots:
    void cursorFlash();
    void updateCursor();

private:
    void setCursorFlash(bool state);

    HexDocument *mDocument;
    int mCursorSubPos;
    bool mSelectionTracing;
    class QTimer *mCursorFlashTimer;
    bool mCursorFlash;
};

class HexTextView : public HexView {
    Q_OBJECT

public:

    HexTextView(HexWidgetPrivate *parent, Qt::WindowFlags f=0);

    void paintEvent(QPaintEvent * event);

    int widgetWidth() const {
        return charWidth()*cols();
    }

    void input(char ch);

    // converts screen coordinates to offset relative to start()
    OffType screenToRelative(QPoint point);

    // converts start() relative offset to global offset
    OffType relativeToGlobal(int rel);

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

private slots:
    void cursorFlash();
    void updateCursor();

private:
    void setCursorFlash(bool state);

    bool mSelectionTracing;
    class QTimer *mCursorFlashTimer;
    bool mCursorFlash;
};


//////////////////////////////// HexSettings //////////////////////////////////////

class HexSettings : public QObject {
    Q_OBJECT

public:
    ~HexSettings();

    static HexSettings *instance();
    QWidget *configPanel();

#define PREDEF_COLOR(color_name) QApplication::palette().color(QPalette::color_name)
    static QColor defOffsetFg()		{return PREDEF_COLOR(WindowText);}
    static QColor defOffsetBg()		{return PREDEF_COLOR(Window);}
    static QColor defDataFgEven()	{return PREDEF_COLOR(Text);}
    static QColor defDataFgOdd()	{return PREDEF_COLOR(Link);}
    static QColor defDataBgEven()	{return PREDEF_COLOR(Base);}
    static QColor defDataBgOdd()	{return PREDEF_COLOR(AlternateBase);}
    static QColor defTextFgEven()	{return PREDEF_COLOR(Text);}
    static QColor defTextBgEven()	{return PREDEF_COLOR(Base);}
    static QColor defTextFgOdd()	{return PREDEF_COLOR(Text);}
    static QColor defTextBgOdd()	{return PREDEF_COLOR(AlternateBase);}
    static QColor defSelFg()		{return PREDEF_COLOR(HighlightedText);}
    static QColor defSelBg()		{return PREDEF_COLOR(Highlight);}
    static QColor defCursorFg()		{return PREDEF_COLOR(WindowText);}
    static QColor defCursorBg()		{return QColor(Qt::red);}
    static QFont defFont() {
#ifdef Q_OS_WIN
        QFont font("courier");
        font.setWeight(QFont::Bold);
        font.setPointSize(8);
#else
        QFont font("monospace");
        font.setPointSize(8);
#endif
        return font;
    }

    static int defCols() {return 16;}
    static int defGroupCols() {return 4;}


signals:
    void changed();

private:
    HexSettings(QObject *parent);
    void emitChanged();
    void readSettings();

    friend class HexSettingsPanel;
    static HexSettings *mInstance;
};


///////////////////////////////// Color Picker //////////////////////////////////////

class QColorPicker : public QFrame
{
    Q_OBJECT
public:
    QColorPicker(QWidget* parent);
    ~QColorPicker();

public slots:
    void setCol(int h, int s);

signals:
    void newCol(int h, int s);

protected:
    QSize sizeHint() const;
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    //enum { pWidth = 220, pHeight = 200 };
    enum { pWidth = 150, pHeight = 100 };

    int hue;
    int sat;

    QPoint colPt();
    int huePt(const QPoint &pt);
    int satPt(const QPoint &pt);
    void setCol(const QPoint &pt);

    QPixmap *pix;
};

class QLuminancePicker : public QWidget
{
    Q_OBJECT
public:
    QLuminancePicker(QWidget* parent=0);
    ~QLuminancePicker();

public slots:
    void setCol(int h, int s, int v);
    void setCol(int h, int s);

signals:
    void newHsv(int h, int s, int v);

protected:
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    enum { foff = 3, coff = 4 }; //frame and contents offset
    int val;
    int hue;
    int sat;

    int y2val(int y);
    int val2y(int val);
    void setVal(int v);

    QPixmap *pix;
};

class ColorPicker : public QWidget {
    Q_OBJECT
public:
    ColorPicker(QWidget *parent);

public slots:
    void setColor(QColor c);

signals:
    void newColor(QColor c);

private slots:
    void setHsv(int h, int s, int v);

private:
    QColor mColor;
    QColorPicker *mColorPicker;
    QLuminancePicker *mLuminancePicker;
};


class HexSettingsPanel : public QWidget {
    Q_OBJECT

public:
    HexSettingsPanel(QWidget *parent=0);
    ~HexSettingsPanel();

private slots:
    void apply();
    void indexChanged(int index);
    void setColor(QColor c);
    void editFont();
    void setCols(int cols);
    void setGroupCols(int groupCols);

private:
    QIcon iconForColor(const QColor &c);
    void readSettings();

    struct ColorSetup {
        ColorSetup() {}
        ColorSetup(const ColorSetup &cs) : text(cs.text), color(cs.color) {}
        ColorSetup(QString t, QColor *c) : text(t), color(c) {}
        ColorSetup &operator=(const ColorSetup &cs) {text = cs.text;color = cs.color; return *this;}
        QString text;
        QColor *color;
    };

    QColor
        mOffsetFg,
        mOffsetBg,
        mDataFgEven,
        mDataFgOdd,
        mDataBgEven,
        mDataBgOdd,
        mTextFgEven,
        mTextBgEven,
        mTextFgOdd,
        mTextBgOdd,
        mSelFg,
        mSelBg,
        mCursorFg,
        mCursorBg,
    ;

    QFont mFont;
    int mCols;
    int mGroupCols;


    ColorPicker *mColorPicker;
    QLabel *mFontLabel;
    QComboBox *mColorBox;
    QVBoxLayout *mLabelLayout;
    QVBoxLayout *mValueLayout;
    QVector<ColorSetup> mColors;
    HexSettings *mSettings;
};

///////////////////////////////////////// Column //////////////////////////////////////////////

class Column : public QFrame {
    Q_OBJECT

public:
    Column(QWidget *child, QWidget *parent=0);

private:
    QWidget *mChild;
};



class QAction;
class QMenu;
class QSignalMapper;
class QTabBar;
class QMdiArea;
class QMdiSubWindow;

class HexCursor;
class HexWindow;

class TabbedMdi : public QWidget {
    Q_OBJECT

public:
    TabbedMdi(QWidget *parent);

    QTabBar *tabBar() {
        return mTabBar;
    }

    QMdiArea *mdiArea() {
        return mMdiArea;
    }

    void add(QWidget *widget, QString text);
    void remove(QWidget *widget);

public slots:
    void activate(QWidget *widget);

signals:
    void activated(QWidget *widget);

private slots:
    void handleTabChange(int index);

private:

    int mIndex;
    QHash<QWidget*, int> mWidgetToIndex;
    QHash<int, QWidget*> mIndexToWidget;
    QTabBar *mTabBar;
    QMdiArea *mMdiArea;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void addWindow(class HexWindow *window);
    void addAction(HexActionType type, QAction *);
    void updateStatus(QString status);

    void loadSettings();
    void saveSettings();

signals:
    void focusChanged(HexCursor *cur);
    void focusChanged(HexTextCursor *cur);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void about();
    void updateMenus();
    void updateWindowMenu();
    void setActiveSubWindow(QWidget *window);
    void subWindowClosed(QObject *window);
    void activateTab(QMdiSubWindow *sw);
    void tabActivated(QWidget *window);

private:
    struct ConfigPanel {
        QWidget *widget;
        QString name;
        QIcon icon;
    };

    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    QTabBar *tabBar() {
        return mTabbedMdi->tabBar();
    }

    QMdiArea *mdiArea() {
        return mTabbedMdi->mdiArea();
    }

    HexWindow *activeMdiChild();

    TabbedMdi *mTabbedMdi;
    QSignalMapper *windowMapper;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *toolMenu;
    QMenu *settingsMenu;
    QMenu *viewMenu;
    QMenu *windowMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;

    QList<QAction*> fileActions;
    QList<QAction*> editActions;
    QList<QAction*> toolActions;
    QList<QAction*> settingsActions;
    QList<QAction*> helpActions;

    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *closeAct;
    QAction *closeAllAct;
    QAction *tileAct;
    QAction *cascadeAct;
    QAction *nextAct;
    QAction *previousAct;
    QAction *separatorAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *configAct;
};




class QAction;
class PluginDialog;

struct PluginInfo {
    QString path;
    HexPlugin *handle;
};

class HexEdImpl : public HexEd {
    Q_OBJECT

public:
    HexEdImpl(int argc, char **argv);
    ~HexEdImpl();

    int exec();

    void addWindow(HexWindow *window);
    void addAction(HexActionType type, QAction *);
    void updateStatus(QString status);

private slots:
    void showPluginsDialog();
    void pluginsDialogDone();

private:
    void loadPlugins();

    class MainWindow *mainWindow;

    QList<QAction> textActions;
    QList<QAction> toolActions;

    QDir pluginsDir;
    QVector<HexPluginInfo> loadedPlugins;
    QAction *pluginsAct;
    PluginDialog *pluginsDialog;
};

class HexUndoCommand : public QUndoCommand {
public:
    HexUndoCommand(const QString &text)
    {
        setText(text);
    }

    HexDocument *document() {
        return mDocument;
    }

    void setDocument(HexDocument *doc) {
        mDocument = doc;
    }

    void del(OffType start, OffType end) {
        document()->del(start, end);
    }

    QByteArray cut(OffType start, OffType end) {
        return document()->cut(start, end);
    }

    QByteArray copy(OffType start, OffType end) {
        return document()->copy(start, end);
    }

    QByteArray copyAsText(OffType start, OffType end) {
        return document()->copyAsText(start, end);
    }

    void paste(OffType where, const QByteArray &what) {
        document()->paste(where, what);
    }

    void pasteOver(OffType where, const QByteArray &what) {
        document()->pasteOver(where, what);
    }

    uint8_t replaceByte(OffType where, uint8_t with) {
        return document()->replaceByte(where, with);
    }

    void saveCursor() {
        mAnchor = document()->cursor()->anchor();
        mPosition = document()->cursor()->position();
    }

    void restoreCursor() {
        document()->cursor()->setCursor(mAnchor, mPosition);
    }

private:
    HexDocument *mDocument;
    OffType mAnchor;
    OffType mPosition;
};

class HexCut : public HexUndoCommand {
public:
    HexCut(OffType start, OffType end)
            : HexUndoCommand("cut selection"),
            mStart(start), mEnd(end)
    {
    }

    virtual void undo() {
        saveCursor();
        paste(mStart, mData);
        mData = QByteArray();
    }

    virtual void redo() {
        mData = cut(mStart, mEnd);
        restoreCursor();
    }

private:
    OffType mStart;
    OffType mEnd;
    QByteArray mData;
};

class HexDel : public HexUndoCommand {
public:
    HexDel(OffType start, OffType end)
            : HexUndoCommand("delete selection"),
            mStart(start), mEnd(end)
    {
    }

    virtual void redo() {
        saveCursor();
        mData = copy(mStart, mEnd);
        del(mStart, mEnd);
    }

    virtual void undo() {
        paste(mStart, mData);
        mData = QByteArray();
        restoreCursor();
    }

private:
    OffType mStart;
    OffType mEnd;
    QByteArray mData;
};

class HexPasteOver : public HexUndoCommand {
public:
    HexPasteOver(OffType where, QByteArray what)
            : HexUndoCommand("paste over"),
            mStart(where), mEnd(where+what.size()), mWhat(what)
    {
    }

    virtual void redo() {
        saveCursor();
        QByteArray save = copy(mStart, mEnd);
        pasteOver(mStart, mWhat);
        mWhat = save;
    }

    virtual void undo() {
        QByteArray save = copy(mStart, mEnd);
        pasteOver(mStart, mWhat);
        mWhat = save;
        restoreCursor();
    }

private:
    OffType mStart;
    OffType mEnd;
    QByteArray mWhat;
};

class HexPaste : public HexUndoCommand {
public:
    HexPaste(OffType where, QByteArray what)
            : HexUndoCommand("paste"),
            mStart(where), mEnd(where+what.size()), mWhat(what)
    {
    }

    virtual void redo() {
        saveCursor();
        paste(mStart, mWhat);
        mWhat = QByteArray();
    }

    virtual void undo() {
        mWhat = copy(mStart, mEnd);
        del(mStart, mEnd);
        restoreCursor();
    }

private:
    OffType mStart;
    OffType mEnd;
    QByteArray mWhat;
};

class HexReplaceByte : public HexUndoCommand {
public:
    HexReplaceByte(OffType where, uint8_t with)
            : HexUndoCommand("replace byte"),
            mWhere(where), mWith(with)
    {
    }

    virtual void redo() {
        saveCursor();
        mWith = replaceByte(mWhere, mWith);
    }

    virtual void undo() {
        mWith = replaceByte(mWhere, mWith);
        restoreCursor();
    }

private:
    OffType mWhere;
    uint8_t mWith;
};



class BasicFileAccess : public HexPlugin {
    Q_OBJECT

public:
    bool init(HexEd *);
    HexPluginInfo info();

private slots:
    void newBuffer();
    void newTextBuffer();
    void openFile();
    void loadFile();
    void loadTextFile();

private:
    HexEd *mEd;

    QAction *newBufferAct;
    QAction *newTextBufferAct;
    QAction *openFileAct;
    QAction *loadFileAct;
    QAction *loadTextFileAct;
};



class QWidget;
class InspectorFunction;
class HexDocument;


class InspectorModel : public QAbstractTableModel {
    Q_OBJECT

public:
    InspectorModel(QList<InspectorFunction*> &mInspectors, QObject *parent = 0);

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

    void setCursor(HexCursor *cur);

public slots:
    void updateModel();

private slots:
    void documentDestroyed(QObject *obj);

private:
    QList<InspectorFunction*> &mInspectors;
    HexCursor *mCursor;
};



class InspectorFunction : public QObject {
    Q_OBJECT
public:
    virtual QString name() = 0;
    virtual QString value(class HexDocument *doc, OffType offset) = 0;

    virtual QWidget *configWidget() {
        return 0;
    }

signals:
    void updateRequested();
};


class ScalarInspector : public InspectorFunction {
    Q_OBJECT
public:
    enum ByteOrder {
        Lsb, Msb
    };

    enum DisplayType {
        DispS1, DispU1, DispS2, DispU2, DispS4, DispU4,
        DispF4, DispF8, DispOctal, DispBinary
    };

    ScalarInspector(const QString &name, DisplayType type, ByteOrder order=Lsb, bool hex=0)
        : mHex(hex), mName(name), mType(type), mOrder(order)
    {
    }

    QString value(HexDocument *doc, OffType offset);

    QString name() {
        return mName;
    }

    void setName(const QString &name) {
        mName = name;
    }

    void setType(DisplayType type) {
        mType = type;
    }

    void setOrder(ByteOrder order) {
        mOrder = order;
    }

    bool hex() {
        return mHex;
    }

    void setHex(bool hex) {
        mHex = hex;
    }


    virtual QWidget *configWidget() {
        return 0;
    }

signals:
    //void updateRequested();

private:

    QString bitsToString(uint8_t data) {
        char bitBuf[32];
        for ( int i = 0; i < 8; i++ )
            bitBuf[7-i] = (data&(1<<i)) ? '1' : '0';
        bitBuf[8] = 0;
        return bitBuf;
    }

    bool mHex;
    QString mName;
    DisplayType mType;
    ByteOrder mOrder;
};



class QVBoxLayout;
class QCheckBox;
class QLineEdit;
class QLabel;


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

class InspectorWidget : public QFrame {
    Q_OBJECT

public:
    InspectorWidget(QWidget *parent=0, Qt::WindowFlags f=0);
    ~InspectorWidget();


    HexCursor *cursor() {
        return mCursor;
    }
    HexDocument *document() {
        return cursor()->document();
    }


public slots:
    void focusChanged(HexCursor *cur);

signals:
    void offsetChanged(OffType offset);

private slots:
    void newOffset();
    void wordFormatChanged(bool state);
    void unsignedFormatChanged(bool state);

    void setOffset(OffType offset);

private:

    bool mLsb;
    bool mHex;

    class ScalarInspector
        *mS1,
        *mU1,
        *mS2,
        *mU2,
        *mS4,
        *mU4,
        *mF4,
        *mF8,
        *mOctal,
        *mBinary,
    ;

    HexCursor *mCursor;

    QVBoxLayout *mLayout;
    LabeledCheckBox *mCheckLsb;
    LabeledCheckBox *mCheckHex;
    LabeledValue *mOffsetValue;
    class InspectorModel *mModel;
    class QTableView *mTableView;
    QList<class InspectorFunction*> mInspectors;
};


class Panel : public HexWindow {
    Q_OBJECT

public:
    Panel();
    QString name();

    bool docked();
    Qt::DockWidgetArea dockArea();

private slots:
    void focusChanged(HexCursor *cur);

private:
    InspectorWidget *mInspectorWidget;
};

class BasicInspector : public HexPlugin {
    Q_OBJECT

public:
    bool init(HexEd *);
    HexPluginInfo info();

private:
    HexEd *mEd;
};

class ConfigPlugin : public HexPlugin {
    Q_OBJECT

public:
    bool init(HexEd *);
    HexPluginInfo info();

private slots:
    void popupCfgDialog();
    void cfgDialogClosed();

private:
    HexEd *mEd;
    QAction *mCfgAct;
};

class QListWidget;
class QStackedLayout;

class ConfigDialog : public HexWindow {
    Q_OBJECT

public:
    ConfigDialog(QWidget *parent=0);
    ~ConfigDialog();
    void addPanel(QWidget *panel, QString text, QIcon icon = QIcon());
    void closeEvent(QCloseEvent *event);

signals:
    void closed(bool);

private:
    QListWidget *mOptionSelector;
    QStackedLayout *mOptionPanel;
};



#endif
