/*
 * proto.h
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

#ifndef PROTO_H
#define PROTO_H

#ifdef N2VM_LIB_INT
	#include <types.h>
#else
	#include <n2vm/types.h>
#endif /* N2VM_LIB_INT */

#ifdef N2VM_LIB_INT
	unsigned char get_char(int num, int pos);
	unsigned int get_int(char* buff);
	unsigned short get_short(char* buff);
#endif /* N2VM_LIB_INT */

int n2vm_init();
n2vm_t* n2vm_new(int mem_min, int mem_max, int stack_max, int sys_max);
int n2vm_run(n2vm_t* vm);
int n2vm_clean(n2vm_t* vm);

#endif /* PROTO_H */
