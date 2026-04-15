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
	Token_Type_INT_CONST,
	Token_Type_FLOAT_CONST,
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

	Token_Type_AUTO,
	Token_Type_BREAK,
	Token_Type_CASE,
	Token_Type_CONST,
	Token_Type_CONTINUE,
	Token_Type_DEFAULT,
	Token_Type_DO,
	Token_Type_DOUBLE,
	Token_Type_ELSE,
	Token_Type_ENUM,
	Token_Type_EXTERN,
	Token_Type_FLOAT,
	Token_Type_FOR,
	Token_Type_GOTO,
	Token_Type_IF,
	Token_Type_INT,
	Token_Type_LONG,
	Token_Type_REGISTER,
	Token_Type_RETURN,
	Token_Type_SHORT,
	Token_Type_SIGNED,
	Token_Type_SIZEOF,
	Token_Type_STATIC,
	Token_Type_STRUCT,
	Token_Type_SWITCH,
	Token_Type_TYPEDEF,
	Token_Type_UNION,
	Token_Type_UNSIGNED,
	Token_Type_VOID,
	Token_Type_VOLATILE,
	Token_Type_WHILE,
	Token_Type_NULL,
	Token_Type_SIZE_T,

	Token_Type_UNKNOWN,
} typedef Token_Type;

char* Token_Type_to_str(Token_Type t) {
	if (t == Token_Type_IDENTIFIER) return "IDENTIFIER";
	else if (t == Token_Type_INT_CONST) return "INT_CONST";
	else if (t == Token_Type_FLOAT_CONST) return "FLOAT_CONST";
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

	else if (t == Token_Type_AUTO) return "AUTO";
	else if (t == Token_Type_BREAK) return "BREAK";
	else if (t == Token_Type_CASE) return "CASE";
	else if (t == Token_Type_CHAR) return "CHAR";
	else if (t == Token_Type_CONST) return "CONST";
	else if (t == Token_Type_CONTINUE) return "CONTINUE";
	else if (t == Token_Type_DEFAULT) return "DEFAULT";
	else if (t == Token_Type_DO) return "DO";
	else if (t == Token_Type_DOUBLE) return "DOUBLE";
	else if (t == Token_Type_ELSE) return "ELSE";
	else if (t == Token_Type_ENUM) return "ENUM";
	else if (t == Token_Type_EXTERN) return "EXTERN";
	else if (t == Token_Type_FLOAT) return "FLOAT";
	else if (t == Token_Type_FOR) return "FOR";
	else if (t == Token_Type_GOTO) return "GOTO";
	else if (t == Token_Type_IF) return "IF";
	else if (t == Token_Type_INT) return "INT";
	else if (t == Token_Type_LONG) return "LONG";
	else if (t == Token_Type_REGISTER) return "REGISTER";
	else if (t == Token_Type_RETURN) return "RETURN";
	else if (t == Token_Type_SHORT) return "SHORT";
	else if (t == Token_Type_SIGNED) return "SIGNED";
	else if (t == Token_Type_SIZEOF) return "SIZEOF";
	else if (t == Token_Type_STATIC) return "STATIC";
	else if (t == Token_Type_STRUCT) return "STRUCT";
	else if (t == Token_Type_SWITCH) return "SWITCH";
	else if (t == Token_Type_TYPEDEF) return "TYPEDEF";
	else if (t == Token_Type_UNION) return "UNION";
	else if (t == Token_Type_UNSIGNED) return "UNSIGNED";
	else if (t == Token_Type_VOID) return "VOID";
	else if (t == Token_Type_VOLATILE) return "VOLATILE";
	else if (t == Token_Type_WHILE) return "WHILE";
	else if (t == Token_Type_NULL) return "NULL";
	else if (t == Token_Type_SIZE_T) return "SIZE_T";

	else return "UNKNOWN";
}

struct {
	size_t line_no;
	size_t col_no;
	char* text;
	Token_Type type;
} typedef Token;

// error handling
void error_unexpected_token(Token tok, Token_Type t) {
	fprintf(stderr, "Parse Error: Unexpected token %s at %ld:%ld\nExpected %s but found %s (%s)\n",
			tok.text, tok.line_no, tok.col_no, Token_Type_to_str(t), tok.text, Token_Type_to_str(tok.type));
	exit(1);
}

void error_internal(const char* msg) {
	fprintf(stderr, "Fatal internal error: %s\n", msg);
	exit(1);
}

