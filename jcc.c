/*
 * Single File Self-Hosting C Compiler targeting x86-64
 *
 * 100% Handcoded
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h> // used for chmod
#include <string.h>   // used for sprintf

#define BUFFER_SIZE 65536
#define MAX_TOKEN 1024

/*
 * Stage: TOKENIZER
 *
 * This stage breaks the input into tokens
 */

enum {
	Token_Type_IDENTIFIER,
	Token_Type_NUMBER,
	Token_Type_KEYWORD,
	Token_Type_STRING,
	Token_Type_CHAR,
	Token_Type_O_PAREN,
	Token_Type_C_PAREN,
	Token_Type_O_BRACK,
	Token_Type_C_BRACK,
	Token_Type_O_BRACE,
	Token_Type_C_BRACE,
	Token_Type_SYMBOL,
	Token_Type_COMMENT,

	Token_Type_MULTIPLICATIVE,
	Token_Type_ADDITIVE,
	Token_Type_SHIFT,
	Token_Type_RELATIONAL,
	Token_Type_EQUALITY,
	Token_Type_AND,
	Token_Type_EXCLUSIVE_OR,
	Token_Type_INCLUSIVE_OR,
	Token_Type_LOGICAL_AND,
	Token_Type_LOGICAL_OR,
	Token_Type_CONDITIONAL,
	Token_Type_COLON,

	Token_Type_UNKNOWN,
} typedef Token_Type;

char* Token_Type_to_str(Token_Type t) {
	if (t == Token_Type_IDENTIFIER) return "IDENTIFIER";
	else if (t == Token_Type_NUMBER) return "NUMBER";
	else if (t == Token_Type_KEYWORD) return "KEYWORD";
	else if (t == Token_Type_STRING) return "STRING";
	else if (t == Token_Type_CHAR) return "CHAR";
	else if (t == Token_Type_O_PAREN) return "OPEN PARENTHESIS";
	else if (t == Token_Type_C_PAREN) return "CLOSED PARENTHESIS";
	else if (t == Token_Type_O_BRACK) return "OPEN BRACKET";
	else if (t == Token_Type_C_BRACK) return "CLOSED BRACKET";
	else if (t == Token_Type_O_BRACE) return "OPEN BRACE";
	else if (t == Token_Type_C_BRACE) return "CLOSED BRACE";
	else if (t == Token_Type_SYMBOL) return "SYMBOL";
	else if (t == Token_Type_COMMENT) return "COMMENT";

	else if (t == Token_Type_MULTIPLICATIVE) return "MULTIPLICATIVE";
	else if (t == Token_Type_ADDITIVE) return "ADDITIVE";
	else if (t == Token_Type_SHIFT) return "SHIFT";
	else if (t == Token_Type_RELATIONAL) return "RELATIONAL";
	else if (t == Token_Type_EQUALITY) return "EQUALITY";
	else if (t == Token_Type_AND) return "AND";
	else if (t == Token_Type_EXCLUSIVE_OR) return "EXCLUSIVE_OR";
	else if (t == Token_Type_INCLUSIVE_OR) return "INCLUSIVE_OR";
	else if (t == Token_Type_LOGICAL_AND) return "LOGICAL_AND";
	else if (t == Token_Type_LOGICAL_OR) return "LOGICAL_OR";
	else if (t == Token_Type_CONDITIONAL) return "CONDITIONAL";
	else if (t == Token_Type_COLON) return "COLON";

	else return "UNKNOWN";
}

struct {
	size_t line_no;
	size_t col_no;
	char* text;
	Token_Type type;
} typedef Token;

char keywords[] = "auto break case char const continue default do double else enum extern float for goto if int long register return short signed sizeof static struct switch typedef union unsigned void volatile while NULL size_t";

size_t cur_line = 1;
size_t cur_col = 0;

size_t cur_pos = 0;

char buffer[BUFFER_SIZE];
// having a buffer size of 2 allows for peek_char() to lookahead
char char_buffer[2] = { 0 }; 
int reached_eof = 0;

Token token_buffer[2];

// just to make self-hosting easier a lot of clib adjacent functions are
// reimplemented so we dont have to define them later, obviously the *end* goal
// is to link against clib but for bootstrapping purposes that goal is pretty
// far off
int is_whitespace(char ch) {
	if (ch == ' ' || ch == '\n' || ch == '\t') return 1;
	else return 0;
}

int is_alpha(char ch) {
	if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'))
		return 1;
	else return 0;
}

int is_num(char ch) {
	if ('0' <= ch && ch <= '9')
		return 1;
	else return 0;
}

int is_alnum(char ch) {
	return is_alpha(ch) || is_num(ch);
}

char next_char() {
	// TODO: rework for non stdin input
	int int_c = getc(stdin);
	if (int_c == EOF) reached_eof = 1;
	char_buffer[0] = char_buffer[1];
	char_buffer[1] = (char)int_c;
	char ch = char_buffer[0];
	if (ch == '\n') {
		cur_line++;
		cur_col = 0;
	} else {
		cur_col++;
	}
	buffer[cur_pos] = ch;
	cur_pos++;
	return ch;
}

