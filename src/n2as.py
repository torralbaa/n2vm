#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
#  n2as.py
#  
#  Copyright 2020 Alvarito050506 <donfrutosgomez@gmail.com>
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
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

import re
import sys
import struct
from getopt import *
from codecs import encode, decode

base_regex = r"\s*(.*)\s*";
inst_regex = r"([a-z]{3})";
reg_regex = r"(0x[0-9a-f]{1,2})";
val_regex = r"(0x[0-9a-f]{1,4}|@[_a-z][_a-z0-9]*)";
cond_regex = r"(0x[0-9a-f]{1,4}|@[_a-z][_a-z0-9]*)";
op_regex = f"^\s*{inst_regex}\s+({reg_regex}(\s*,\s*{val_regex})?)?(\s*!\s*(eq|ne|gt|lt|al))?\s*(;.*)?\s*$";
ignore_regex = r"^(\s*|\s*;(.*)\s*)$";
label_regex = r"^\s*(\.([_a-z][_a-z0-9]*):\s*)\s*$";
data_regex = r"^\s*\.data\s+(\"(.+)\"|(0x[0-9a-f]{1,8}))\s*$";
ops = {
	"nop": 0x00,
	"inc": 0x01,
	"dec": 0x02,
	"add": 0x03,
	"sub": 0x04,
	"mul": 0x05,
	"div": 0x06,
	"lri": 0x07,
	"lrt": 0x08,
	"lrr": 0x09,
	"lrc": 0x0a,
	"lrh": 0x0b,
	"lrm": 0x0c,
	"lmc": 0x0d,
	"lmh": 0x0e,
	"lmr": 0x0f,
	"jmp": 0x10,
	"cmp": 0x11,
	"shl": 0x12,
	"shr": 0x13,
	"ior": 0x14,
	"xor": 0x15,
	"and": 0x16,
	"not": 0x17,
	"out": 0x18,
	"inp": 0x19,
	"cll": 0x1a,
	"sys": 0x1b,
	"ret": 0x1c,
	"hlt": 0x1d
};
conds = {
	"al": 0b0000,
	"eq": 0b1000,
	"ne": 0b0100,
	"gt": 0b0010,
	"lt": 0b0001
};
vtable = dict();
line_num = 0;
file_name = "";
i = 0;

INT16_MAX = 65535;
INT24_MAX = 16777215;
INT32_MAX = 4294967295;

RESET = "\033[0m";
BOLD = "\033[1m";
ERR = "\033[91m";

def basename(path):
	return path.split("/")[-1];

def error(msg):
	return print(f"{BOLD}{ERR}Error:{RESET} {msg}");

def comp_error(msg, code):
	return print(f"{BOLD}{file_name}:{line_num} {ERR}Error:{RESET} {msg}\n\t{BOLD}{ERR}{code}{RESET}");

def comp(asm):
	match = re.fullmatch(op_regex, asm);
	plain = re.fullmatch(base_regex, asm).group(1);
	op = 0x00;
	reg = 0x00;
	val = 0x00;
	cond = 0b0000;
	tmp_reg = "";
	tmp_val = "";

	if match is None:
		comp_error("Invalid assembly syntax.", plain);
		return (None, -1);

	try:
		op = ops[match.group(1)];
	except:
		comp_error(f"Invalid instruction: {BOLD}{match.group(1)}{RESET}.", plain);
		return (None, -1);

	try:
		tmp_reg = match.group(3);
		tmp_val = match.group(5);
		reg = int(tmp_reg, 16);
		if tmp_val[0] == "@":
			try:
				val = vtable[tmp_val[1:]];
			except (NameError, KeyError):
				comp_error(f"Invalid label: {BOLD}{tmp_val[1:]}{RESET}.", plain);
				return (None, -1);
		else:
			val = int(tmp_val, 16);
	except (IndexError, TypeError):
		pass;

	try:
		cond = conds[match.group(7)];
	except KeyError:
		pass;

	if reg > 15:
		comp_error(f"Invalid register: {BOLD}{tmp_reg}{RESET}.", plain);
		return (None, -1);

	if val > INT16_MAX:
		comp_error(f"Integer overflow: {BOLD}{val} > INT16_MAX ({INT16_MAX}){RESET}.", plain);
		return (None, -1);

	return (struct.pack(">H", (op << 8) | (cond << 4) | reg) + struct.pack("<H", val), 0);

def main(argc, argv):
	global line_num;
	global file_name;
	global i;

	inp = None;
	out = None;
	dl_match = None;
	data = None;

	try:
		opts, args = getopt(argv[1:], "o:", ["output=", "help"]);
	except GetoptError as err:
		error(f"In option --{err.opt}.");
		return -1;

	for opt, arg in opts:
		if opt in ("-o", "--output"):
			out = open(arg, "wb");
		elif opt == "--help":
			print(f"Usage: {basename(sys.argv[0])} [options] file\n");
			print("Options:");
			print("  --help\tDisplay this help and exit.");
			print("  --output=FILE\tPlace the output into <FILE>.");
			print("  -o FILE\tSame as --output.");
			print("  -h\t\tSame as --help.");
			return 0;

	if len(args) < 1:
		error("No input file.");
		return -1;

	try:
		inp = open(args[0], "r");
		file_name = args[0];
	except FileNotFoundError:
		error("Can't open the file.");
		return -1;

	if out is None:
		out = open("./a.out", "wb");

	for line in inp:
		if re.fullmatch(ignore_regex, line) is not None:
			continue;
		else:
			dl_match = re.fullmatch(data_regex, line);
			if dl_match is not None:
				data = decode(encode(dl_match.group(1), "utf-8", "backslashreplace"), "unicode-escape");
				try:
					val = int(data, 16);
					i += 4;
				except (IndexError, TypeError, ValueError):
					i += len(data) - 2;
					pass;
			else:
				dl_match = re.fullmatch(label_regex, line);
				if dl_match is not None:
					vtable.update({
						dl_match.group(2): i
					});
				else:
					i += 4;

	inp.seek(0);
	for line in inp:
		line_num += 1;
		if re.fullmatch(ignore_regex, line) is not None:
			continue;
		else:
			dl_match = re.fullmatch(data_regex, line);
			if dl_match is not None:
				data = decode(encode(dl_match.group(1), "utf-8", "backslashreplace"), "unicode-escape");
				try:
					val = int(data, 16);
					out.write(struct.pack("!I", val));
					i += 4;
				except (IndexError, TypeError, ValueError):
					i += len(data) - 2;
					out.write(bytes(data[1:-1], "utf-8"));
					pass;
				continue;
			elif re.fullmatch(label_regex, line) is not None:
				continue;
		compiled, errno = comp(line);
		if errno < 0:
			inp.close();
			out.close();
			return -1;
		out.write(compiled);

	inp.close();
	out.close();

if __name__ == "__main__":
	sys.exit(main(len(sys.argv), sys.argv));
