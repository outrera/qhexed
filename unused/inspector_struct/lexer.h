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

#ifndef LEXER_H
#define LEXER_H

#ifndef __FLEX_LEXER_H
#undef yyFlexLexer
#include <FlexLexer.h>
#endif

#include <stdexcept>
#include <QHash>
#include <QString>
#include <QStack>
#include <HexPlugin.h>

#include "HexGraph.h"
#include "parser.yy.h"

using namespace std;

namespace NAMESPACE {

class ParseError {
public:
	int bline;
	int bcol;
	int eline;
	int ecol;
	QString msg;
};


class Lexer : public yyFlexLexer {
	int yylex();

public:

	Lexer(std::istream* src = 0);

	virtual ~Lexer();

	// parser call nest operator to get tokens
	Parser::token_type operator()(Parser::semantic_type* lval, Parser::location_type* lloc = 0);

	virtual void LexerError(const char* msg);

	void clear();

	int allocString(const QString &str);
	QString getString(int index);
	void freeString(int index);

	int resolve(QString id);
	Node *node(int index);

	HexCursor *cursor() {
		return mCursor;
	}

	void setCursor(HexCursor *c) {
		mCursor = c;
	}

	QVector<Node*> &nodes() {
		return treeNodes;
	}

	void setNodes(QVector<Node*> n) {
		clear();
		treeNodes = n;
	}

	void pcrel(int p, int c);

	int newExpr(int type, QVariant value=QVariant());
	int newExpr(int type, int lhs, int rhs);

	void addStructEntry(QString type, QString id, int expr);
	void beginStruct(QString name);
	void endStruct();

	bool parse(const QString &source);
	QString toString();

	int errorLine() {
		return errLine;
	}

	QString errorMessage() {
		return errMsg;
	}

private:
	HexCursor *mCursor;
	void traverse(Node *n);

	int tmpStrIdx;
	QHash<int,QString> tmpStrings;
	QStack<int> nsStack;
	QVector<Node*> treeNodes;

	QString errMsg;
	int errLine;

	// following two used by traverse()
	int level;
	QString saveString;

	Parser::semantic_type* yylval;
};

}

typedef NAMESPACE::Lexer MCParser;

#endif // LEXER_H

