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
	cond = "!" ("eq"|"ne"|"gt"|"lt"|"al");
	*/

regular:
	if (YYCURSOR >= lexer->end)
	{
		return TOK_END;
	}
	lexer->top = lexer->cur;

	/*!re2c
	"var" { return TOK_VAR; }
	"asm" { return TOK_ASM; }
	"func" { return TOK_FUNC; }
	"return" { return TOK_RETURN; }
	"call" { return TOK_CALL; }
	"cmp" { return TOK_CMP; }
	"case" { return TOK_CASE; }
	reg { return TOK_REG; }
	name { return TOK_NAME; }
	cond { return TOK_COND; }
	"=" { return TOK_ASSIGN; }
	int { return TOK_INT; }
	str { return TOK_CSTR; }
	"{" { return TOK_LBR; }
	"}" { return TOK_RBR; }
	";" { return TOK_SEMICOLON; }
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
		return TOK_END;
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
		return TOK_END;
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
	unsigned int data_len;

	if ((label.type & TOK_VAR) == TOK_VAR)
	{
		strcat(data, ".");
		strcat(data, label.name);
		strcat(data, ":\n\t.data ");
		strcat(data, label.val);
		if ((label.type & TOK_CSTR) == TOK_CSTR)
		{
			data_len = strlen(data);
			strcpy(data + data_len - 1, "\\x00\"");
		}
		strcat(data, "\n");
	} else if (label.type == TOK_FUNC)
	{
		strcat(code, ".");
		strcat(code, label.name);
		strcat(code, ":\n");
	}
	return 0;
}

int write_code(label_t label, char* code)
{
	if (label.type == TOK_ASM)
	{
		label.val[0] = '\t';
		label.val[strlen(label.val) - 1] = '\n';
		strcat(code, label.val);
	} else if (label.type == TOK_CALL)
	{
		strcat(code, "\tlri 0x00, @");
		strcat(code, label.name);
		strcat(code, "\n\tcll 0x00\n");
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
	status_t status;
	int excepted = TOK_ANY;
	int code_len = 0;
	unsigned int tmp_val = 0;

	lexer.top = inp;
	lexer.cur = inp;
	lexer.pos = inp;
	lexer.end = inp + strlen(inp);
	lexer.file = file;
	lexer.line = 1;

	func_any = TOK_VAR | TOK_NAME | TOK_REG | TOK_SEMICOLON | TOK_RETURN | TOK_ASM | TOK_LBR | TOK_RBR | TOK_CALL | TOK_CMP;
	status.level = 0;
	status.cases = 0;
	status.reassign = 1;

	strcpy(code, ".start:\n\tlri 0x00, @main\n\tcll 0x00\n\thlt\n\n");

	tok = lex(&lexer);
	while (tok)
	{
		if ((excepted != TOK_ANY && (excepted & tok) != tok) || tok == -1)
		{
			COMP_ERROR("Unexcepted token: " BOLD "%c (0x%02x)" RESET ", excepted 0x%02x.\n", *(lexer.cur + 1), tok, excepted);
			puts(code);
			puts(data);
			return -1;
		}
		switch (tok)
		{
			case TOK_VAR:
				excepted = TOK_NAME;
				label.type = TOK_VAR;
			break;
			case TOK_NAME:
				label.name = get_token(&lexer);
				if (label.type == TOK_FUNC)
				{
					write_label(label, code, data);
					excepted = TOK_LBR;
					status.func = 0;
					label.type = 0x00;
				} else if (label.type == TOK_CALL)
				{
					write_code(label, code);
					excepted = TOK_SEMICOLON;
					label.type = 0x00;
				} else if (label.type == TOK_REG)
				{
					label.assign = RA;
					write_code(label, code);
				} else
				{
					if (excepted == TOK_ANY || excepted == func_any)
					{
						status.reassign = 0;
					}
					excepted = TOK_ASSIGN;
				}
			break;
			case TOK_REG:
				if (label.type == TOK_REG)
				{
					label.val = get_token(&lexer);
					label.assign = RR;
					write_code(label, code);
					free(label.val);
				} else if (label.type == TOK_CMP)
				{
					if (label.val == NULL)
					{
						label.val = get_token(&lexer);
						sprintf(code + strlen(code), "\tcmp 0x%02x, ", atoi(label.val + 2));
						excepted = TOK_REG;
					} else
					{
						label.val = get_token(&lexer);
						sprintf(code + strlen(code), "0x%02x\n", atoi(label.val + 2));
						label.type = 0x00;
						//excepted = TOK_LBR;
						excepted = TOK_SEMICOLON;
					}
					free(label.val);
				} else
				{
					label.val = get_token(&lexer);
					label.type = TOK_REG;
					excepted = TOK_ASSIGN;
				}
			break;
			case TOK_ASSIGN:
				if (status.reassign == 0)
				{
					excepted = TOK_INT;
				} else if (label.type == TOK_REG)
				{
					excepted = TOK_INT | TOK_NAME | TOK_REG;
				} else
				{
					excepted = TOK_INT | TOK_CSTR;
				}
			break;
			case TOK_INT:
				label.val = get_token(&lexer);
				code_len = strlen(code);
				if (label.type == TOK_VAR)
				{
					label.type |= TOK_INT;
					write_label(label, code, data);
					free(label.name);
				} else if (status.reassign == 0)
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
					status.reassign = 1;
				} else if (excepted == (TOK_INT | TOK_SEMICOLON))
				{
					strcat(code, "\tlri 0x0e, ");
					strcat(code, label.val);
					strcat(code, "\n");
				} else if (label.type == TOK_REG)
				{
					tmp_val = strtol(label.val, NULL, 16);
					sprintf(code + strlen(code), "\tlri 0x%02x, 0x%02x\n", atoi(label.name + 2), tmp_val);
				}
				label.type = 0x00;
				excepted = TOK_SEMICOLON;
				free(label.val);
			break;
			case TOK_CSTR:
				label.val = get_token(&lexer);
				if (label.type == TOK_VAR)
				{
					label.type |= TOK_CSTR;
					write_label(label, code, data);
					free(label.name);
				} else
				{
					write_code(label, code);
				}
				label.type = 0x00;
				excepted = TOK_SEMICOLON;
				free(label.val);
			break;
			case TOK_SEMICOLON:
				if (status.func == 0)
				{
					excepted = func_any;
				} else
				{
					excepted = TOK_ANY;
				}
			break;
			case TOK_RETURN:
				excepted = TOK_INT | TOK_SEMICOLON;
			break;
			case TOK_ASM:
				label.name = NULL;
				label.type = TOK_ASM;
				excepted = TOK_CSTR;
			break;
			case TOK_FUNC:
				label.type = TOK_FUNC;
				excepted = TOK_NAME;
			break;
			case TOK_LBR:
				if (label.type == TOK_FUNC || label.type == 0x00)
				{
					excepted = TOK_ANY;
				} else
				{
					excepted = TOK_CASE;
				}
				status.level++;
			break;
			case TOK_RBR:
				excepted = TOK_ANY;
				if (status.level == 1 && status.func == 0)
				{
					strcat(code, "\tret\n");
				}
				status.level--;
			break;
			case TOK_CALL:
				excepted = TOK_NAME;
				label.type = TOK_CALL;
			break;
			case TOK_CMP:
				label.type = TOK_CMP;
				label.val = NULL;
				excepted = TOK_REG;
			break;
			/*case TOK_CASE:
			status.cases
				label.type = TOK_COND;
			break;*/
			case TOK_END:
			break;
			default:
				COMP_ERROR("Unknown token: " BOLD "0x%02x" RESET ".\n", *(lexer.cur));
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
	char* out_code;
	char* out_data;
	char* out_path;
	char* n2as_path;
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
				printf("  --no-preproc\tDo not preprocess.\n");
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