char peek_char() {
	return char_buffer[1];
}

char next_non_ws_char() {
	char ch = next_char();
	while (is_whitespace(ch))
		ch = next_char();
	return ch;
}

int is_keyword(char* str) {
	char* base = str;
	char* keyword_ptr = keywords;
	while (*keyword_ptr != '\0') {
		if (*str == *keyword_ptr) {
			str++;
			keyword_ptr++;
		} else if (*str == '\0' && *keyword_ptr == ' ') {
			return 1;
		} else {
			str = base;
			while (*keyword_ptr != ' ' && *keyword_ptr != '\0')
				keyword_ptr++;
			keyword_ptr++;
		}
	}
	if (*str == '\0') return 1;
	return 0;
}

// bootleg strcmp
int str_eql(const char* s1, const char* s2) {
	const char* p1 = s1;
	const char* p2 = s2;
	int eql = 0;
	while (eql == 0 && *p1 != '\0' && *p2 != '\0') {
		if (*p1 == *p2) {
			p1++;
			p2++;
		} else eql = 1;
	}
	if (*p1 == *p2) return 0;
	return 1;
}

// main tokenizer function
Token next_token() {
	Token tok;
	do {
		tok.line_no = cur_line;
		tok.col_no = cur_col;
		char* tok_text = malloc(sizeof(char) * MAX_TOKEN);
		tok.text = tok_text;
		char ch = next_non_ws_char();
		*(tok_text++) = ch;
		if (is_alpha(ch)) {
			// identifier or keyword
			while (is_alnum(peek_char()) || peek_char() == '_') {
				ch = next_char();
				*(tok_text++) = ch;
			}
			*tok_text = '\0';
			if (is_keyword(tok.text)) tok.type = Token_Type_KEYWORD;
			else tok.type = Token_Type_IDENTIFIER;
		} else if (ch == '"') {
			// string
			int escaped = 0;
			while (peek_char() != '"' || escaped) {
				ch = next_char();
				*(tok_text++) = ch;
				if (ch == '\\' && !escaped) {
					escaped = 1;
				} else if (escaped == 1) {
					escaped = 0;
				}
			}
			ch = next_char();
			*(tok_text++) = ch;
			tok.type = Token_Type_STRING;
		} else if (ch == '\'') {
			// char
			int escaped = 0;
			while (peek_char() != '\'' || escaped) {
				ch = next_char();
				*(tok_text++) = ch;
				if (ch == '\\' && !escaped) {
					escaped = 1;
				} else if (escaped == 1) {
					escaped = 0;
				}
			}
			ch = next_char();
			*(tok_text++) = ch;
			tok.type = Token_Type_CHAR;
		} else if (is_num(ch)) {
			// number
			while (is_num(peek_char())) {
				ch = next_char();
				*(tok_text++) = ch;
			}
			tok.type = Token_Type_NUMBER;
		} else if (ch == '(') {
			tok.type = Token_Type_O_PAREN;
		} else if (ch == ')') {
			tok.type = Token_Type_C_PAREN;
		} else if (ch == '[') {
			tok.type = Token_Type_O_BRACK;
		} else if (ch == ']') {
			tok.type = Token_Type_C_BRACK;
		} else if (ch == '{') {
			tok.type = Token_Type_O_BRACE;
		} else if (ch == '}') {
			tok.type = Token_Type_C_BRACE;
		} else if (ch == '/' && peek_char() == '/') {
			// comment
			while (peek_char() != '\n') {
				ch = next_char();
				*(tok_text++) = ch;
			}
			tok.type = Token_Type_COMMENT;
		} else if (ch == '/' && peek_char() == '*') {
			// multiline comment
			ch = next_char();
			*(tok_text++) = ch;
			while (1) {
				ch = next_char();
				*(tok_text++) = ch;
				if (ch == '*' && peek_char() == '/') break;
			}
			ch = next_char();
			*(tok_text++) = ch;
			tok.type = Token_Type_COMMENT;
		} else {
			// symbol logic
			tok.type = Token_Type_SYMBOL;
			if (ch == '=') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
					tok.type = Token_Type_EQUALITY;
				}
			} else if (ch == '>') {
				if (peek_char() == '>') {
					ch = next_char();
					*(tok_text++) = ch;
					if (peek_char() == '=') {
						ch = next_char();
						*(tok_text++) = ch;
					} else tok.type = Token_Type_SHIFT;
				} else if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
					tok.type = Token_Type_RELATIONAL;
				} else tok.type = Token_Type_RELATIONAL;
			} else if (ch == '<') {
				if (peek_char() == '<') {
					ch = next_char();
					*(tok_text++) = ch;
					if (peek_char() == '=') {
						ch = next_char();
						*(tok_text++) = ch;
					} else tok.type = Token_Type_SHIFT;
				} else if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
					tok.type = Token_Type_RELATIONAL;
				} else tok.type = Token_Type_RELATIONAL;
			} else if (ch == '&') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else if (peek_char() == '&') {
					ch = next_char();
					*(tok_text++) = ch;
					tok.type = Token_Type_LOGICAL_AND;
				} else tok.type = Token_Type_AND;
			} else if (ch == '|') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else if (peek_char() == '|') {
					ch = next_char();
					*(tok_text++) = ch;
					tok.type = Token_Type_LOGICAL_OR;
				} else tok.type = Token_Type_INCLUSIVE_OR;
			} else if (ch == '-') {
				if (peek_char() == '-') {
					ch = next_char();
					*(tok_text++) = ch;
				} else if (peek_char() == '>') {
					ch = next_char();
					*(tok_text++) = ch;
				} else if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else tok.type = Token_Type_ADDITIVE;
			} else if (ch == '+') {
				if (peek_char() == '+') {
					ch = next_char();
					*(tok_text++) = ch;
				} else if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else tok.type = Token_Type_ADDITIVE;
			} else if (ch == '!') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
					tok.type = Token_Type_EQUALITY;
				}
			} else if (ch == '*') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else tok.type = Token_Type_MULTIPLICATIVE;
			} else if (ch == '/') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else tok.type = Token_Type_MULTIPLICATIVE;
			} else if (ch == '%') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else tok.type = Token_Type_MULTIPLICATIVE;
			} else if (ch == '^') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else tok.type = Token_Type_EXCLUSIVE_OR;
			} else if (ch == '?') {
				*(tok_text++) = ch;
				tok.type = Token_Type_CONDITIONAL;
			} else if (ch == ':') {
				*(tok_text++) = ch;
				tok.type = Token_Type_COLON;
			} else if (ch == '~' || ch == ',' || ch == '.' || ch == ';') {}
			else {
				tok.type = Token_Type_UNKNOWN;
			}
		}
		*tok_text = '\0';
	} while (tok.type == Token_Type_COMMENT);
	token_buffer[0] = token_buffer[1];
	token_buffer[1] = tok;
	// printf("Got token %s\n", tok.text);
	return token_buffer[0];
}

