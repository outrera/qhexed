#include <QtGui>
#include "HexGraph.h"
//#include "lexer.h"

namespace NAMESPACE {

#if 0
Expr::Expr(Lexer *l, int type, QVariant value) {
	mLexer = l;
	setClassType(EXPR);
	setType(type);
	setValue(value);
}

Expr::Expr(Lexer *l, int type, int lhs, int rhs) {
	mLexer = l;
	setClassType(EXPR);
	setType(type);
	mLhs = lhs;
	mRhs = rhs;
}

Expr::Expr(const Expr &e) {
	*this = e;
}

Expr *Expr::toExpr() {
	return this;
}

Expr &Expr::operator=(const Expr &e) {
	setType(e.type());
	mLhs = e.mLhs;
	mRhs = e.mRhs;
	mLexer = e.mLexer;
	setValue(e.value());
	return *this;
}

HexCursor *Expr::cursor() const {
	return mLexer->cursor();
}

const QVariant &Expr::value() const {
	return mValue;
}

void Expr::setValue(QVariant v) {
	mValue = v;
}

int Expr::type() const {
	return mType;
}

void Expr::setType(int type) {
	mType = type;
}

Expr *Expr::l() const {
	return mLexer->node(mLhs)->toExpr();
}

Expr *Expr::r() const {
	return mLexer->node(mRhs)->toExpr();
}

void Expr::setL(int e) {
	mLhs = e;
}

void Expr::setR(int e) {
	mRhs = e;
}

QVariant Expr::calc() {
	switch(type()) {
	case INTEGER: return value().toString().toInt();
	case FLOAT: return value().toString().toDouble();
	case VAR_REF: {
		int index = value().toInt();
		if(!index) return 0;

		StructEntry *se = mLexer->node(index)->toStructEntry();

		u1 buf[8];
		HexDocument *doc = cursor()->document();

		for(int i = 0; i < 8; i++)
			buf[i] = (*doc)[se->offset+i];

		if(se->type == "char") return (int)*(char*)buf;
		else if(se->type == "s1") return (int)*(s1*)buf;
		else if(se->type == "u1") return (int)*(u1*)buf;
		else if(se->type == "s2") return (int)*(s2*)buf;
		else if(se->type == "u2") return (int)*(u2*)buf;
		else if(se->type == "s4") return (int)*(s4*)buf;
		else if(se->type == "u4") return (int)*(u4*)buf;
		else if(se->type == "s8") return (int)*(s8*)buf;
		else if(se->type == "u8") return (int)*(u8*)buf;
		else if(se->type == "float") return (double)*(float*)buf;
		else if(se->type == "dobule") return *(double*)buf;
		else return 0;
	}
	case NEG: return neg(l()->calc());
	case '!': return lnot(l()->calc());
	case '~': return bnot(l()->calc());
	case '+': return add(l()->calc(), r()->calc());
	case '-': return sub(l()->calc(), r()->calc());
	case '*': return mul(l()->calc(), r()->calc());
	case '/': return div(l()->calc(), r()->calc());
	case '%': return mod(l()->calc(), r()->calc());
	case '&': return band(l()->calc(), r()->calc());
	case '|': return bor(l()->calc(), r()->calc());
	case '^': return bxor(l()->calc(), r()->calc());
	case SHL: return shl(l()->calc(), r()->calc());
	case SHR: return shr(l()->calc(), r()->calc());
	case ADROF: return adrof(l());

	default:
		return QVariant();
	}
}

QVariant Expr::adrof(Expr *e) {
	if(e->type() != VAR_REF) return 0;
	int index = value().toInt();
	if(index != 0) return 0;

	StructEntry *se = mLexer->node(index)->toStructEntry();
	return (int)se->offset;
}

QVariant Expr::neg(const QVariant &v) {
	switch(v.type()) {
	case QVariant::Int: return -v.toInt();
	case QVariant::Double: return -v.toDouble();
	default: return QVariant(0);
	}
}

QVariant Expr::lnot(const QVariant &v) {
	switch(v.type()) {
	case QVariant::Int: return !v.toInt();
	case QVariant::Double: return !(int)v.toDouble();
	default: return QVariant(0);
	}
}

QVariant Expr::bnot(const QVariant &v) {
	switch(v.type()) {
	case QVariant::Int: return ~v.toInt();
	case QVariant::Double: return ~(int)v.toDouble();
	default: return QVariant(0);
	}
}

#define VARTYPE(a,b) ((QVariant::a<<8) | QVariant::b)
QVariant Expr::add(const QVariant &v0, const QVariant &v1) {
	switch((v0.type()<<8)|v1.type()) {
	case VARTYPE(Int,Int): return v0.toInt() + v1.toInt();
	case VARTYPE(Double,Int): return v0.toDouble() + v1.toInt();
	case VARTYPE(Int,Double): return v0.toInt() + v1.toDouble();
	}
	return QVariant(0);
}

QVariant Expr::sub(const QVariant &v0, const QVariant &v1) {
	switch((v0.type()<<8)|v1.type()) {
	case VARTYPE(Int,Int): return v0.toInt() - v1.toInt();
	case VARTYPE(Double,Int): return v0.toDouble() - v1.toInt();
	case VARTYPE(Int,Double): return v0.toInt() - v1.toDouble();
	}
	return QVariant(0);
}

QVariant Expr::mul(const QVariant &v0, const QVariant &v1) {
	switch((v0.type()<<8)|v1.type()) {
	case VARTYPE(Int,Int): return v0.toInt() * v1.toInt();
	case VARTYPE(Double,Int): return v0.toDouble() * v1.toInt();
	case VARTYPE(Int,Double): return v0.toInt() * v1.toDouble();
	}
	return QVariant(0);
}

QVariant Expr::band(const QVariant &v0, const QVariant &v1) {
	switch((v0.type()<<8)|v1.type()) {
	case VARTYPE(Int,Int): return v0.toInt() & v1.toInt();
	case VARTYPE(Double,Int): return (int)v0.toDouble() & v1.toInt();
	case VARTYPE(Int,Double): return v0.toInt() & (int)v1.toDouble();
	}
	return QVariant(0);
}

QVariant Expr::bor(const QVariant &v0, const QVariant &v1) {
	switch((v0.type()<<8)|v1.type()) {
	case VARTYPE(Int,Int): return v0.toInt() | v1.toInt();
	case VARTYPE(Double,Int): return (int)v0.toDouble() | v1.toInt();
	case VARTYPE(Int,Double): return v0.toInt() | (int)v1.toDouble();
	}
	return QVariant(0);
}

QVariant Expr::bxor(const QVariant &v0, const QVariant &v1) {
	switch((v0.type()<<8)|v1.type()) {
	case VARTYPE(Int,Int): return v0.toInt() ^ v1.toInt();
	case VARTYPE(Double,Int): return (int)v0.toDouble() ^ v1.toInt();
	case VARTYPE(Int,Double): return v0.toInt() ^ (int)v1.toDouble();
	}
	return QVariant(0);
}

QVariant Expr::shl(const QVariant &v0, const QVariant &v1) {
	switch((v0.type()<<8)|v1.type()) {
	case VARTYPE(Int,Int): return v0.toInt() << v1.toInt();
	case VARTYPE(Double,Int): return (int)v0.toDouble() << v1.toInt();
	case VARTYPE(Int,Double): return v0.toInt() << (int)v1.toDouble();
	}
	return QVariant(0);
}

QVariant Expr::shr(const QVariant &v0, const QVariant &v1) {
	switch((v0.type()<<8)|v1.type()) {
	case VARTYPE(Int,Int): return v0.toInt() >> v1.toInt();
	case VARTYPE(Double,Int): return (int)v0.toDouble() >> v1.toInt();
	case VARTYPE(Int,Double): return v0.toInt() >> (int)v1.toDouble();
	}
	return QVariant(0);
}

QVariant Expr::div(const QVariant &v0, const QVariant &v1) {
	switch((v0.type()<<8)|v1.type()) {
	case VARTYPE(Int,Int): return v0.toInt() / (v1.toInt() ? v1.toInt() : 1);
	case VARTYPE(Double,Int): return v0.toDouble() / (v1.toInt() ? v1.toInt() : 1);
	case VARTYPE(Int,Double): return v0.toInt() / (v1.toDouble() > 0.000001 ? v1.toDouble() : 1.0);
	}
	return QVariant(0);
}

QVariant Expr::mod(const QVariant &v0, const QVariant &v1) {
	switch((v0.type()<<8)|v1.type()) {
	case VARTYPE(Int,Int): return v0.toInt() % (v1.toInt() ? v1.toInt() : 1);
	case VARTYPE(Double,Int): return (int)v0.toDouble() % (v1.toInt() ? v1.toInt() : 1);
	case VARTYPE(Int,Double): return v0.toInt() % ((int)v1.toDouble() ? (int)v1.toDouble() : 1);
	}
	return QVariant(0);
}
#endif

}

