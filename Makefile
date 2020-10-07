#
#  Makefile
#  
#  Copyright 2020 Alvarito050506 <donfrutosgomez@gmail.com>
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; version 2 of the License.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#  
#  

CFLAGS:=-I./include -DN2VM_LIB_INT -O3 -fPIC -DPIC -DNDEBUG
CC_CFLAGS:=-DN2CC

all: cc dir lib test
	gcc ./src/n2vm.c ./src/main.c $(CFLAGS) $(EXTRA_CFLAGS) -DMAIN -o ./build/n2vm

cc: dir
	re2c ./src/n2cc.re -o ./build/n2cc.c
	gcc ./build/n2cc.c $(CFLAGS) $(CC_CFLAGS) $(EXTRA_CFLAGS) -o ./build/n2cc

lib: dir
	gcc -shared -fPIC ./src/n2vm.c $(CFLAGS) $(EXTRA_CFLAGS) -o ./build/libn2vm.so

test: dir
	@./build/n2cc -o ./build/n2_test ./test/test.n2
	@python3 ./src/n2as.py -o ./build/asm_test ./test/test.asm
	@python3 ./src/n2as.py -o ./build/base ./test/base.asm
	@python3 ./src/n2as.py -o ./build/emp ./test/emp.asm
	@cat ./build/base ./build/emp > ./build/os

dir:
	mkdir -p ./build/

install:
	@cp ./build/n2vm /usr/bin/n2vm
	@cp ./build/n2cc /usr/bin/n2cc
	@cp ./src/n2as.py /usr/bin/n2as
	@cp ./build/libn2vm.so /usr/lib/libn2vm.so
	@mkdir -p /usr/include/n2vm/
	@cp -r ./include/. /usr/include/n2vm/
	@chmod 0755 /usr/lib/libn2vm.so
	@chmod a+x /usr/bin/n2vm
	@chmod a+x /usr/bin/n2cc
	@chmod a+x /usr/bin/n2as

clean:
	rm -rf ./build/