void error_invalid_type(const char* msg) {
	fprintf(stderr, "Type Error: %s\n", msg);
	exit(1);
}

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
			if (str_eql(tok.text, "auto") == 0) tok.type = Token_Type_AUTO;
			else if (str_eql(tok.text, "break") == 0) tok.type = Token_Type_BREAK;
			else if (str_eql(tok.text, "case") == 0) tok.type = Token_Type_CASE;
			else if (str_eql(tok.text, "char") == 0) tok.type = Token_Type_CHAR;
			else if (str_eql(tok.text, "const") == 0) tok.type = Token_Type_CONST;
			else if (str_eql(tok.text, "continue") == 0) tok.type = Token_Type_CONTINUE;
			else if (str_eql(tok.text, "default") == 0) tok.type = Token_Type_DEFAULT;
			else if (str_eql(tok.text, "do") == 0) tok.type = Token_Type_DO;
			else if (str_eql(tok.text, "double") == 0) tok.type = Token_Type_DOUBLE;
			else if (str_eql(tok.text, "else") == 0) tok.type = Token_Type_ELSE;
			else if (str_eql(tok.text, "enum") == 0) tok.type = Token_Type_ENUM;
			else if (str_eql(tok.text, "extern") == 0) tok.type = Token_Type_EXTERN;
			else if (str_eql(tok.text, "float") == 0) tok.type = Token_Type_FLOAT;
			else if (str_eql(tok.text, "for") == 0) tok.type = Token_Type_FOR;
			else if (str_eql(tok.text, "goto") == 0) tok.type = Token_Type_GOTO;
			else if (str_eql(tok.text, "if") == 0) tok.type = Token_Type_IF;
			else if (str_eql(tok.text, "int") == 0) tok.type = Token_Type_INT;
			else if (str_eql(tok.text, "long") == 0) tok.type = Token_Type_LONG;
			else if (str_eql(tok.text, "register") == 0) tok.type = Token_Type_REGISTER;
			else if (str_eql(tok.text, "return") == 0) tok.type = Token_Type_RETURN;
			else if (str_eql(tok.text, "short") == 0) tok.type = Token_Type_SHORT;
			else if (str_eql(tok.text, "signed") == 0) tok.type = Token_Type_SIGNED;
			else if (str_eql(tok.text, "sizeof") == 0) tok.type = Token_Type_SIZEOF;
			else if (str_eql(tok.text, "static") == 0) tok.type = Token_Type_STATIC;
			else if (str_eql(tok.text, "struct") == 0) tok.type = Token_Type_STRUCT;
			else if (str_eql(tok.text, "switch") == 0) tok.type = Token_Type_SWITCH;
			else if (str_eql(tok.text, "typedef") == 0) tok.type = Token_Type_TYPEDEF;
			else if (str_eql(tok.text, "union") == 0) tok.type = Token_Type_UNION;
			else if (str_eql(tok.text, "unsigned") == 0) tok.type = Token_Type_UNSIGNED;
			else if (str_eql(tok.text, "void") == 0) tok.type = Token_Type_VOID;
			else if (str_eql(tok.text, "volatile") == 0) tok.type = Token_Type_VOLATILE;
			else if (str_eql(tok.text, "while") == 0) tok.type = Token_Type_WHILE;
			else if (str_eql(tok.text, "NULL") == 0) tok.type = Token_Type_NULL;
			else if (str_eql(tok.text, "size_t") == 0) tok.type = Token_Type_SIZE_T;
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
			tok.type = Token_Type_INT_CONST;
			if (peek_char() == '.') {
				ch = next_char();
				*(tok_text++) = ch;
				while (is_num(peek_char())) {
					ch = next_char();
					*(tok_text++) = ch;
				}
				tok.type = Token_Type_FLOAT_CONST;
			}
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
	Type_VOID,
	Type_CHAR,
	Type_SHORT,
	Type_INT,
	Type_LONG,
	Type_FLOAT,
	Type_DOUBLE,
	// signed and unsigned decay to int
} typedef Type_Specifier;

struct Type {
	int is_const;
	int is_volatile;
	int is_signed;
	Type_Specifier specifier;
} typedef Type;

Type* make_type(Type_Specifier s) {
	Type* t = malloc(sizeof(Type));
	t->specifier = s;
	t->is_signed = 1;
	return t;
}

enum {
	AST_Type_TERMINAL,
	AST_Type_TYPE_NAME,
	AST_Type_CAST_EXPRESSION,
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
	AST_Type ast_type;
	Type* type;
	union {
		// TODO: condense binary ops into union
		struct {
			struct AST_node* left;
			Token op;
			struct AST_node* right;
		} binary_expression;
		struct {
			struct AST_node* condition;
			struct AST_node* left;
			struct AST_node* right;
		} conditional_expression;
		struct {
			struct AST_node* right;
		} cast_expression;
		struct {
			Token value;
		} terminal;
	};
} typedef AST_node;

