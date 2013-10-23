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

%skeleton "lalr1.cc"
%parse-param {Lexer& yylex}
%define "parser_class_name" "Parser"
%name-prefix="NAMESPACE"
// generate location processing code
%locations
%defines

%{

namespace NAMESPACE {
class Lexer;
}

using namespace std;
#include <stdio.h>
#include <stdexcept>
%}

%union {
	int i;
}

%{
#include <QtCore>
#include "lexer.h"
%}

%token END			0
%token STRUCT
%token <i> TYPE
%token <i> INTEGER
%token <i> FLOAT
%token <i> ID

%type <i> expr
%type <i> struct_

//%destructor { delete $$; } ID
//%destructor { delete $$; } TYPE

%right '=' AADD ASUB AMUL ADIV AREM AAND AOR
%left LOR /* logical or */
%left LAND /* logical and */
%left '|'
%left '^'
%left '&'
%left EQ NE
%left GTE LTE
%left SHL SHR
%left '+' '-'
%left '*' '/' '%'
%left '.' PTRMEMBER
//%left NEG     /* negation--unary minus */
%right NEG '~' '!' PTRDEREF ADROF INC DEC SIZEOF

%start global

%%

global:	/* empty */
	|	global decl
;

decl:
		struct_ ';'
	|	struct_ ID ';'

struct_:
		STRUCT ID '{' structBody '}' {
			$$ = $4
			yylex.endStruct();
		}
;

structBody:
	|	structBody struct_
	|	structBody structEntry
;


structEntry:
		TYPE ID ';' {
			yylex.addStructEntry(yylex.getString($1), yylex.getString($2), 0);
			yylex.freeString($1);
			yylex.freeString($2);
		}
	|	TYPE ID '[' expr ']' ';' {
			yylex.addStructEntry(yylex.getString($1), yylex.getString($2), $4);
			yylex.freeString($1);
			yylex.freeString($2);
		}

;


expr:
		INTEGER
	|	FLOAT
	|	ID {
			$$ = yylex.newExpr(Expr::VAR_REF, yylex.resolve(yylex.getString($1)));
			yylex.freeString($1);
		}
	|	expr '+' expr			{ $$ = yylex.newExpr('+', $1, $3);}
	|	expr '-' expr			{ $$ = yylex.newExpr('-', $1, $3);}
	|	expr '*' expr			{ $$ = yylex.newExpr('*', $1, $3);}
	|	expr '/' expr			{ $$ = yylex.newExpr('/', $1, $3);}
	|	expr '%' expr			{ $$ = yylex.newExpr('%', $1, $3);}
	|	expr SHL expr			{ $$ = yylex.newExpr(Expr::SHL, $1, $3);}
	|	expr SHR expr			{ $$ = yylex.newExpr(Expr::SHR, $1, $3);}
	|	expr '|' expr			{ $$ = yylex.newExpr('|', $1, $3);}
	|	expr '^' expr			{ $$ = yylex.newExpr('^', $1, $3);}
	|	expr '&' expr			{ $$ = yylex.newExpr('&', $1, $3);}
	|	expr LAND expr			{ $$ = yylex.newExpr(Expr::LAND, $1, $3);}
	|	expr LOR expr			{ $$ = yylex.newExpr(Expr::LOR, $1, $3);}
	|	'-' expr  %prec NEG		{ $$ = yylex.newExpr(Expr::NEG, $2, 0);}
	|	'(' expr ')'			{ $$ = $2;}
;


%%

namespace NAMESPACE {

void Parser::error(
	const location_type& loc, 
	const std::string& msg
) {
	ParseError pe;
	pe.msg = msg.c_str();
	pe.bline = (int)loc.begin.line;
	pe.bcol = (int)loc.begin.column;
	pe.eline = (int)loc.end.line;
	pe.ecol = (int)loc.end.column;
	throw pe;
}

}

