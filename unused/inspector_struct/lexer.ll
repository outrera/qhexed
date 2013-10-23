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

%option 8bit
%option noyywrap
%option c++
%option yyclass="NAMESPACE::Lexer"
%option pointer

/* tell parser to maintain lineno() variable */
%option yylineno

%{
#include <math.h>
#include <string.h>

#include <QtGui>

#include "lexer.h"
#include "parser.yy.h"

using namespace NAMESPACE;

%}

DIGIT    [0-9]
LETTER   [a-zA-Z]
ID       {LETTER}("_"|{LETTER}|{DIGIT})*

%%

"char"|"u1"|"u2"|"u4"|"u8"|"s1"|"s2"|"s4"|"s8" {
	yylval->i = allocString(QString(yytext));
	return Parser::token::TYPE;
}

"struct" {return Parser::token::STRUCT;}
"sizeof" {return Parser::token::SIZEOF;}

"//".*"\n" {/*ignore comments*/ }
"/*"(.|"\n")+"*/" {/*ignore comments*/ }

{DIGIT}+ {
	/* An integer */
	yylval->i = newExpr(Expr::INTEGER, QString(yytext));
	return Parser::token::INTEGER;
}

{DIGIT}+"."{DIGIT}* {
	/* A float */
	yylval->i = newExpr(Expr::FLOAT, QString(yytext));
	return Parser::token::FLOAT;
}

{ID} {
	yylval->i = allocString(QString(yytext));
	return Parser::token::ID;
}

"="|"+"|"-"|"*"|"/"|"^"|"("|")"|"{"|"}"|"["|"]"|"<"|">"|";"|":"|"?"|"!"|"~"|"." {
	return yytext[0];
}

"->" {return Parser::token::PTRMEMBER;}
"++" {return Parser::token::INC;}
"--" {return Parser::token::DEC;}
"<<" {return Parser::token::SHL;}
">>" {return Parser::token::SHR;}
"==" {return Parser::token::EQ;}
"!=" {return Parser::token::NE;}
">=" {return Parser::token::GTE;}
"<=" {return Parser::token::LTE;}
"&&" {return Parser::token::LAND;}
"||" {return Parser::token::LOR;}
"+=" {return Parser::token::AADD;}
"-=" {return Parser::token::ASUB;}
"*=" {return Parser::token::AMUL;}
"/=" {return Parser::token::ADIV;}
"%=" {return Parser::token::AREM;}
"&=" {return Parser::token::AAND;}
"|=" {return Parser::token::AOR;}

[\n] {/*return '\n';*/}
[ \x0d\t]+ {/* eat up whitespace */}

. {
	ParseError pe;
	char tmp[256];
	sprintf(tmp, "Unrecognized character: %s (0x%x)\n", yytext, yytext[0]);
	pe.msg = tmp;
	pe.bline = lineno();
	throw pe;
}




%%

#include <iostream>
#include <sstream>
#include <fstream>

