/*
 * common.c
 * 
 * Copyright 2018-2020 Alvarito050506 <donfrutosgomez@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

unsigned char get_char(int num, int pos)
{
	return (char)(255 & (num >> pos));
}

unsigned short get_short(char* buff)
{
	unsigned short tmp = 0;
	unsigned char* ptr = (unsigned char*)&tmp;

	ptr[0] = buff[1];
	ptr[1] = buff[0];
	return tmp;
}

unsigned int get_int(char* buff)
{
	unsigned int tmp = 0;
	unsigned char* ptr = (unsigned char*)&tmp;

	ptr[0] = buff[3];
	ptr[1] = buff[2];
	ptr[2] = buff[1];
	ptr[3] = buff[0];
	return tmp;
}

int fast_printf(const char* string)
{
	write(STDOUT_FILENO, string, strlen(string));
	fflush(stdout);
	return 0;
}
