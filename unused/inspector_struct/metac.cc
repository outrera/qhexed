#include <QtGui>
#include "lexer.h"

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("usage: %s <filename.mc>\n", argv[0]);
		return -1;
	}

	QFile file(argv[1]);
	if(!file.open(QFile::ReadOnly)) {
		printf("failed to open input file\n");
		return -1;
	}
	QTextStream ts(&file);
	QString inText = ts.readAll();

	MCParser parser;
	for(int i = 0; i < 1; i++) {
		if(!parser.parse(inText)) {
			printf("Parse error at %d: %s\n", parser.errorLine(), parser.errorMessage().toUtf8().data());
			return -1;
		}
		printf("%s", parser.toString().toUtf8().data());
	}

	return 0;
}