namespace NAMESPACE {

Lexer::Lexer(std::istream* src)
	: yyFlexLexer(src, 0)
{
	mCursor = 0;
	clear();
}

Lexer::~Lexer() {
	clear();
}

void Lexer::clear() {
	qDeleteAll(treeNodes);
	treeNodes.clear();
	tmpStrings.clear();
	tmpStrIdx = 0;
	treeNodes.append(new Root);
	nsStack.push(0);
}

void Lexer::LexerError(const char* msg) {
	ParseError pe;
	pe.msg = msg;
	pe.bline = lineno();
	throw pe;
}


Parser::token_type Lexer::operator() (Parser::semantic_type* lval, Parser::location_type* lloc) {
	yylval = lval;
	Parser::token_type tok = (Parser::token_type)yylex();
	if(lloc) {
		lloc->begin.line = lineno();
	}
	return tok;
}

int Lexer::allocString(const QString &str) {
	tmpStrings[tmpStrIdx] = str;
	return tmpStrIdx++;
}

QString Lexer::getString(int index) {
	QHash<int,QString>::iterator i = tmpStrings.find(index);
	if(i == tmpStrings.end())  return QString("<invalid>");
	return *i;
}

void Lexer::freeString(int idx) {
	tmpStrings.remove(idx);
}

int Lexer::resolve(QString id) {
	for(int i = nsStack.size()-1; i > 0; i--) {
		Struct *s = treeNodes[nsStack[i]]->toStruct();
		if(!s) continue;

		foreach(int index, s->children) {
			Node *tn = treeNodes[index];
			if(tn->classType() == tn->STRUCT_ENTRY) {
				StructEntry *se = tn->toStructEntry();
				if(se->name == id) return se->index;
			}
		}
	}
	return 0;
}

Node *Lexer::node(int index) {
	Q_ASSERT(index < treeNodes.size());
	return treeNodes[index];
}

int Lexer::newExpr(int type, QVariant value) {
	treeNodes.append(new Expr(this, type, value));
	return treeNodes.size() - 1;
}

int Lexer::newExpr(int type, int lhs, int rhs) {
	treeNodes.append(new Expr(this, type, lhs, rhs));
	return treeNodes.size() - 1;
}

void Lexer::addStructEntry(QString type, QString id, int sizeExpr) {
	StructEntry *se = new StructEntry;
	se->type = type;
	se->name = id;
	se->sizeExpr = sizeExpr;
	se->index = treeNodes.size();
	se->parent = nsStack.top();
	treeNodes.append(se);
	treeNodes[nsStack.top()]->children.append(treeNodes.size()-1);
}

void Lexer::beginStruct(QString name) {
	Struct *s = new Struct;
	s->name = name;
	s->parent = nsStack.top();
	treeNodes.append(s);
	treeNodes[nsStack.top()]->children.append(treeNodes.size()-1);
	nsStack.push(treeNodes.size()-1);
}

void Lexer::endStruct() {
	nsStack.pop();
}

void Lexer::traverse(Node *n) {
	QString indent;
	for(int i = 0; i < level; i++)
		indent += "    ";

	switch(n->classType()) {
	case Node::ROOT:
		foreach(int index, n->children) {
			traverse(treeNodes[index]);
		}
		break;
	case Node::STRUCT:
		saveString += indent + n->toStruct()->name + " {\n";
		foreach(int index, n->children) {
			level++;
			traverse(treeNodes[index]);
			level--;
		}
		saveString += indent + "}\n";
		break;
	case Node::STRUCT_ENTRY: {
		StructEntry *se = n->toStructEntry();
		if(se->sizeExpr) {
			saveString += indent + se->type + " " + se->name + "[";
			traverse(treeNodes[se->sizeExpr]);
			saveString += "]\n";
		} else {
			saveString += indent + se->type + " " + se->name + "\n";
		}
		break;}
	case Node::EXPR: {
		Expr *e = n->toExpr();
		switch(e->type()) {
		case Expr::INTEGER:
			saveString += e->value().toString();
			break;
		case Expr::FLOAT:
			saveString += e->value().toString();
			break;
		case Expr::VAR_REF: {
			int varIndex = e->value().toInt();
			if(!varIndex)  saveString += "<unresolved_field>";
			else saveString += node(varIndex)->toStructEntry()->name;
			break;}
		case '+':
			saveString += "(";
			traverse(e->l());
			saveString += " + ";
			traverse( e->r());
			saveString += ")";
			break;
		}
		break;}
	}
}

bool Lexer::parse(const QString &source) {
	clear();
	istringstream iss(string(source.toUtf8().data()));
	yyrestart(&iss);

	try {
		Parser parser(*this);
		parser.parse();
	}

	catch(ParseError pe) {
		errLine = pe.bline;
		errMsg = pe.msg;
		clear();
		return false;
	}
	return true;
}

QString Lexer::toString() {
	saveString = "";
	level = 0;
	traverse(treeNodes[0]);
	return saveString;
}

}