Token peek_token() {
	return token_buffer[1];
}

Token current_token() {
	return token_buffer[0];
}

/*
 * Stage: PARSER
 *
 * This stage takes all the tokens and breaks them down into an AST
 */

enum {
	AST_Type_TERMINAL,
	AST_Type_MULTIPLICATIVE_EXPRESSION,
	AST_Type_ADDITIVE_EXPRESSION,
	AST_Type_SHIFT_EXPRESSION,
	AST_Type_RELATIONAL_EXPRESSION,
	AST_Type_EQUALITY_EXPRESSION,
	AST_Type_AND_EXPRESSION,
	AST_Type_EXCLUSIVE_OR_EXPRESSION,
	AST_Type_INCLUSIVE_OR_EXPRESSION,
	AST_Type_LOGICAL_AND_EXPRESSION,
	AST_Type_LOGICAL_OR_EXPRESSION,
	AST_Type_CONDITIONAL_EXPRESSION,
} typedef AST_Type;

struct AST_node {
	AST_Type type;
	union {
		// TODO: condense binary ops into union
		struct {
			struct AST_node* left;
			struct AST_node* op;
			struct AST_node* right;
		} binary_expression;
		struct {
			struct AST_node* condition;
			struct AST_node* left;
			struct AST_node* right;
		} conditional_expression;
		struct {
			Token value;
		} terminal;
	};
} typedef AST_node;

// some general purpose helpers
void error_unexpected_token(Token tok, Token_Type t) {
	fprintf(stderr, "Parse Error: Unexpected token %s at %ld:%ld\nExpected %s but found %s (%s)\n",
			tok.text, tok.line_no, tok.col_no, Token_Type_to_str(t), tok.text, Token_Type_to_str(tok.type));
	exit(1);
}

void error_internal(const char* msg) {
	fprintf(stderr, "Fatal internal error: %s\n", msg);
	exit(1);
}

int AST_is_binary(AST_Type type) {
	if (type == AST_Type_MULTIPLICATIVE_EXPRESSION) return 1;
	else if (type == AST_Type_ADDITIVE_EXPRESSION) return 1;
	else if (type == AST_Type_SHIFT_EXPRESSION) return 1;
	else if (type == AST_Type_RELATIONAL_EXPRESSION) return 1;
	else if (type == AST_Type_EQUALITY_EXPRESSION) return 1;
	else if (type == AST_Type_AND_EXPRESSION) return 1;
	else if (type == AST_Type_EXCLUSIVE_OR_EXPRESSION) return 1;
	else if (type == AST_Type_INCLUSIVE_OR_EXPRESSION) return 1;
	else if (type == AST_Type_LOGICAL_AND_EXPRESSION) return 1;
	else if (type == AST_Type_LOGICAL_OR_EXPRESSION) return 1;
	return 0;
}

