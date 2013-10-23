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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>

#include <QtGui>
#include "OpenProcessDialog.h"


OpenProcessDialog::OpenProcessDialog(QWidget *parent)
	: QDialog(parent)
{

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	QHBoxLayout *buttonsLayout = new QHBoxLayout;

	okButton = new QPushButton(tr("&Ok"));
	okButton->setEnabled(false);
	cancelButton = new QPushButton(tr("&Cancel"));
	connect(okButton, SIGNAL(pressed()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(pressed()), this, SLOT(reject()));

	QList<ProcDesc> pidList = listPids();
	tableWidget = new QTableWidget(pidList.size(), 3);

	QStringList horizontalHeaders;
	horizontalHeaders << "PID" << "User" << "Command";
	tableWidget->setHorizontalHeaderLabels(horizontalHeaders);
	for(int i = 0; i < pidList.size(); i++) {
		tableWidget->setItem(i, 0, new QTableWidgetItem(pidList.at(i).pid));
		tableWidget->setItem(i, 1, new QTableWidgetItem(pidList.at(i).user));
		tableWidget->setItem(i, 2, new QTableWidgetItem(pidList.at(i).comm));
	}
	tableWidget->verticalHeader()->hide();
	tableWidget->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
	tableWidget->resizeRowsToContents();
	tableWidget->resizeColumnsToContents();
	tableWidget->setAlternatingRowColors(true);
	tableWidget->setSortingEnabled(true);
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	tableWidget->setShowGrid(false);
	connect(tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(selectRow(int,int)));
	connect(tableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(acceptRow(int,int)));

	mainLayout->addWidget(tableWidget);
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);
	buttonsLayout->addStretch();
	mainLayout->addLayout(buttonsLayout);
	curRow = -1;
}

QString OpenProcessDialog::ask(const QString &title) {
	OpenProcessDialog *lpd = new OpenProcessDialog(qApp->activeWindow());
	lpd->setWindowTitle(title);
	lpd->exec();
	QString ret;
	if(lpd->result() == lpd->Accepted) {
		ret = lpd->getPid();
	} else {
		ret = QString();
	}
	return ret;
}

QList<OpenProcessDialog::ProcDesc> OpenProcessDialog::listPids() {
	QList<ProcDesc> ret;

	const int bufSize = 1<<15;
	char *buf = new char[1<<15];

	sprintf(buf, "ps -A -o pid,ruser,comm");

	FILE *cmdout = popen(buf, "r");

	if(fgets(buf, bufSize, cmdout)) {
		while(fgets(buf, bufSize, cmdout)) {
			char pid[256], user[256], comm[256];
			ProcDesc desc;
			if(!buf) continue;

			// NOTE: stack overflow is possible here
			sscanf(buf, "%s %s %s", pid, user, comm);
			desc.pid = pid;
			desc.user = user;
			desc.comm = comm;

			ret.append(desc);
		}
	}

	// we dont need descriptor anymore
	fclose(cmdout);
	delete buf;

	return ret;
}

void OpenProcessDialog::selectRow(int row, int column) {
	pid = tableWidget->item(row, 0)->text();
	curRow = row;
	okButton->setEnabled(true);
}


void OpenProcessDialog::acceptRow(int row, int column) {
	pid = tableWidget->item(row, 0)->text();
	accept();
}


#if 0
extern char *mybasename (char *);
static char *parse_parens (char *buf);

int *get_pids (char *process_name, int get_all) {
	DIR *dir;
	struct dirent *ent;
	int status;
	char *dname, fname[100], *cp, buf[256];
	struct stat st;
	uid_t uid;
	FILE *fp;
	int pid, *pids, num_pids, pids_size;

	dir = opendir ("/proc");
	if (! dir) {
		perror ("opendir /proc");
		return NULL;
	}
	uid = getuid ();
	pids = NULL;
	num_pids = pids_size = 0;

	while ((ent = readdir (dir)) != NULL) {
		dname = ent->d_name;
		if (! isdigit (*dname)) continue;
		pid = atoi (dname);
		sprintf (fname, "/proc/%d/cmdline", pid);
		// get the process owner
		status = stat (fname, &st);
		if (status != 0) continue;
		if (! get_all && uid != st.st_uid) continue;
		// get the command line
		fp = fopen (fname, "r");
		if (! fp) continue;
		cp = fgets (buf, sizeof (buf), fp);
		fclose (fp);
		// an empty command line means the process is swapped out
		if (! cp || ! *cp) {
			// get the process name from the statfile
			sprintf (fname, "/proc/%d/stat", pid);
			fp = fopen (fname, "r");
			if (! fp) continue;
			cp = fgets (buf, sizeof (buf), fp);
			if (cp == NULL) continue;
			fclose (fp);
			cp = parse_parens (buf);
			if (cp == NULL) continue;
		}
		// ok, we got the process name.
		if (strcmp (process_name, mybasename (cp))) continue;
		while (pids_size < num_pids + 2) {
			pids_size += 5;
			pids = (int *) realloc (pids, sizeof (int) * pids_size);
		}
		pids[num_pids++] = pid;
		pids[num_pids] = -1;
	}
	closedir (dir);
	return (pids);
}

char *mybasename (char *path) {
	char *cp;

	cp = strrchr (path, '/');
	return (cp ? cp + 1 : path);
}



//  parse_parens () -- return an index just past the first open paren in
//	buf, and terminate the string at the matching close paren.
static char *parse_parens (char *buf) {
	char *cp, *ip;
	int depth;

	cp = strchr (buf, '(');
	if (cp == NULL) return NULL;
	cp++;
	depth = 1;
	for (ip = cp; *ip; ip++) {
		if (*ip == '(')
			depth++;
		if (*ip == ')') {
			depth--;
			if (depth == 0) {
				*ip = 0;
				break;
			}
		}
	}
	return cp;
}
#endif
