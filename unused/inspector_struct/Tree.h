#ifndef TREE_H
#define TREE_H

#include <HexPlugin.h>
#include "HexNet.h"

class Tree;
class Root;
class Expr;
class Struct;
class StructEntry;

class Lexer;

class TreeNode;
typedef Graph<TreeNode> Graph;

class TreeNode : public Graph::Node {
public:
	enum {
		ROOT=USER_NODE,
		TYPE,
		DATA,
		EXPR,
		STRUCT,
		STRUCT_FIELD,
	};

	enum {
		SIG_DEL=USER_SIGNAL,
	};

	TreeNode(Graph *g)
		: Node(g), mDelFlag(false)
	{
	}

	TreeNode(TreeNode *n)
		: Node(n->graph()), mDelFlag(false)
	{
		n->conn(SIG_DEL, this);
	}

	~TreeNode() {
		mDelFlag = true;
		bcast(SIG_DEL);
	}

	virtual void reciv(Node *src, int sig, void *data) {
		if(sig == SIG_DEL && !mDelFlag) delete this;
	}

	virtual int nodeType() = 0;

private:
	bool mDelFlag;
};

class Root : public TreeNode {
public:
	Root(Graph *g) : TreeNode(g) {}
	Root(TreeNode *n) : TreeNode(n) {}
	int classType() {return ROOT;}
};

class Data;

class Type : public TreeNode {
public:
	Type(Graph *g) : TreeNode(g) {}
	Type(TreeNode *n) : TreeNode(n) {}
	int classType() {return TYPE;}
	virtual QString typeName() = 0;
	virtual Data *read(HexDocument *doc, OffType offset) = 0;
};

class Data : public TreeNode {
public:
	Data(Graph *g) : TreeNode(g) {}
	Data(TreeNode *n) : TreeNode(n) {}
	int classType() {return DATA;}

	Type *type() {return mType;}
	void setType(Type *type) {mType = type;}

	QString name() {return mName;}
	void setName(QString name) {mName = name;}

	OffType offset() {return mOffset;}
	void setOffset(OffType offset) {mOffset = offset;}

	QString value() {return mOffset;}
	void setValue(QString value) {mValue = value;}

private:
	Type *mType;
	OffType mOffset;
	QString mName;
	QString mValue;
};

class U1 : public Type {
public:
	U1(Graph *g) : Type(g) {}
	U1(TreeNode *n) : Type(n) {}
	int classType() {return TYPE;}
	virtual QString typeName() {return "u1";}
};

#if 0
class StructField : public Node {
public:
	StructEntry() {
		setClassType(STRUCT_ENTRY);
		offset = 0;
	}

	StructEntry *toStructEntry() {
		return this;
	}

	//int readInt(HexCursor *cur);

	OffType offset;
	QString type;
	QString name;
	int sizeExpr; // size expression
	int index;
	QString value;
};

class Struct : public Node {
public:
	Struct() {
		setClassType(STRUCT);
		offset = 0;
	}

	Struct *toStruct() {
		return this;
	}

	OffType offset;
	QString name;
};

class Expr : public Node {
public:
	// NOTE: all int operations should be redone to long long

	enum {
		VAR_REF=128,
		INTEGER,
		FLOAT,
		NEG,
		SHR,
		SHL,
		LAND,
		LOR,
		EQ,
		NE,
		GT,
		LT,
		GTE,
		LTE,
		AADD,
		ASUB,
		AMUL,
		ADIV,
		AREM,
		AAND,
		AOR,
		PTRMEMBER,
		PTRDEREF,
		ADROF,
		INC,
		DEC,
		SIZEOF
	};

	Expr(Lexer *l, int type, QVariant value);
	Expr(Lexer *l, int type, int lhs, int rhs);
	Expr(const Expr &e);

	Expr *toExpr();

	Expr &operator=(const Expr &e);

	HexCursor *cursor() const;

	const QVariant &value() const;

	void setValue(QVariant v);

	int type() const;
	void setType(int type);

	Expr *l() const;
	void setL(int e);

	Expr *r() const;
	void setR(int e);

	QVariant calc();

private:
	QVariant adrof(Expr *e);

	QVariant neg(const QVariant &v);

	QVariant lnot(const QVariant &v);
	QVariant bnot(const QVariant &v);

	QVariant add(const QVariant &v0, const QVariant &v1);
	QVariant sub(const QVariant &v0, const QVariant &v1);
	QVariant mul(const QVariant &v0, const QVariant &v1);
	QVariant band(const QVariant &v0, const QVariant &v1);
	QVariant bor(const QVariant &v0, const QVariant &v1);
	QVariant bxor(const QVariant &v0, const QVariant &v1);
	QVariant shl(const QVariant &v0, const QVariant &v1);
	QVariant shr(const QVariant &v0, const QVariant &v1);
	QVariant div(const QVariant &v0, const QVariant &v1);
	QVariant mod(const QVariant &v0, const QVariant &v1);

	int mType;
	int mLhs;
	int mRhs;
	Lexer *mLexer;
	QVariant mValue;
};

#endif

#endif