char* AST_Type_to_str(AST_Type t) {
	if (t == AST_Type_TERMINAL) return "TERMINAL";
	else if (t == AST_Type_MULTIPLICATIVE_EXPRESSION) return "MULTIPLICATIVE_EXPRESSION";
	else if (t == AST_Type_ADDITIVE_EXPRESSION) return "ADDITIVE_EXPRESSION";
	else if (t == AST_Type_SHIFT_EXPRESSION) return "SHIFT_EXPRESSION";
	else if (t == AST_Type_RELATIONAL_EXPRESSION) return "RELATIONAL_EXPRESSION";
	else if (t == AST_Type_EQUALITY_EXPRESSION) return "EQUALITY_EXPRESSION";
	else if (t == AST_Type_AND_EXPRESSION) return "AND_EXPRESSION";
	else if (t == AST_Type_EXCLUSIVE_OR_EXPRESSION) return "EXCLUSIVE_OR_EXPRESSION";
	else if (t == AST_Type_INCLUSIVE_OR_EXPRESSION) return "INCLUSIVE_OR_EXPRESSION";
	else if (t == AST_Type_LOGICAL_AND_EXPRESSION) return "LOGICAL_AND_EXPRESSION";
	else if (t == AST_Type_LOGICAL_OR_EXPRESSION) return "LOGICAL_OR_EXPRESSION";
	else if (t == AST_Type_CONDITIONAL_EXPRESSION) return "CONDITIONAL_EXPRESSION";

	else return "UNKNOWN";
}

// return an AST_node terminal of current token if it is type t
AST_node* accept(Token_Type t) {
	if (t == current_token().type) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_TERMINAL;
		node->terminal.value = current_token();
		next_token();
		return node;
	}
	return NULL;
}

// return an AST_node terminal of current token if it is type t and crash if not
AST_node* expect(Token_Type t) {
	AST_node* node = accept(t);
	if (node) return node;
	error_unexpected_token(current_token(), t);
	return NULL;
}

AST_node* parse_conditional_expression();

// recursive descent parsing rules:
AST_node* parse_primary_expression() {
	AST_node* left;
	if (current_token().type == Token_Type_O_PAREN) {
		expect(Token_Type_O_PAREN);
		left = parse_conditional_expression(); // TODO: actually parse expression
		expect(Token_Type_C_PAREN);
	} else if (current_token().type == Token_Type_NUMBER) { // TODO: parse general constants and identifiers
		left = expect(Token_Type_NUMBER);
	}
	return left;
}

AST_node* parse_multiplicative_expression() {
	AST_node* left = parse_primary_expression();
	if (current_token().type == Token_Type_MULTIPLICATIVE) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_MULTIPLICATIVE_EXPRESSION;
		node->binary_expression.left = left;
		node->binary_expression.op = expect(Token_Type_MULTIPLICATIVE);
		node->binary_expression.right = parse_multiplicative_expression();
		return node;
	}
	return left;
}

AST_node* parse_additive_expression() {
	AST_node* left = parse_multiplicative_expression();
	if (current_token().type == Token_Type_ADDITIVE) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_ADDITIVE_EXPRESSION;
		node->binary_expression.left = left;
		node->binary_expression.op = expect(Token_Type_ADDITIVE);
		node->binary_expression.right = parse_additive_expression();
		return node;
	}
	return left;
}

AST_node* parse_shift_expression() {
	AST_node* left = parse_additive_expression();
	if (current_token().type == Token_Type_SHIFT) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_SHIFT_EXPRESSION;
		node->binary_expression.left = left;
		node->binary_expression.op = expect(Token_Type_SHIFT);
		node->binary_expression.right = parse_shift_expression();
		return node;
	}
	return left;
}

AST_node* parse_relational_expression() {
	AST_node* left = parse_shift_expression();
	if (current_token().type == Token_Type_RELATIONAL) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_RELATIONAL_EXPRESSION;
		node->binary_expression.left = left;
		node->binary_expression.op = expect(Token_Type_RELATIONAL);
		node->binary_expression.right = parse_relational_expression();
		return node;
	}
	return left;
}

AST_node* parse_equality_expression() {
	AST_node* left = parse_relational_expression();
	if (current_token().type == Token_Type_EQUALITY) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_EQUALITY_EXPRESSION;
		node->binary_expression.left = left;
		node->binary_expression.op = expect(Token_Type_EQUALITY);
		node->binary_expression.right = parse_equality_expression();
		return node;
	}
	return left;
}

AST_node* parse_and_expression() {
	AST_node* left = parse_equality_expression();
	if (current_token().type == Token_Type_AND) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_AND_EXPRESSION;
		node->binary_expression.left = left;
		node->binary_expression.op = expect(Token_Type_AND);
		node->binary_expression.right = parse_and_expression();
		return node;
	}
	return left;
}

AST_node* parse_exclusive_or_expression() {
	AST_node* left = parse_and_expression();
	if (current_token().type == Token_Type_EXCLUSIVE_OR) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_EXCLUSIVE_OR_EXPRESSION;
		node->binary_expression.left = left;
		node->binary_expression.op = expect(Token_Type_EXCLUSIVE_OR);
		node->binary_expression.right = parse_exclusive_or_expression();
		return node;
	}
	return left;
}

