/*
 * Single File Self-Hosting C Compiler
 *
 * 100% Handcoded
 */

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 65536
#define MAX_TOKEN 1024

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
	Token_Type_PLUS_MINUS,
	Token_Type_UNKNOWN,
} typedef Token_Type;

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
		} else if (is_num(peek_char())) {
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
				}
			} else if (ch == '>') {
				if (peek_char() == '>') {
					ch = next_char();
					*(tok_text++) = ch;
					if (peek_char() == '=') {
						ch = next_char();
						*(tok_text++) = ch;
					}
				} else if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				}
			} else if (ch == '<') {
				if (peek_char() == '<') {
					ch = next_char();
					*(tok_text++) = ch;
					if (peek_char() == '=') {
						ch = next_char();
						*(tok_text++) = ch;
					}
				} else if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				}
			} else if (ch == '&') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else if (peek_char() == '&') {
					ch = next_char();
					*(tok_text++) = ch;
				}
			} else if (ch == '|') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else if (peek_char() == '|') {
					ch = next_char();
					*(tok_text++) = ch;
				}
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
				} else tok.type = Token_Type_PLUS_MINUS;
			} else if (ch == '+') {
				if (peek_char() == '+') {
					ch = next_char();
					*(tok_text++) = ch;
				} else if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				} else tok.type = Token_Type_PLUS_MINUS;
			} else if (ch == '!') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				}
			} else if (ch == '*') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				}
			} else if (ch == '/') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				}
			} else if (ch == '%') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				}
			} else if (ch == '^') {
				if (peek_char() == '=') {
					ch = next_char();
					*(tok_text++) = ch;
				}
			} else if (ch == '~' || ch == ',' || ch == '?' || ch == ':' || ch == '.' || ch == ';') {}
			else {
				tok.type = Token_Type_UNKNOWN;
			}
		}
		*tok_text = '\0';
	} while (tok.type == Token_Type_COMMENT);
	token_buffer[0] = token_buffer[1];
	token_buffer[1] = tok;
	return token_buffer[0];
}

Token peek_token() {
	return token_buffer[1];
}

Token current_token() {
	return token_buffer[0];
}

enum {
	AST_Type_TERMINAL,
	AST_Type_ADDITIVE_EXPRESSION,
	AST_Type_MULTIPLICATIVE_EXPRESSION,
} typedef AST_Type;

struct AST_node {
	AST_Type type;
	union {
		struct {
			struct AST_node* left;
			struct AST_node* op;
			struct AST_node* right;
		} additive_expression;
		struct {
			Token value;
		} terminal;
	};
} typedef AST_node;

void error_unexpected_token(Token tok) {
	fprintf(stderr, "Parse Error: Unexpected token %s at %ld:%ld\n", tok.text, tok.line_no, tok.col_no);
	exit(1);
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
	error_unexpected_token(current_token());
	return NULL;
}

AST_node* parse_additive_expression() {
	AST_node* left = expect(Token_Type_NUMBER);
	if (peek_token().type == Token_Type_PLUS_MINUS) {
		AST_node* node = malloc(sizeof(AST_node));
		node->additive_expression.left = left;
		node->additive_expression.op = expect(Token_Type_PLUS_MINUS);
		node->additive_expression.right = parse_additive_expression();
		return node;
	}
	return left;
}

int main() {
	// fill buffers to allow for peeking
	next_char();
	next_token();
	next_token();
	/*while (!reached_eof) {
		Token tok = next_token();
		printf("Found token %s of type %d\n", tok.text, tok.type);
	}*/
	AST_node* program = parse_additive_expression();
}
