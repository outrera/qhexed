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

#include "FilterEditDialog.h"
#include "Filter.h"

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
	emit changed();
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

	connect(mLineEdit, SIGNAL(textChanged(const QString &)),
			this, SLOT(textChanged(const QString &)));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(mLabel);
	layout->addStretch();
	layout->addWidget(mLineEdit);
}

void LabeledValue::setValue(QString value) {
	mLineEdit->setText(value);
}

void LabeledValue::textChanged(const QString & text) {
	emit changed();
}


/////////////////////////////////////////////////////////////////////////////

FilterEditDialog::FilterEditDialog(Filter *f) {
	mFilter = f;
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	nameLV = new LabeledValue("Name:", mFilter->name);
	mainLayout->addWidget(nameLV);
	commandLV = new LabeledValue("Command:", mFilter->command);
	mainLayout->addWidget(commandLV);
	commentLV = new LabeledValue("Comment:", mFilter->comment);
	mainLayout->addWidget(commentLV);
	textInputLCB = new LabeledCheckBox("Takes Text as Input", mFilter->textInput);
	mainLayout->addWidget(textInputLCB);
	textOutputLCB = new LabeledCheckBox("Outputs Text", mFilter->textOutput);
	mainLayout->addWidget(textOutputLCB);

	connect(nameLV, SIGNAL(changed()), this, SLOT(updateFilter()));
	connect(commandLV, SIGNAL(changed()), this, SLOT(updateFilter()));
	connect(commentLV, SIGNAL(changed()), this, SLOT(updateFilter()));
	connect(textInputLCB, SIGNAL(changed()), this, SLOT(updateFilter()));
	connect(textOutputLCB, SIGNAL(changed()), this, SLOT(updateFilter()));

	mainLayout->addStretch();

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();

	QPushButton *okButton = new QPushButton(tr("&OK"));
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	buttonLayout->addWidget(okButton);

	QPushButton *cancelButton = new QPushButton(tr("&Cancel"));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	buttonLayout->addWidget(cancelButton);

	buttonLayout->addStretch();
	mainLayout->addLayout(buttonLayout);	
}

FilterEditDialog::~FilterEditDialog() {
}

void FilterEditDialog::updateFilter() {
	mFilter->name = nameLV->lineEdit()->text();
	mFilter->command =  commandLV->lineEdit()->text();
	mFilter->comment = commentLV->lineEdit()->text();
	mFilter->textInput = textInputLCB->checked();
	mFilter->textOutput = textOutputLCB->checked();
}
