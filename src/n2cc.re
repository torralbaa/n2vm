/*
 * n2cc.re
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
#include <unistd.h>
#include <getopt.h>

#include <types.h>
#include <misc.h>

token_t lex(lexer_t* lexer)
{
	/*!re2c
	re2c:yyfill:enable = 0;
	re2c:flags:tags = 1;

	any = [\x00-\xff];
	ident = [a-zA-Z_][a-zA-Z_0-9]*;
	end = [\x00];
	func = "func";
	lf = "\r\n" | "\n";
	blank = [ \t\v\f]+;
	int = "0x" [0-9a-f]+;
	str = ["] ([^"\n] | [\\][^])* ["];
	name = [a-zA-Z_][a-zA-Z_0-9]*;
	islash = [/];
	comm_end = "*" islash;
	*/

regular:
	if (YYCURSOR >= lexer->end)
	{
		return END;
	}
	lexer->top = lexer->cur;

	/*!re2c
	"var" { return VAR; }
	"asm" { return ASM; }
	"func" { return FUNC; }
	"return" { return RETURN; }
	name { return NAME; }
	"=" { return ASSIGN; }
	int { return INT; }
	str { return STR; }
	"{" { return LBR; }
	"}" { return RBR; }
	"(" { return LPA; }
	")" { return RPA; }
	";" { return SEMICOLON; }
	"/*" { goto comment; }
	"//" { goto inline_comment; }

	lf
	{
		lexer->pos = YYCURSOR;
		lexer->line++;
		goto regular;
	}

	blank { goto regular; }

	any { return -1; }
	*/

comment:
	if (YYCURSOR >= lexer->end)
	{
		return END;
	}
	/*!re2c
	comm_end { goto regular; }

	lf
	{
		lexer->pos = YYCURSOR;
		lexer->line++;
		goto comment;
	}

	any { goto comment; }
	*/

inline_comment:
	if (YYCURSOR >= lexer->end)
	{
		return END;
	}
	/*!re2c
	lf
	{
		lexer->pos = YYCURSOR;
		lexer->line++;
		goto regular;
	}

	any { goto inline_comment; }
	*/
	return 0;
}

char* get_token(lexer_t* lexer)
{
	int len;
	char* token;

	len = (int)(YYCURSOR - lexer->top);
	token = malloc(len + 1);
	strncpy(token, (char*)lexer->top, (size_t)(len));
	token[len] = 0x00;
	return token;
}

int write_label(label_t label, char* code, char* data)
{
	if (label.type == VAR)
	{
		strcat(data, ".");
		strncat(data, label.name, strlen(label.name));
		strcat(data, ":\n\t.data ");
		strncat(data, label.val, strlen(label.val));
		strcat(data, "\n");
	} else if (label.type == ASM)
	{
		label.val[0] = '\t';
		label.val[strlen(label.val) - 1] = '\n';
		strncat(code, label.val, strlen(label.val));
	} else if (label.type == FUNC)
	{
		strcat(code, ".");
		strncat(code, label.name, strlen(label.name));
		strcat(code, ":\n");
	}
	return 0;
}

int compile(char* inp, char* file, char* code, char* data)
{
	lexer_t lexer;
	token_t tok;
	label_t label;
	int func = 1;
	int reassign = 1;
	int level = 0;
	int excepted = ANY;
	unsigned int tmp_val = 0;

	lexer.top = inp;
	lexer.cur = inp;
	lexer.pos = inp;
	lexer.end = inp + strlen(inp);
	lexer.file = file;

	strcpy(code, ".start:\n\tlri 0x00, @main\n\tjmp 0x00\n\thlt\n\n");

	tok = lex(&lexer);
	while (tok)
	{
		if ((excepted != ANY && (excepted & tok) != tok) || tok == -1)
		{
			COMP_ERROR("Unexcepted token: " BOLD "%c" RESET ".\n", *(lexer.cur));
			return -1;
		}
		switch (tok)
		{
			case VAR:
				excepted = NAME;
				label.type = VAR;
			break;
			case NAME:
				label.name = get_token(&lexer);
				if (label.type == FUNC)
				{
					write_label(label, code, data);
					label.type = 0;
					excepted = LBR;
					func = 0;
				} else
				{
					if (excepted == ANY || excepted == (FUNC_ANY))
					{
						reassign = 0;
					}
					excepted = ASSIGN;
				}
			break;
			case ASSIGN:
				if (reassign == 0)
				{
					excepted = INT;
				} else
				{
					excepted = INT | STR;
				}
			break;
			case INT:
				label.val = get_token(&lexer);
				if (label.type == VAR)
				{
					write_label(label, code, data);
					free(label.name);
				} else if (reassign == 0)
				{
					tmp_val = strtol(label.val, NULL, 16);
					if (tmp_val > INT16_MAX)
					{
						sprintf(code + strlen(code), "\tlri 0x00, 0x%02x\n", tmp_val & ((1 << 16) - 1));
						sprintf(code + strlen(code), "\tlrt 0x00, 0x%02x\n", tmp_val >> 16);
					} else
					{
						sprintf(code + strlen(code), "\tlri 0x00, 0x%02x\n", tmp_val);
					}
					sprintf(code + strlen(code), "\tlri 0x01, @%s\n", label.name);
					strcat(code, "\tlmr 0x00, 0x01\n");
					reassign = 1;
				} else if (excepted == (INT | SEMICOLON))
				{
					strcat(code, "\tlri 0x0e, ");
					strncat(code, label.val, strlen(label.val));
					strcat(code, "\n");
				}
				label.type = 0;
				excepted = SEMICOLON;
				free(label.val);
			break;
			case STR:
				label.val = get_token(&lexer);
				write_label(label, code, data);
				if (label.type == VAR)
				{
					free(label.name);
				}
				label.type = 0;
				excepted = SEMICOLON;
				free(label.val);
			break;
			case SEMICOLON:
				if (func == 0)
				{
					excepted = FUNC_ANY;
				} else
				{
					excepted = ANY;
				}
			break;
			case RETURN:
				excepted = INT | SEMICOLON;
			break;
			case ASM:
				label.name = NULL;
				label.type = ASM;
				excepted = STR;
			break;
			case FUNC:
				label.type = FUNC;
				excepted = NAME;
			break;
			case LBR:
				excepted = ANY;
				level++;
			break;
			case RBR:
				excepted = ANY;
				if (level == 1 && func == 0)
				{
					strcat(code, "\tret\n");
				}
				level--;
			break;
			case END:
			break;
			default:
				COMP_ERROR("Unknown token: " BOLD "%c" RESET ".\n", *(lexer.cur));
				return -1;
			break;
		}
		tok = lex(&lexer);
	}
	return 0;
}