// some general purpose helpers
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
	else if (t == AST_Type_CAST_EXPRESSION) return "CAST_EXPRESSION";
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

// return an AST_node terminal of current token regardless of type
AST_node* blind_accept() {
	AST_node* node = malloc(sizeof(AST_node));
	node->ast_type = AST_Type_TERMINAL;
	node->terminal.value = current_token();
	next_token();
	return node;
}

// return an AST_node terminal of current token if it is type t
AST_node* accept(Token_Type t) {
	if (t == current_token().type) {
		AST_node* node = malloc(sizeof(AST_node));
		node->ast_type = AST_Type_TERMINAL;
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
	} else if (current_token().type == Token_Type_INT_CONST) {
		left = expect(Token_Type_INT_CONST);
		left->type = make_type(Type_INT);
	} else if (current_token().type == Token_Type_FLOAT_CONST) {
		left = expect(Token_Type_FLOAT_CONST);
		left->type = make_type(Type_DOUBLE);
	}
	return left;
}

int is_type_qualifier(Token tok) {
	Token_Type t = tok.type;
	return t == Token_Type_CONST || t == Token_Type_VOLATILE;
}

int is_type_specifier(Token tok) {
	Token_Type t = tok.type;
	return t == Token_Type_VOID || 
		t == Token_Type_CHAR ||
		t == Token_Type_SHORT ||
		t == Token_Type_INT ||
		t == Token_Type_LONG ||
		t == Token_Type_FLOAT ||
		t == Token_Type_DOUBLE ||
		t == Token_Type_SIGNED ||
		t == Token_Type_UNSIGNED;
}

int is_specifier_qualifier(Token tok) {
	return is_type_specifier(tok) || is_type_qualifier(tok);
}

Type_Specifier token_to_specifier(Token tok) {
	Token_Type t = tok.type;
	if (t == Token_Type_VOID) return Type_VOID;
	else if (t == Token_Type_CHAR) return Type_CHAR;
	else if (t == Token_Type_SHORT) return Type_SHORT;
	else if (t == Token_Type_INT) return Type_INT;
	else if (t == Token_Type_LONG) return Type_LONG;
	else if (t == Token_Type_FLOAT) return Type_FLOAT;
	else if (t == Token_Type_DOUBLE) return Type_DOUBLE;
	error_internal("Couldn't convert token to specifier..");
}

Type* parse_type_name() {
	int found_specifier = 0;
	Type* node = malloc(sizeof(AST_node));
	node->is_signed = 1;
	if (!is_specifier_qualifier(current_token()))
		error_unexpected_token(current_token(), Token_Type_VOID); // TODO: expect multiple types of tokens
	while (is_specifier_qualifier(current_token())) {
		Token tok = current_token();
		if (is_type_qualifier(tok)) {
			if (tok.type == Token_Type_CONST) {
				node->is_const = 1;
			} else if (tok.type == Token_Type_VOLATILE) {
				node->is_volatile = 1;
			}
		} else {
			if (tok.type == Token_Type_SIGNED) {
				if (found_specifier == 0) node->specifier = Type_INT;
				node->is_signed = 1;
			} else if (tok.type == Token_Type_UNSIGNED) {
				if (found_specifier == 0) node->specifier = Type_INT;
				node->is_signed = 0;
			} else node->specifier = token_to_specifier(tok);
			found_specifier = 1;
		}
		blind_accept();
	}
	if (found_specifier == 0) {
		error_unexpected_token(current_token(), Token_Type_VOID);
	}
	return node;
	// ill worry about abstract declarators later fml
}

int is_type_name(Token tok) {
	// TODO: this will need to check typedef'd types eventually
	// this should be first(type_name)
	Token_Type t = tok.type;
	return is_type_specifier(tok) || is_type_qualifier(tok) ||
			t == Token_Type_STRUCT ||
			t == Token_Type_UNION ||
			t == Token_Type_ENUM;
}

AST_node* parse_cast_expression() {
	if (current_token().type == Token_Type_O_PAREN && is_type_name(peek_token())) {
		AST_node* node = malloc(sizeof(AST_node));
		node->ast_type = AST_Type_CAST_EXPRESSION;
		expect(Token_Type_O_PAREN);
		node->type = parse_type_name();
		expect(Token_Type_C_PAREN);
		node->cast_expression.right = parse_primary_expression();
		return node;
	} else {
		return parse_primary_expression();
	}
}

