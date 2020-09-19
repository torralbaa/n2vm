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

CFLAGS:=-I./include

all: cc
	mkdir -p ./build/
	gcc ./src/n2vm.c $(CFLAGS) -o ./build/n2vm

cc:
	re2c ./src/n2cc.re -o ./build/n2cc.c
	gcc ./build/n2cc.c $(CFLAGS) -o ./build/n2cc

install:
	cp ./build/n2vm /usr/bin/n2vm
	cp ./build/n2cc /usr/bin/n2cc
	cp ./src/n2as.py /usr/bin/n2as
	chmod a+x /usr/bin/n2vm
	chmod a+x /usr/bin/n2cc
	chmod a+x /usr/bin/n2as
