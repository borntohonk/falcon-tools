/*
* Copyright (c) 2018 Reisyukaku
* Copyright (c) 2022 EliseZeroTwo <mail@elise.moe>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms and conditions of the GNU General Public License,
* version 2, as published by the Free Software Foundation.
*
* This program is distributed in the hope it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _FS_H_
#define _FS_H_

#include <stddef.h>

unsigned int sdMount();
void sdUnmount();
unsigned int fopen(const char *path, const char *mode);
unsigned int fread(void *buf, size_t size, size_t ntimes);
unsigned int fwrite(void *buf, size_t size, size_t ntimes);
size_t fsize();
void fclose();
size_t enumerateDir(char ***output, char *path, char *pattern);

#endif