Type* binary_type_coercion_arithmetic(Type* a, Type* b) {
	Type* type = make_type(Type_INT);
	type->is_signed = a->is_signed & b->is_signed;
	if (a->specifier == Type_LONG || b->specifier == Type_LONG) type->specifier = Type_LONG;
	if (a->specifier == Type_FLOAT || b->specifier == Type_FLOAT) type->specifier = Type_FLOAT;
	if (a->specifier == Type_DOUBLE || b->specifier == Type_DOUBLE) type->specifier = Type_DOUBLE;
	return type;
}

Type* binary_type_coercion_shift(Type* a, Type* b) {
	Type* type = make_type(Type_INT);
	type->is_signed = a->is_signed;
	if (a->specifier == Type_LONG || b->specifier == Type_LONG) type->specifier = Type_LONG;
	if (a->specifier == Type_FLOAT || b->specifier == Type_FLOAT) error_invalid_type("Invalid type for shift operator");
	if (a->specifier == Type_DOUBLE || b->specifier == Type_DOUBLE) error_invalid_type("Invalid type for shift operator");
	return type;
}

Type* binary_type_coercion_relational(Type* a, Type* b) {
	Type* type = make_type(Type_INT);
	return type;
}

Type* binary_type_coercion_bitwise(Type* a, Type* b) {
	Type* type = make_type(Type_INT);
	type->is_signed = a->is_signed & b->is_signed;
	if (a->specifier == Type_LONG || b->specifier == Type_LONG) type->specifier = Type_LONG;
	if (a->specifier == Type_FLOAT || b->specifier == Type_FLOAT) error_invalid_type("Invalid type for shift operator");
	if (a->specifier == Type_DOUBLE || b->specifier == Type_DOUBLE) error_invalid_type("Invalid type for shift operator");
	return type;
}

Type* binary_type_coercion_logical(Type* a, Type* b) {
	Type* type = make_type(Type_INT);
	return type;
}

// all binary ops use the exact same logic
#define PARSE_BINARY_EXPRESSION(NAME, OPERATOR, AST, NEXT, TYPE) AST_node* NAME() { \
	AST_node* left = NEXT(); \
	while (current_token().type == OPERATOR) { \
		AST_node* node = malloc(sizeof(AST_node)); \
		node->ast_type = AST; \
		node->binary_expression.left = left; \
		node->binary_expression.op = current_token(); \
		expect(OPERATOR); \
		node->binary_expression.right = NEXT(); \
		node->type = TYPE(node->binary_expression.left->type, node->binary_expression.right->type); \
		left = node; \
	} \
	return left; \
}

PARSE_BINARY_EXPRESSION(parse_multiplicative_expression, Token_Type_MULTIPLICATIVE,
		AST_Type_MULTIPLICATIVE_EXPRESSION, parse_cast_expression, binary_type_coercion_arithmetic)
PARSE_BINARY_EXPRESSION(parse_additive_expression, Token_Type_ADDITIVE,
		AST_Type_ADDITIVE_EXPRESSION, parse_multiplicative_expression, binary_type_coercion_arithmetic)
PARSE_BINARY_EXPRESSION(parse_shift_expression, Token_Type_SHIFT,
		AST_Type_SHIFT_EXPRESSION, parse_additive_expression, binary_type_coercion_shift)
PARSE_BINARY_EXPRESSION(parse_relational_expression, Token_Type_RELATIONAL,
		AST_Type_RELATIONAL_EXPRESSION, parse_shift_expression, binary_type_coercion_relational)
PARSE_BINARY_EXPRESSION(parse_equality_expression, Token_Type_EQUALITY,
		AST_Type_EQUALITY_EXPRESSION, parse_relational_expression, binary_type_coercion_relational)
PARSE_BINARY_EXPRESSION(parse_and_expression, Token_Type_AND,
		AST_Type_AND_EXPRESSION, parse_equality_expression, binary_type_coercion_bitwise)
PARSE_BINARY_EXPRESSION(parse_exclusive_or_expression, Token_Type_EXCLUSIVE_OR,
		AST_Type_EXCLUSIVE_OR_EXPRESSION, parse_and_expression, binary_type_coercion_bitwise)
PARSE_BINARY_EXPRESSION(parse_inclusive_or_expression, Token_Type_INCLUSIVE_OR,
		AST_Type_INCLUSIVE_OR_EXPRESSION, parse_exclusive_or_expression, binary_type_coercion_bitwise)
PARSE_BINARY_EXPRESSION(parse_logical_and_expression, Token_Type_LOGICAL_AND,
		AST_Type_LOGICAL_AND_EXPRESSION, parse_inclusive_or_expression, binary_type_coercion_logical)
PARSE_BINARY_EXPRESSION(parse_logical_or_expression, Token_Type_LOGICAL_OR,
		AST_Type_LOGICAL_OR_EXPRESSION, parse_logical_and_expression, binary_type_coercion_logical)

