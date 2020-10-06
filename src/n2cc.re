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
#include <sys/types.h>
#include <sys/wait.h>

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
	reg = "$r" ([0-9] | "1" [0-5] | "0" [0-9]);
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
	"call" { return CALL; }
	reg { return REG; }
	name { return NAME; }
	"=" { return ASSIGN; }
	int { return INT; }
	str { return CSTR; }
	"{" { return LBR; }
	"}" { return RBR; }
	";" { return SEMICOLON; }
	"/*" { goto comment; }
	"//" { goto inline_comment; }
	"#" { goto inline_comment; }

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
		strcat(data, label.name);
		strcat(data, ":\n\t.data ");
		strcat(data, label.val);
		strcat(data, "\n");
	} else if (label.type == FUNC)
	{
		strcat(code, ".");
		strcat(code, label.name);
		strcat(code, ":\n");
	}
	return 0;
}

int write_code(label_t label, char* code)
{
	if (label.type == ASM)
	{
		label.val[0] = '\t';
		label.val[strlen(label.val) - 1] = '\n';
		strcat(code, label.val);
	} else if (label.type == CALL)
	{
		strcat(code, "\tlri 0x00, @");
		strcat(code, label.name);
		strcat(code, "\n\tjmp 0x00\n");
	} else if (label.assign == RR)
	{
		sprintf(code + strlen(code), "\tlrr 0x%02x, 0x%02x\n", atoi(label.name + 2), atoi(label.val + 2));
	} else if (label.assign == RA)
	{
		sprintf(code + strlen(code), "\tlri 0x%02x, @%s\n", atoi(label.val + 2), label.name);
	}
	return 0;
}

int compile(char* inp, char* file, char* code, char* data)
{
	lexer_t lexer;
	token_t tok;
	token_t func_any;
	label_t label;
	assign_t assign;
	pid_t pid;
	int func = 1;
	int reassign = 1;
	int level = 0;
	int excepted = ANY;
	int code_len = 0;
	unsigned int tmp_val = 0;

	lexer.top = inp;
	lexer.cur = inp;
	lexer.pos = inp;
	lexer.end = inp + strlen(inp);
	lexer.file = file;
	lexer.line = 1;

	func_any = VAR | NAME | REG | SEMICOLON | RETURN | ASM | LBR | RBR | CALL;

	strcpy(code, ".start:\n\tlri 0x00, @main\n\tjmp 0x00\n\thlt\n\n");

	tok = lex(&lexer);
	while (tok)
	{
		if ((excepted != ANY && (excepted & tok) != tok) || tok == -1)
		{
			COMP_ERROR("Unexcepted token: " BOLD "%c %i excepted %i" RESET ".\n", *(lexer.cur + 1), tok, excepted);
			puts(code);
			puts(data);
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
					excepted = LBR;
					func = 0;
					label.type = 0x00;
				} else if (label.type == CALL)
				{
					write_code(label, code);
					excepted = SEMICOLON;
					label.type = 0x00;
				} else if (label.type == REG)
				{
					label.assign = RA;
					write_code(label, code);
				} else
				{
					if (excepted == ANY || excepted == func_any)
					{
						reassign = 0;
					}
					excepted = ASSIGN;
				}
			break;
			case REG:
				if (label.type == REG)
				{
					label.val = get_token(&lexer);
					label.assign = RR;
					write_code(label, code);
				} else
				{
					label.val = get_token(&lexer);
					label.type = REG;
					excepted = ASSIGN;
				}
			break;
			case ASSIGN:
				if (reassign == 0)
				{
					excepted = INT;
				} else if (label.type == REG)
				{
					excepted = INT | NAME | REG;
				} else
				{
					excepted = INT | CSTR;
				}
			break;
			case INT:
				label.val = get_token(&lexer);
				code_len = strlen(code);
				if (label.type == VAR)
				{
					write_label(label, code, data);
					free(label.name);
				} else if (reassign == 0)
				{
					tmp_val = strtol(label.val, NULL, 16);
					if (tmp_val > UINT16_MAX)
					{
						sprintf(code + code_len, "\tlri 0x02, 0x%02x\n", tmp_val & ((1 << 16) - 1));
						sprintf(code + strlen(code), "\tlrt 0x02, 0x%02x\n", tmp_val >> 16);
					} else
					{
						sprintf(code + code_len, "\tlri 0x02, 0x%02x\n", tmp_val);
					}
					strcat(code, "\tlri 0x01, @");
					strcat(code, label.name);
					strcat(code, "\n\tlmr 0x01, 0x02\n");
					reassign = 1;
				} else if (excepted == (INT | SEMICOLON))
				{
					strcat(code, "\tlri 0x0e, ");
					strcat(code, label.val);
					strcat(code, "\n");
				} else if (label.type == REG)
				{
					tmp_val = strtol(label.val, NULL, 16);
					sprintf(code + strlen(code), "\tlri 0x%02x, 0x%02x\n", atoi(label.name + 2), tmp_val);
				}
				label.type = 0x00;
				excepted = SEMICOLON;
				free(label.val);
			break;
			case CSTR:
				label.val = get_token(&lexer);
				if (label.type == VAR)
				{
					write_label(label, code, data);
					free(label.name);
				} else
				{
					write_code(label, code);
				}
				label.type = 0x00;
				excepted = SEMICOLON;
				free(label.val);
			break;
			case SEMICOLON:
				if (func == 0)
				{
					excepted = func_any;
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
				excepted = CSTR;
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
			case CALL:
				excepted = NAME;
				label.type = CALL;
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
	int wstatus;
	char option;
	char* buff;
	char* tmp_buff;
	char* out_code;
	char* out_data;
	char* out_path;
	char* n2as_path;
	char** tmp_argv;
	FILE* input_fd;
	FILE* output_fd;
	pid_t pid;
	actions_t actions;

	char tmp_template[19] = "/tmp/.n2cc_XXXXXX";
	struct option long_opts[] = {
		{"output", required_argument, NULL, 'o'},
		{"help", no_argument, NULL, 'h'},
		{"no-preproc", no_argument, NULL, 'n'},
		{NULL, 0, NULL, 0}
	};

	out_path = "./a.out";
	actions = PREPROC | COMPILE | ASSEMBLE | OPTIMIZE;

	opterr = 0;
	option = getopt_long(argc, argv, "o:hnS", long_opts, &argci);
	while (option > 0 && option < 128)
	{
		switch (option)
		{
			case 'o':
				out_path = malloc(strlen(optarg) + 1);
				strcpy(out_path, optarg);
			break;
			case 'S':
				actions &= ~ASSEMBLE;
			break;
			case 'n':
				actions &= ~PREPROC;
			break;
			case 'h':
				printf("Usage: %s [options] file\n\n", argv[0]);
				printf("Options:\n");
				printf("  --help\tDisplay this help and exit.\n");
				printf("  --output=FILE\tPlace the output into <FILE>.\n");
				printf("  --no-preproc\tNo preprocess.\n");
				printf("  -o FILE\tSame as --output.\n");
				printf("  -h\t\tSame as --help.\n");
				printf("  -n\t\tSame as --no-preproc.\n");
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
		ERROR("Can't open the input file.\n");
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
		ERROR("Can't open the output file.\n");
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
			FORK(execl("/usr/bin/env", "env", "n2as", "-o", out_path, tmp_template, NULL));
		} else
		{
			FORK(execl(n2as_path, n2as_path, "-o", out_path, tmp_template, NULL));
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
//execl("/usr/bin/env", "env", "n2as", "-o", out_path, tmp_template, NULL);