AST_node* parse_inclusive_or_expression() {
	AST_node* left = parse_exclusive_or_expression();
	if (current_token().type == Token_Type_INCLUSIVE_OR) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_INCLUSIVE_OR_EXPRESSION;
		node->binary_expression.left = left;
		node->binary_expression.op = expect(Token_Type_INCLUSIVE_OR);
		node->binary_expression.right = parse_inclusive_or_expression();
		return node;
	}
	return left;
}

AST_node* parse_logical_and_expression() {
	AST_node* left = parse_inclusive_or_expression();
	if (current_token().type == Token_Type_LOGICAL_AND) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_LOGICAL_AND_EXPRESSION;
		node->binary_expression.left = left;
		node->binary_expression.op = expect(Token_Type_LOGICAL_AND);
		node->binary_expression.right = parse_logical_and_expression();
		return node;
	}
	return left;
}

AST_node* parse_logical_or_expression() {
	AST_node* left = parse_logical_and_expression();
	if (current_token().type == Token_Type_LOGICAL_OR) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_LOGICAL_OR_EXPRESSION;
		node->binary_expression.left = left;
		node->binary_expression.op = expect(Token_Type_LOGICAL_OR);
		node->binary_expression.right = parse_logical_and_expression();
		return node;
	}
	return left;
}

AST_node* parse_conditional_expression() {
	AST_node* left = parse_logical_or_expression();
	if (current_token().type == Token_Type_CONDITIONAL) {
		AST_node* node = malloc(sizeof(AST_node));
		node->type = AST_Type_CONDITIONAL_EXPRESSION;
		node->conditional_expression.condition = left;
		node->conditional_expression.left = parse_logical_and_expression(); // TODO: this should be expression
		node->conditional_expression.right = parse_logical_and_expression();
		return node;
	}
	return left;
}

// for debugging purposes
void print_ast(AST_node* node, int depth) {
	for (int i = 0; i < depth; i++)
		printf("    ");
	if (node->type == AST_Type_TERMINAL) {
		printf("Token %s of type %s\n", node->terminal.value.text, Token_Type_to_str(node->terminal.value.type));
	} else if (AST_is_binary(node->type)) {
		printf("Node: %s\n", AST_Type_to_str(node->type));
		print_ast(node->binary_expression.left, depth + 1);
		print_ast(node->binary_expression.op, depth + 1);
		print_ast(node->binary_expression.right, depth + 1);
	} else if (node->type == AST_Type_CONDITIONAL_EXPRESSION) {
		printf("Node: CONDITIONAL_EXPRESSION\n");
		print_ast(node->conditional_expression.condition, depth + 1);
		print_ast(node->conditional_expression.left, depth + 1);
		print_ast(node->conditional_expression.right, depth + 1);
	}
}

/*
 * Stage: CODEGEN
 *
 * We now take the information from the AST and use it to generate runnable x86_64
 */

/*
 * Before we can really do anything we need to generate ELF boilerplate so the
 * OS knows our file is intended to be an executable
 */

FILE* out_file;
uint64_t current_byte = 0;

// append a byte to output file
void emit_byte(unsigned char byte) {
	putc(byte, out_file);
	current_byte++;
}

// set byte at address in file
void set_byte(uint64_t addr, unsigned char byte) {
	fseek(out_file, (long)addr, SEEK_SET);
	putc(byte, out_file);
	fseek(out_file, 0, SEEK_END);
}

// set an i32 at address in file
void set_i32(uint64_t addr, int32_t data) {
	fseek(out_file, (long)addr, SEEK_SET);
	putc(data & 0xff, out_file);
	putc((data >> 8) & 0xff, out_file);
	putc((data >> 16) & 0xff, out_file);
	putc((data >> 24) & 0xff, out_file);
	fseek(out_file, 0, SEEK_END);
}

// set an i64 at address in file
void set_i64(uint64_t addr, int64_t data) {
	fseek(out_file, (long)addr, SEEK_SET);
	putc(data & 0xff, out_file);
	putc((data >> 8) & 0xff, out_file);
	putc((data >> 16) & 0xff, out_file);
	putc((data >> 24) & 0xff, out_file);
	putc((data >> 32) & 0xff, out_file);
	putc((data >> 40) & 0xff, out_file);
	putc((data >> 48) & 0xff, out_file);
	putc((data >> 56) & 0xff, out_file);
	fseek(out_file, 0, SEEK_END);
}

typedef struct {
	uint8_t  e_ident[16];
	uint16_t e_type;	
	uint16_t e_machine;
	uint32_t e_version;
	uint64_t e_entry;
	uint64_t e_phoff;
	uint64_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} Elf64_Header;

typedef struct {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
} Elf64_Program_Header;