AST_node* parse_conditional_expression() {
	AST_node* left = parse_logical_or_expression();
	if (current_token().type == Token_Type_CONDITIONAL) {
		AST_node* node = malloc(sizeof(AST_node));
		node->ast_type = AST_Type_CONDITIONAL_EXPRESSION;
		node->conditional_expression.condition = left;
		expect(Token_Type_CONDITIONAL);
		node->conditional_expression.left = parse_conditional_expression(); // TODO: this should be expression
		expect(Token_Type_COLON);
		node->conditional_expression.right = parse_conditional_expression();
		node->type = binary_type_coercion_arithmetic(node->conditional_expression.left->type, node->conditional_expression.right->type);
		return node;
	}
	return left;
}

// for debugging purposes
void print_token(Token tok, int depth) {
	for (int i = 0; i < depth; i++)
		printf("    ");
	printf("Token: %s of type %s\n", tok.text, Token_Type_to_str(tok.type));
}

void print_type(Type* type, int depth) {
	for (int i = 0; i < depth; i++)
		printf("    ");
	if (type->is_const) printf("CONST ");
	if (type->is_volatile) printf("VOLATILE ");
	if (type->is_signed == 0) printf("UNSIGNED ");

	if (type->specifier == Type_VOID) printf("VOID");
	else if (type->specifier == Type_CHAR) printf("CHAR");
	else if (type->specifier == Type_SHORT) printf("SHORT");
	else if (type->specifier == Type_INT) printf("INT");
	else if (type->specifier == Type_LONG) printf("LONG");
	else if (type->specifier == Type_FLOAT) printf("FLOAT");
	else if (type->specifier == Type_DOUBLE) printf("DOUBLE");
	printf("\n");
}

