#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
#  compiler.py
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
op_regex = r"^\s*([a-z]{3})(\s+(0x[0-9a-f]{1,7})\s*(,\s*(0x[0-9a-f]{1,7}))*)*\s*$";
ignore_regex = r"^((\.([a-z]+):\s*)|\s*|\s*;(.*)\s*)$";
data_regex = r"^\s*\.data\s+\"(.+)\"\s*$";
ops = {
	"nop": 0x00,
	"inc": 0x01,
	"dec": 0x02,
	"add": 0x03,
	"sub": 0x04,
	"mul": 0x05,
	"div": 0x06,
	"lri": 0x07,
	"lrr": 0x08,
	"lrm": 0x09,
	"lmi": 0x0a,
	"lmr": 0x0b,
	"lmm": 0x0c,
	"jmp": 0x0d,
	"jif": 0x0e,
	"jno": 0x0f,
	"shl": 0x10,
	"shr": 0x11,
	"ior": 0x12,
	"xor": 0x13,
	"and": 0x14,
	"not": 0x15,
	"out": 0x17,
	"hlt": 0x18
};

INT24_MAX = 16777215;

RESET = "\033[0m";
BOLD = "\033[1m";
ERR = "\033[91m";

def error(msg):
	return print(f"{BOLD}{ERR}Error:{RESET} {msg}");

def comp_error(msg, code):
	return error(f"{msg}\n\t{BOLD}{ERR}{code}{RESET}");

def comp(asm):
	match = re.fullmatch(op_regex, asm);
	plain = re.fullmatch(base_regex, asm).group(1);
	op = 0x00;
	reg = 0x00;
	val = 0x00;

	if match is None:
		comp_error("Invalid assembly syntax.", plain);
		return -1;

	try:
		op = ops[match.group(1)];
	except:
		comp_error(f"Invalid instruction: {BOLD}{match.group(1)}{RESET}.", plain);
		return -1;

	try:
		reg = int(match.group(3), 16);
		val = int(match.group(5), 16);
	except (IndexError, TypeError):
		pass;

	if reg > 7:
		comp_error(f"Invalid register: {BOLD}{match.group(2)}{RESET}.", plain);
		return -1;

	if val > INT24_MAX:
		comp_error(f"Integer overflow: {BOLD}{val} > INT24_MAX ({INT24_MAX}){RESET}.", plain);
		return -1;

	return (op << 27) | (reg << 24) | val;

def main(argc, argv):
	inp = None;
	out = None;
	data_match = None;
	data = None;

	try:
		opts, args = getopt(argv[1:], ":o:", ["output=", "help"]);
	except GetoptError as err:
		error(f"In option --{err.opt}.");
		return -1;

	for opt, arg in opts:
		if opt in ("-o", "--output"):
			out = open(arg, "wb");
		elif opt == "--help":
			print(f"Usage: {sys.argv[0]} [options] file\n");
			print("Options:");
			print("  --help\tDisplay this help and exit.");
			print("  --output=FILE\tPlace the output into <FILE>.");
			print("  -o FILE\tSame as --output.");
			return 0;

	if len(args) < 1:
		error("No input file.");
		return -1;

	inp = open(args[0], "r");
	if out is None:
		out = open("./a.out", "wb");

	for line in inp:
		if re.fullmatch(ignore_regex, line) is not None:
			continue;
		else:
			data_match = re.fullmatch(data_regex, line);
			if data_match is not None:
				data = decode(encode(data_match.group(1), "utf-8", "backslashreplace"), "unicode-escape");
				out.write(bytes(data, "utf-8"));
				continue;
		compiled = comp(line);
		if compiled < 0:
			inp.close();
			out.close();
			return -1;
		out.write(struct.pack("!I", compiled));

	inp.close();
	out.close();

if __name__ == "__main__":
	sys.exit(main(len(sys.argv), sys.argv));