void write_elf_header() {
	uint64_t start_address = 0x8000000;

	Elf64_Header header;
	for (int i = 0; i < 16; i++)
		header.e_ident[i] = 0x00;
	header.e_ident[0] = 0x7f;
	header.e_ident[1] = 'E';
	header.e_ident[2] = 'L';
	header.e_ident[3] = 'F';
	header.e_ident[4] = 0x02;
	header.e_ident[5] = 0x01;
	header.e_ident[6] = 0x01;
	
	header.e_type      = 0x02; // dynamic executable file
	header.e_machine   = 0x3e; // x86-64
	header.e_version   = 0x01; // (current)
	header.e_phoff     = 0x40; // program header starts after elf header
	header.e_shoff     = 0x00; // TODO
	header.e_flags     = 0x00; // no fucking idea
	header.e_ehsize    = 0x40; // elf header size
	header.e_phentsize = 0x38; // program header size
	header.e_phnum     = 0x01; // # of program headers
	header.e_shentsize = 0x01; // section header size
	header.e_shnum     = 0x04; // # of section headers
	header.e_shstrndx  = 0x03; // index of names section in table

	header.e_entry     = start_address + header.e_phoff + 0x40; // (align to 0x40 instead of 0x38)

	int header_bytes = sizeof(Elf64_Header);
	for (int i = 0; i < header_bytes; i++)
		emit_byte(((unsigned char*)&header)[i]);

	// redundant but ensure proper padding for offsets
	while (current_byte < header.e_phoff)
		emit_byte(0x00);

	Elf64_Program_Header pheader;
	
	pheader.p_type     = 0x01; // load segment to memory
	pheader.p_offset   = 0x00; // no offset
	pheader.p_vaddr    = start_address;
	pheader.p_paddr    = start_address;
	pheader.p_filesz   = 0xA0; // no idea
	pheader.p_memsz    = 0xA0; // no idea
	pheader.p_flags    = 0x05; // r + x
	pheader.p_align    = 0x200000; // also no idea

	int pheader_bytes = sizeof(Elf64_Program_Header);
	for (int i = 0; i < pheader_bytes; i++)
		emit_byte(((unsigned char*)&pheader)[i]);

	while (current_byte < header.e_entry - start_address)
		emit_byte(0x00);
}

/*
 * Keeping track of labels is important, for forward referencing labels we cant
 * emit linearly with a one-pass system, to solve this we can make and mark
 * labels then resolve them after emission
 */

int label_count = 0;
typedef struct {
	int id;
	uint64_t offset;
} Label;

Label labels[1024]; // surely we wont ever need more than this right?? right???
int resolution_count = 0; // resolution count may not equal label count as we
						  // can have two jumps pointing to the same label
uint64_t resolution_addresses[1024];
int resolution_ids[1024];

Label make_label() {
	Label l;
	l.id = label_count;
	l.offset = 0xffffffff; // unknown absolute addr
	labels[label_count] = l;
	label_count++;
	return l;
}

void mark_label(Label* label) {
	label->offset = current_byte;
	labels[label->id].offset = current_byte;
}

void resolve_labels() {
	uint64_t save_byte;
	for (int i = 0; i < resolution_count; i++) {
		set_i32(resolution_addresses[i], labels[resolution_ids[i]].offset - (resolution_addresses[i] + 4));
	}
}

/*
 * Awesome, now we can worry about the actual x86_64 codegen
 */

// for now we will only concern ourselves with 64bit registers
enum {
	Register_RAX, // = 0
	Register_RCX,
	Register_RDX,
	Register_RBX, // preserved
	Register_RSP, // preserved
	Register_RBP, // preserved
	Register_RSI,
	Register_RDI,

	// 64bit registers
	Register_R8,
	Register_R9,
	Register_R10,
	Register_R11,
	Register_R12, // preserved
	Register_R13, // preserved
	Register_R14, // preserved
	Register_R15, // preserved
} typedef Codegen_Register;

// helperes for dealing with register enums:
int reg_is_64bit(Codegen_Register reg) {
	return reg < Register_R8 ? 0 : 1;
}

// get the canonical component of registers, for example r8 -> rax
int reg_canon(Codegen_Register reg) {
	return reg % Register_R8;
}

/*
 * x86_64 Assembly Helpers:
 */

void emit_mov_reg_imm(Codegen_Register reg, int64_t val) {
	// mov [reg] (32bit -- TODO: typechecking)
	if (reg_is_64bit(reg)) {
		emit_byte(0x49);
	} else {
		emit_byte(0x48);
	}
	emit_byte(0xc7);
	emit_byte(0xc0 + reg_canon(reg));
	// little endian
	emit_byte(val & 0xff);
	emit_byte((val >> 8) & 0xff);
	emit_byte((val >> 16) & 0xff);
	emit_byte((val >> 24) & 0xff);
}

void emit_push_reg(Codegen_Register reg) {
	if (reg_is_64bit(reg)) {
		emit_byte(0x41);
	}
	emit_byte(0x50 + reg_canon(reg));
}

void emit_pop_reg(Codegen_Register reg) {
	if (reg_is_64bit(reg)) {
		emit_byte(0x41);
	}
	emit_byte(0x58 + reg_canon(reg));
}

void emit_syscall() {
	emit_byte(0x0f); emit_byte(0x05);
}