void print_ast(AST_node* node, int depth) {
	for (int i = 0; i < depth; i++)
		printf("    ");
	if (node->ast_type == AST_Type_TERMINAL) {
		printf("Terminal: \n");
		print_token(node->terminal.value, depth + 1);
		print_type(node->type, depth + 1);
	} else if (AST_is_binary(node->ast_type)) {
		printf("Node: %s\n", AST_Type_to_str(node->ast_type));
		print_ast(node->binary_expression.left, depth + 1);
		print_token(node->binary_expression.op, depth + 1);
		print_ast(node->binary_expression.right, depth + 1);
		print_type(node->type, depth + 1);
	} else if (node->ast_type == AST_Type_CONDITIONAL_EXPRESSION) {
		printf("Node: CONDITIONAL_EXPRESSION\n");
		print_ast(node->conditional_expression.condition, depth + 1);
		print_ast(node->conditional_expression.left, depth + 1);
		print_ast(node->conditional_expression.right, depth + 1);
		print_type(node->type, depth + 1);
	} else if (node->ast_type == AST_Type_CAST_EXPRESSION) {
		printf("Node: CAST_EXPRESSION\n");
		print_type(node->type, depth + 1);
		print_ast(node->cast_expression.right, depth + 1);
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
uint64_t file_byte = 0;

unsigned char rodata_buf[4096];
uint64_t rodata_byte = 0;
unsigned char text_buf[65536];
uint64_t text_byte = 0;

// append a byte to output file
void emit_byte_file(unsigned char byte) {
	putc(byte, out_file);
	file_byte++;
}
void emit_byte(unsigned char byte, unsigned char* buffer, uint64_t* buffer_ptr) {
	buffer[(*buffer_ptr)++] = byte;
}

void emit_byte_text(unsigned char byte) {
	emit_byte(byte, text_buf, &text_byte);
}

void emit_byte_rodata(unsigned char byte) {
	emit_byte(byte, rodata_buf, &rodata_byte);
}

// set byte at address in buffer
void set_byte(uint64_t addr, unsigned char byte, unsigned char* buffer) {
	buffer[addr] = byte;
}

// set an i32 at address in file
void set_i32(uint64_t addr, int32_t data, unsigned char* buffer) {
	buffer[addr] = data & 0xff;
	buffer[addr + 1] = (data >> 8) & 0xff;
	buffer[addr + 2] = (data >> 16) & 0xff;
	buffer[addr + 3] = (data >> 24) & 0xff;
}

// set an i64 at address in file
void set_i64(uint64_t addr, int64_t data, unsigned char* buffer) {
	buffer[addr] = data & 0xff;
	buffer[addr + 1] = (data >> 8) & 0xff;
	buffer[addr + 2] = (data >> 16) & 0xff;
	buffer[addr + 3] = (data >> 24) & 0xff;
	buffer[addr + 4] = (data >> 32) & 0xff;
	buffer[addr + 5] = (data >> 40) & 0xff;
	buffer[addr + 6] = (data >> 48) & 0xff;
	buffer[addr + 7] = (data >> 56) & 0xff;
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
	int PF_X = 0x1;
	int PF_W = 0x2;
	int PF_R = 0x4;

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
	header.e_phnum     = 0x02; // # of program headers
	header.e_shentsize = 0x40; // section header size
	header.e_shnum     = 0x04; // # of section headers
	header.e_shstrndx  = 0x03; // index of names section in table

	header.e_entry     = start_address + header.e_phoff + 2 * header.e_phentsize + rodata_byte;

	int header_bytes = sizeof(Elf64_Header);
	for (int i = 0; i < header_bytes; i++)
		emit_byte_file(((unsigned char*)&header)[i]);

	// program headers
	int pheader_bytes = sizeof(Elf64_Program_Header);

	Elf64_Program_Header rodata_header;
	
	rodata_header.p_type     = 0x01; // load segment to memory
	rodata_header.p_offset   = header.e_ehsize + pheader_bytes * 2; // data starts after next header
	rodata_header.p_vaddr    = rodata_header.p_offset + start_address;
	rodata_header.p_paddr    = rodata_header.p_vaddr;
	rodata_header.p_filesz   = rodata_byte;
	rodata_header.p_memsz    = rodata_byte;
	rodata_header.p_flags    = PF_R;
	rodata_header.p_align    = 0x1000; // also no idea


	Elf64_Program_Header text_header;
	
	text_header.p_type     = 0x01; // load segment to memory
	text_header.p_offset   = rodata_header.p_offset + rodata_byte; // text starts after rodata
	text_header.p_vaddr    = text_header.p_offset + start_address;
	text_header.p_paddr    = text_header.p_vaddr;
	text_header.p_filesz   = text_byte;
	text_header.p_memsz    = text_byte;
	text_header.p_flags    = PF_R | PF_X;
	text_header.p_align    = 0x1000; // also no idea

	// emit program headers
	for (int i = 0; i < pheader_bytes; i++)
		emit_byte_file(((unsigned char*)&rodata_header)[i]);

	for (int i = 0; i < pheader_bytes; i++)
		emit_byte_file(((unsigned char*)&text_header)[i]);

	// just to make sure we are in the correct location
	while (file_byte < header.e_entry - start_address)
		emit_byte_file(0x00);

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
	label->offset = text_byte;
	labels[label->id].offset = text_byte;
}

void resolve_labels() {
	for (int i = 0; i < resolution_count; i++) {
		set_i32(resolution_addresses[i], labels[resolution_ids[i]].offset - (resolution_addresses[i] + 4), text_buf);
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
				
	// stupid chud floating point registers
	Register_XMM0,
	Register_XMM1,
	Register_XMM2,
	Register_XMM3,
	Register_XMM4,
	Register_XMM5,
	Register_XMM6,
	Register_XMM7,

	Register_XMM8,
	Register_XMM9,
	Register_XMM10,
	Register_XMM11,
	Register_XMM12,
	Register_XMM13,
	Register_XMM14,
	Register_XMM15,
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
		emit_byte_text(0x49);
	} else {
		emit_byte_text(0x48);
	}
	emit_byte_text(0xc7);
	emit_byte_text(0xc0 + reg_canon(reg));
	// little endian
	emit_byte_text(val & 0xff);
	emit_byte_text((val >> 8) & 0xff);
	emit_byte_text((val >> 16) & 0xff);
	emit_byte_text((val >> 24) & 0xff);
}

void emit_push_reg(Codegen_Register reg) {
	if (reg_is_64bit(reg)) {
		emit_byte_text(0x41);
	}
	emit_byte_text(0x50 + reg_canon(reg));
}

void emit_pop_reg(Codegen_Register reg) {
	if (reg_is_64bit(reg)) {
		emit_byte_text(0x41);
	}
	emit_byte_text(0x58 + reg_canon(reg));
}

void emit_syscall() {
	emit_byte_text(0x0f); emit_byte_text(0x05);
}

void emit_add_reg_reg(Codegen_Register reg1, Codegen_Register reg2) {
	if (reg_is_64bit(reg2)) {
		emit_byte_text(0x4c + reg_is_64bit(reg1));
	} else {
		emit_byte_text(0x48 + reg_is_64bit(reg1));
	}
	emit_byte_text(0x01);
	emit_byte_text(0xc0 + (reg_canon(reg2) << 3) + reg_canon(reg1));
}

void emit_sub_reg_reg(Codegen_Register reg1, Codegen_Register reg2) {
	if (reg_is_64bit(reg2)) {
		emit_byte_text(0x4c + reg_is_64bit(reg1));
	} else {
		emit_byte_text(0x48 + reg_is_64bit(reg1));
	}
	emit_byte_text(0x29);
	emit_byte_text(0xc0 + (reg_canon(reg2) >> 3) + reg_canon(reg1));
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
		emit_byte_text(0x48 + reg_is_64bit(reg1));
		emit_byte_text(0x90 + reg_canon(reg2));
	} else {
		emit_byte_text(0x48 + reg_is_64bit(reg1) + (reg_is_64bit(reg2) >> 3));
		emit_byte_text(0x87);
		emit_byte_text(0xc0 + (reg_canon(reg2) >> 3) + reg_canon(reg1));
	}
}

void emit_cqo() {
	emit_byte_text(0x48);
	emit_byte_text(0x99);
}

void emit_label(Label target) {
	// 32bit relative addr, resolve later
	resolution_addresses[resolution_count] = text_byte;
	resolution_ids[resolution_count] = target.id;
	resolution_count++;
	// unknown addr for now
	emit_byte_text(0x00); emit_byte_text(0x00); emit_byte_text(0x00); emit_byte_text(0x00);
}

void emit_jz(Label target) {
	emit_byte_text(0x0f);
	emit_byte_text(0x84);
	emit_label(target);
}

void emit_jnz(Label target) {
	emit_byte_text(0x0f);
	emit_byte_text(0x85);
	emit_label(target);
}

void emit_jmp(Label target) {
	emit_byte_text(0xe9);
	emit_label(target);
}

int short_circuits(const char* op) {
	return !str_eql(op, "&&") || !str_eql(op, "||");
}

void generate_code(AST_node* node) {
	// simple approach: move terminals into rax
	if (node->ast_type == AST_Type_TERMINAL) {
		Token_Type tt = node->terminal.value.type;
		if (tt == Token_Type_INT_CONST) {
			int64_t val = atoi(node->terminal.value.text);
			emit_mov_reg_imm(Register_RAX, val);
		} else if (tt == Token_Type_FLOAT_CONST) {
			double val = atof(node->terminal.value.text);
			// TODO: come back to this later.. i have to implement sections in
			// elf for rodata.. :(
		}
		return;
	} else if (AST_is_binary(node->ast_type)) {
		// generate code for left into rax, push, generate code for right into rax,
		// pop into rcx and operate
		AST_node* left  = node->binary_expression.left;
		Token op    = node->binary_expression.op;
		AST_node* right = node->binary_expression.right;
		char* op_text = op.text;

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
			emit_byte_text(0x48); emit_byte_text(0x0f); emit_byte_text(0xaf); emit_byte_text(0xc1);
		} else if (str_eql(op_text, "/") == 0) {
			emit_xchg_reg_reg(Register_RCX, Register_RAX);
			emit_cqo();
			// idiv rcx
			emit_byte_text(0x48); emit_byte_text(0xf7); emit_byte_text(0xf9);
		} else if (str_eql(op_text, "%") == 0) {
			emit_xchg_reg_reg(Register_RCX, Register_RAX);
			emit_cqo();
			// idiv rcx
			emit_byte_text(0x48); emit_byte_text(0xf7); emit_byte_text(0xf9);
			// mod is stored in rdx, move to rax
			emit_xchg_reg_reg(Register_RAX, Register_RDX);
		} else if (str_eql(op_text, "<<") == 0) {
			emit_xchg_reg_reg(Register_RAX, Register_RCX);
			// shl rax, cl
			emit_byte_text(0x48); emit_byte_text(0xd3); emit_byte_text(0xe0);
		} else if (str_eql(op_text, ">>") == 0) {
			emit_xchg_reg_reg(Register_RAX, Register_RCX);
			// shr rax, cl
			emit_byte_text(0x48); emit_byte_text(0xd3); emit_byte_text(0xe8);
		} else if (str_eql(op_text, "<") == 0) {
			// cmp rax, rcx
			emit_byte_text(0x48); emit_byte_text(0x39); emit_byte_text(0xc1);
			// setl al
			emit_byte_text(0x0f); emit_byte_text(0x9c); emit_byte_text(0xc0);
			// and al, 1
			emit_byte_text(0x24); emit_byte_text(0x01);
		} else if (str_eql(op_text, "<=") == 0) {
			// cmp rax, rcx
			emit_byte_text(0x48); emit_byte_text(0x39); emit_byte_text(0xc1);
			// setle al
			emit_byte_text(0x0f); emit_byte_text(0x9e); emit_byte_text(0xc0);
			// and al, 1
			emit_byte_text(0x24); emit_byte_text(0x01);
		} else if (str_eql(op_text, ">") == 0) {
			// cmp rax, rcx
			emit_byte_text(0x48); emit_byte_text(0x39); emit_byte_text(0xc1);
			// setg al
			emit_byte_text(0x0f); emit_byte_text(0x9f); emit_byte_text(0xc0);
			// and al, 1
			emit_byte_text(0x24); emit_byte_text(0x01);
		} else if (str_eql(op_text, ">=") == 0) {
			// cmp rax, rcx
			emit_byte_text(0x48); emit_byte_text(0x39); emit_byte_text(0xc1);
			// setge al
			emit_byte_text(0x0f); emit_byte_text(0x9d); emit_byte_text(0xc0);
			// and al, 1
			emit_byte_text(0x24); emit_byte_text(0x01);
		} else if (str_eql(op_text, "==") == 0) {
			// cmp rax, rcx
			emit_byte_text(0x48); emit_byte_text(0x39); emit_byte_text(0xc1);
			// sete al
			emit_byte_text(0x0f); emit_byte_text(0x94); emit_byte_text(0xc0);
			// and al, 1
			emit_byte_text(0x24); emit_byte_text(0x01);
		} else if (str_eql(op_text, "!=") == 0) {
			// cmp rax, rcx
			emit_byte_text(0x48); emit_byte_text(0x39); emit_byte_text(0xc1);
			// setne al
			emit_byte_text(0x0f); emit_byte_text(0x95); emit_byte_text(0xc0);
			// and al, 1
			emit_byte_text(0x24); emit_byte_text(0x01);
		} else if (str_eql(op_text, "&") == 0) {
			// and rax, rcx
			emit_byte_text(0x48); emit_byte_text(0x21); emit_byte_text(0xc8);
		} else if (str_eql(op_text, "^") == 0) {
			// xor rax, rcx
			emit_byte_text(0x48); emit_byte_text(0x31); emit_byte_text(0xc8);
		} else if (str_eql(op_text, "|") == 0) {
			// or rax, rcx
			emit_byte_text(0x48); emit_byte_text(0x09); emit_byte_text(0xc8);
		} else if (str_eql(op_text, "&&") == 0) {
			Label false_label = make_label();
			Label end_label = make_label();

			generate_code(left);
			// test rax, rax
			emit_byte_text(0x48); emit_byte_text(0x85); emit_byte_text(0xc0);
			emit_jz(false_label);

			generate_code(right);
			// and al, 1
			emit_byte_text(0x24); emit_byte_text(0x01);
			emit_jmp(end_label);

			mark_label(&false_label);
			// xor rax, rax
			emit_byte_text(0x48); emit_byte_text(0x31); emit_byte_text(0xc0);

			mark_label(&end_label);
		} else if (str_eql(op_text, "||") == 0) {
			Label true_label = make_label();
			Label end_label = make_label();

			generate_code(left);
			// test rax, rax
			emit_byte_text(0x48); emit_byte_text(0x85); emit_byte_text(0xc0);
			emit_jnz(true_label);

			generate_code(right);
			// and al, 1
			emit_byte_text(0x24); emit_byte_text(0x01);
			emit_jmp(end_label);

			mark_label(&true_label);
			// mov al, 1
			emit_byte_text(0xb0); emit_byte_text(0x01);

			mark_label(&end_label);
		} else {
			char buf[128] = "Operator %d not implemented yet :P";
			snprintf(buf, sizeof(buf), buf, op_text);
			error_internal(buf);
		}
	} else if (node->ast_type == AST_Type_CONDITIONAL_EXPRESSION) {
		Label false_label = make_label();
		Label end_label = make_label();

		AST_node* condition = node->conditional_expression.condition;
		AST_node* left = node->conditional_expression.left;
		AST_node* right = node->conditional_expression.right;
		
		generate_code(condition);
		// test rax, rax
		emit_byte_text(0x48); emit_byte_text(0x85); emit_byte_text(0xc0);
		emit_jz(false_label);

		// true_label
		generate_code(left);
		emit_jmp(end_label);

		mark_label(&false_label);
		generate_code(right);

		mark_label(&end_label);
	}
}

void emit_exit() {
	// mov rdi, rax (mov result to exit val)
	emit_byte_text(0x48); emit_byte_text(0x87); emit_byte_text(0xc7);
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

	// codegen
	generate_code(program);
	emit_exit();
	resolve_labels();

	// prepare output
	out_file = fopen("jcc.out", "wb");
	write_elf_header();
	fwrite(rodata_buf, 1, rodata_byte, out_file);
	fwrite(text_buf, 1, text_byte, out_file);

	// cleanup and allow execution
	fclose(out_file);
	chmod("jcc.out", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); // +X
}