int free_buffs(char* input, char* code, char* data)
{
	free(input);
	free(code);
	free(data);
	return 0;
}

int main(int argc, char* argv[])
{
	int sz = 0;
	int argci = 0;
	int asm_fd = 0;
	char option;
	char* buff;
	char* out_code;
	char* out_data;
	char* out_path;
	char* n2as_path;
	FILE* input_fd;
	FILE* output_fd;
	actions_t actions;

	char tmp_template[19] = "/tmp/.n2cc_XXXXXX";
	struct option long_opts[] = {
		{"output", required_argument, NULL, 'o'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	out_path = "./a.out";
	actions = COMPILE | ASSEMBLE | OPTIMIZE;

	opterr = 0;
	option = getopt_long(argc, argv, "o:hS", long_opts, &argci);
	while (option > 0 && option < 128)
	{
		switch (option)
		{
			case 'o':
				out_path = malloc(strlen(optarg) + 1);
				strncpy(out_path, optarg, strlen(optarg));
			break;
			case 'S':
				actions &= ~ASSEMBLE;
			break;
			case 'h':
				printf("Usage: %s [options] file\n\n", argv[0]);
				printf("Options:\n");
				printf("  --help\tDisplay this help and exit.\n");
				printf("  --output=FILE\tPlace the output into <FILE>.\n");
				printf("  -o FILE\tSame as --output.\n");
				printf("  -h\t\tSame as --help.\n");
				printf("  -S\t\tCompile only; do not assemble.\n");
				return 0;
			break;
			case '?':
				if (optopt == 'c')
				{
					ERROR("Option " BOLD "-%c" RESET " requires an argument.\n", optopt);
				} else
				{
					ERROR("Unknown option " BOLD "-%c" RESET ".\n", optopt);
				}
				return -1;
			break;
			case ':':
				ERROR("Option " BOLD "-%c" RESET " requires an argument.\n", optopt);
				return -1;
			break;
			default:
			break;
		}
		option = getopt_long(argc, argv, "o:hS", long_opts, &argci);
	}

	if (optind >= argc)
	{
		ERROR("No input file.\n");
		return -1;
	}

	input_fd = fopen(argv[optind], "r");
	if (input_fd == NULL)
	{
		ERROR("Can't open the input file.");
		return -1;
	}
	fseek(input_fd, 0, SEEK_END);
	sz = ftell(input_fd);
	fseek(input_fd, 0, SEEK_SET);

	buff = malloc(sz + 1);
	fread(buff, 1, sz, input_fd);
	fclose(input_fd);

	output_fd = fopen(out_path, "wb");
	if (input_fd == NULL)
	{
		ERROR("Can't open the output file.");
		free(buff);
		return -1;
	}

	out_code = (char*)malloc(32768);
	out_data = (char*)malloc(32768);

	if (compile(buff, argv[optind], out_code, out_data) != 0)
	{
		free_buffs(buff, out_code, out_data);
		fclose(output_fd);
		return -1;
	}

	if ((actions & ASSEMBLE) == ASSEMBLE)
	{
		asm_fd = mkstemp(tmp_template);
		write(asm_fd, out_code, strlen(out_code));
		write(asm_fd, out_data, strlen(out_data));
		n2as_path = getenv("AS");
		if (n2as_path == NULL)
		{
			execl("/usr/bin/env", "env", "n2as", "-o", out_path, tmp_template, NULL);
		} else
		{
			execl(n2as_path, n2as_path, "-o", out_path, tmp_template, NULL);
		}
		unlink(tmp_template);
		close(asm_fd);
	} else
	{
		fwrite(out_code, 1, strlen(out_code), output_fd);
		fwrite(out_data, 1, strlen(out_data), output_fd);
	}

	fclose(output_fd);
	free_buffs(buff, out_code, out_data);
	return 0;
}