void emit_add_reg_reg(Codegen_Register reg1, Codegen_Register reg2) {
	if (reg_is_64bit(reg2)) {
		emit_byte(0x4c + reg_is_64bit(reg1));
	} else {
		emit_byte(0x48 + reg_is_64bit(reg1));
	}
	emit_byte(0x01);
	emit_byte(0xc0 + (reg_canon(reg2) << 3) + reg_canon(reg1));
}

void emit_sub_reg_reg(Codegen_Register reg1, Codegen_Register reg2) {
	if (reg_is_64bit(reg2)) {
		emit_byte(0x4c + reg_is_64bit(reg1));
	} else {
		emit_byte(0x48 + reg_is_64bit(reg1));
	}
	emit_byte(0x29);
	emit_byte(0xc0 + (reg_canon(reg2) >> 3) + reg_canon(reg1));
}

void emit_xchg_reg_reg(Codegen_Register reg1, Codegen_Register reg2) {
	if (reg1 == reg2) return;
	// order doesnt matter for xchg
	if (reg1 > reg2) {
		Codegen_Register tmp = reg1;
		reg1 = reg2;
		reg2 = tmp;
	}
	// xchg rax has special encoding
	if (reg1 == Register_RAX) {
		emit_byte(0x48 + reg_is_64bit(reg1));
		emit_byte(0x90 + reg_canon(reg2));
	} else {
		emit_byte(0x48 + reg_is_64bit(reg1) + (reg_is_64bit(reg2) >> 3));
		emit_byte(0x87);
		emit_byte(0xc0 + (reg_canon(reg2) >> 3) + reg_canon(reg1));
	}
}

void emit_cqo() {
	emit_byte(0x48);
	emit_byte(0x99);
}

void emit_label(Label target) {
	// 32bit relative addr, resolve later
	resolution_addresses[resolution_count] = current_byte;
	resolution_ids[resolution_count] = target.id;
	resolution_count++;
	// unknown addr for now
	emit_byte(0x00); emit_byte(0x00); emit_byte(0x00); emit_byte(0x00);
}

void emit_jz(Label target) {
	emit_byte(0x0f);
	emit_byte(0x84);
	emit_label(target);
}

void emit_jnz(Label target) {
	emit_byte(0x0f);
	emit_byte(0x85);
	emit_label(target);
}

void emit_jmp(Label target) {
	emit_byte(0xe9);
	emit_label(target);
}

int short_circuits(const char* op) {
	return !str_eql(op, "&&") || !str_eql(op, "||");
}

