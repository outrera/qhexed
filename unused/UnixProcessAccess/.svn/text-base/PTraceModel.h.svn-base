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


#ifndef PTRACEMODEL_H
#define PTRACEMODEL_H

#include <HexDataModel.h>
#include <HexPlugin.h>

class PTraceModel : public HexDataModel {
public:
	PTraceModel();
	~PTraceModel();

	bool open(QString pid);

	void write(OffType dst, const void *src, OffType size);
	void read(void *dst, OffType src, OffType size);
	QString errorString();
	OffType getLength();
	bool isGrowable();
	bool isWriteable();
	OffType getPageSize();

private:

	QString mError;
	int mPid;
};

#endif

