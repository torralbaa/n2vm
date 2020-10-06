#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto.h>

int main(int argc, char* argv[])
{
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
	sz = fread(vm->mem, sizeof(unsigned char), vm->mem_sz, fd);

	n2vm_init();
	n2vm_run(vm);
	n2vm_clean(vm);

	fclose(fd);
	return 0;
}
