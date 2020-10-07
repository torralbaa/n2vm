/*
 * main.c
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
#include <stdlib.h>
#include <string.h>
//#include <time.h>
#include <unistd.h>

#include <proto.h>
#include <ops.h>

IO(vid, {
	VAL2REG();

	printf("[VID] %s", vm->mem + vm->gpr[src_reg]);
});

IO(gch, {
	VAL2REG();

	vm->gpr[src_reg] = CHR(X_GETCHAR(), 0);
});

int main(int argc, char* argv[])
{
	int io_index = 0;
	unsigned int sz = 0;
	n2vm_t* vm;
	FILE* fd;

	if (argc < 2)
	{
		ERROR("No file specified.\n");
		return -1;
	}

	if (strcmp("--help", argv[1]) == 0 || strcmp("-h", argv[1]) == 0)
	{
		printf("Usage: %s [options] file\n\n", argv[0]);
		printf("Options:\n");
		printf("  --help\tDisplay this help and exit.\n");
		printf("  -v\t\tDisplays the information about this VM.\n");
		return 0;
	} else if (strcmp("-v", argv[1]) == 0)
	{
		PRINT_INFO(VERSION);
		PRINT_INFO(MEM_MIN);
		PRINT_INFO(MEM_MAX);
		PRINT_INFO(STACK_MAX);
		PRINT_INFO(SYS_TAB_MAX);
		return 0;
	}

	fd = fopen(argv[1], "rb");
	if (fd == NULL)
	{
		ERROR("Can't open the file.\n");
		return -1;
	}

	vm = n2vm_new(MEM_MIN, MEM_MAX, STACK_MAX, SYS_TAB_MAX);
	if (n2vm_bind(vm, vid, &io_index) != 0)
	{
		ERROR("I/O port already in use.\n");
		n2vm_clean(vm);
		return -1;
	}
	io_index = 1;
	if (n2vm_bind(vm, gch, &io_index) != 1)
	{
		ERROR("I/O port already in use.\n");
		n2vm_clean(vm);
		return -1;
	}
	sz = fread(vm->mem, sizeof(unsigned char), vm->mem_sz, fd);

	n2vm_init();
	n2vm_run(vm);
	n2vm_clean(vm);

	fclose(fd);
	return 0;
}
