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


#include <sys/wait.h>
#include <sys/ptrace.h>
#include <string.h>
#include <errno.h>

#include "PTraceModel.h"

PTraceModel::PTraceModel() {
	mPid = -1;
}

PTraceModel::~PTraceModel() {
	if(mPid > 0)
		ptrace(PTRACE_DETACH, mPid, NULL, NULL);
}

bool PTraceModel::open(QString pid) {
	if(pid.isEmpty() || (mPid = pid.toInt()) <= 0) {
		mError = "Invalid PID";
		mPid = -1;
		return 0;
	}

	if((ptrace(PTRACE_ATTACH, mPid, NULL, NULL)) < 0) {
		char buf[512];
		mError = strerror_r(errno, buf, 512);
		mPid = -1;
		return false;
	}
	waitpid(mPid, NULL, WUNTRACED);

	return true;
}

void PTraceModel::write(OffType dst, const void *src, OffType size) {
	long word;
	int count = 0;
	for(count = 0;  count < (int)size; count += 4) {
		memcpy(&word, (uint8_t*)src + count, sizeof(word));
		word = ptrace(PTRACE_POKETEXT, mPid, dst + count, word);
	}
}

void PTraceModel::read(void *dst, OffType src, OffType size) {
	int i, count;
	long word;
	count = i = 0;
	unsigned long *ptr = (unsigned long *) dst;
	while(count < (int)size) {
		word = ptrace(PTRACE_PEEKTEXT, mPid, src + count, NULL);
		count += 4;
		ptr[i++] = word;
	}
}

QString PTraceModel::errorString() {
	return mError;
}

OffType PTraceModel::getLength() {
	return 0xffffffff;
}

bool PTraceModel::isGrowable() {
	return false;
}

bool PTraceModel::isWriteable() {
	return true;
}

OffType PTraceModel::getPageSize() {
	return 1024;
}