void generate_code(AST_node* node) {
	// simple approach: move terminals into rax
	if (node->type == AST_Type_TERMINAL) {
		// TODO: typechecking
		int64_t val = atoi(node->terminal.value.text);
		emit_mov_reg_imm(Register_RAX, val);
		return;
	} else if (AST_is_binary(node->type)) {
		// generate code for left into rax, push, generate code for right into rax,
		// pop into rcx and operate
		AST_node* left  = node->binary_expression.left;
		AST_node* op    = node->binary_expression.op;
		AST_node* right = node->binary_expression.right;
		char* op_text = op->terminal.value.text;

		// we can only do l/r codegen for non short-circuiting ops
		if (!short_circuits(op_text)) {
			generate_code(left);
			emit_push_reg(Register_RAX); // TODO: very temporary solution, ideally
										 // we want some type of register
										 // allocation but that can come after an
										 // ir

			generate_code(right);
			emit_pop_reg(Register_RCX);
		}
		
		if (str_eql(op_text, "+") == 0) {
			emit_add_reg_reg(Register_RAX, Register_RCX);
		} else if (str_eql(op_text, "-") == 0) {
			emit_sub_reg_reg(Register_RCX, Register_RAX);
			emit_xchg_reg_reg(Register_RCX, Register_RAX);
		} else if (str_eql(op_text, "*") == 0) {
			// imul rax, rcx
			emit_byte(0x48); emit_byte(0x0f); emit_byte(0xaf); emit_byte(0xc1);
		} else if (str_eql(op_text, "/") == 0) {
			emit_xchg_reg_reg(Register_RCX, Register_RAX);
			emit_cqo();
			// idiv rcx
			emit_byte(0x48); emit_byte(0xf7); emit_byte(0xf9);
		} else if (str_eql(op_text, "%") == 0) {
			emit_xchg_reg_reg(Register_RCX, Register_RAX);
			emit_cqo();
			// idiv rcx
			emit_byte(0x48); emit_byte(0xf7); emit_byte(0xf9);
			// mod is stored in rdx, move to rax
			emit_xchg_reg_reg(Register_RAX, Register_RDX);
		} else if (str_eql(op_text, "<<") == 0) {
			emit_xchg_reg_reg(Register_RAX, Register_RCX);
			// shl rax, cl
			emit_byte(0x48); emit_byte(0xd3); emit_byte(0xe0);
		} else if (str_eql(op_text, ">>") == 0) {
			emit_xchg_reg_reg(Register_RAX, Register_RCX);
			// shr rax, cl
			emit_byte(0x48); emit_byte(0xd3); emit_byte(0xe8);
		} else if (str_eql(op_text, "<") == 0) {
			// cmp rax, rcx
			emit_byte(0x48); emit_byte(0x39); emit_byte(0xc1);
			// setl al
			emit_byte(0x0f); emit_byte(0x9c); emit_byte(0xc0);
			// and al, 1
			emit_byte(0x24); emit_byte(0x01);
		} else if (str_eql(op_text, "<=") == 0) {
			// cmp rax, rcx
			emit_byte(0x48); emit_byte(0x39); emit_byte(0xc1);
			// setle al
			emit_byte(0x0f); emit_byte(0x9e); emit_byte(0xc0);
			// and al, 1
			emit_byte(0x24); emit_byte(0x01);
		} else if (str_eql(op_text, ">") == 0) {
			// cmp rax, rcx
			emit_byte(0x48); emit_byte(0x39); emit_byte(0xc1);
			// setg al
			emit_byte(0x0f); emit_byte(0x9f); emit_byte(0xc0);
			// and al, 1
			emit_byte(0x24); emit_byte(0x01);
		} else if (str_eql(op_text, ">=") == 0) {
			// cmp rax, rcx
			emit_byte(0x48); emit_byte(0x39); emit_byte(0xc1);
			// setge al
			emit_byte(0x0f); emit_byte(0x9d); emit_byte(0xc0);
			// and al, 1
			emit_byte(0x24); emit_byte(0x01);
		} else if (str_eql(op_text, "==") == 0) {
			// cmp rax, rcx
			emit_byte(0x48); emit_byte(0x39); emit_byte(0xc1);
			// sete al
			emit_byte(0x0f); emit_byte(0x94); emit_byte(0xc0);
			// and al, 1
			emit_byte(0x24); emit_byte(0x01);
		} else if (str_eql(op_text, "!=") == 0) {
			// cmp rax, rcx
			emit_byte(0x48); emit_byte(0x39); emit_byte(0xc1);
			// setne al
			emit_byte(0x0f); emit_byte(0x95); emit_byte(0xc0);
			// and al, 1
			emit_byte(0x24); emit_byte(0x01);
		} else if (str_eql(op_text, "&") == 0) {
			// and rax, rcx
			emit_byte(0x48); emit_byte(0x21); emit_byte(0xc8);
		} else if (str_eql(op_text, "^") == 0) {
			// xor rax, rcx
			emit_byte(0x48); emit_byte(0x31); emit_byte(0xc8);
		} else if (str_eql(op_text, "|") == 0) {
			// or rax, rcx
			emit_byte(0x48); emit_byte(0x09); emit_byte(0xc8);
		} else if (str_eql(op_text, "&&") == 0) {
			Label false_label = make_label();
			Label end_label = make_label();

			generate_code(left);
			// test rax, rax
			emit_byte(0x48); emit_byte(0x85); emit_byte(0xc0);
			// jz false_label
			emit_jz(false_label);

			generate_code(right);
			// and al, 1
			emit_byte(0x24); emit_byte(0x01);
			emit_jmp(end_label);

			mark_label(&false_label);
			// xor rax, rax
			emit_byte(0x48); emit_byte(0x31); emit_byte(0xc0);

			mark_label(&end_label);
		} else if (str_eql(op_text, "||") == 0) {
			Label true_label = make_label();
			Label end_label = make_label();

			generate_code(left);
			// test rax, rax
			emit_byte(0x48); emit_byte(0x85); emit_byte(0xc0);
			// jnz true_label
			emit_jnz(true_label);

			generate_code(right);
			// and al, 1
			emit_byte(0x24); emit_byte(0x01);
			emit_jmp(end_label);

			mark_label(&true_label);
			// mov al, 1
			emit_byte(0xb0); emit_byte(0x01);

			mark_label(&end_label);
		} else {
			char buf[128] = "Operator %d not implemented yet :P";
			snprintf(buf, sizeof(buf), buf, op_text);
			error_internal(buf);
		}
	}
}

void emit_exit() {
	// mov rdi, rax (mov result to exit val)
	emit_byte(0x48); emit_byte(0x87); emit_byte(0xc7);
	// mov rax, 60
	emit_mov_reg_imm(Register_RAX, 60);
	// syscall
	emit_syscall();
}

/*
 * Stage: MAIN DRIVER
 *
 * Now we can take all of these components and put them together in a way that
 * makes sense!
 */

int main() {
	printf("== JCC ==\n");
	printf("Usage: ./jcc < [input]\n");
	// fill buffers to allow for peeking
	next_char();
	next_token();
	next_token();

	// parse program
	AST_node* program = parse_conditional_expression();
	print_ast(program, 0);

	// prepare output
	out_file = fopen("jcc.out", "wb");
	write_elf_header();

	// codegen
	generate_code(program);
	emit_exit();
	resolve_labels();

	// cleanup and allow execution
	fclose(out_file);
	chmod("jcc.out", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); // +X
}
