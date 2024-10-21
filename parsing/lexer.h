#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <cjson/cJSON.h>
#include <ctype.h>


enum e_quote_state
{
    NONE,
    SINGLE = '\'',
    DOUBLE = '"',
};

enum e_token_type
{
    TT_EOF,
    TT_WORD,

    TT_REDIRECT_INPUT = '<',
    TT_REDIRECT_OUTPUT = '>',
    TT_LEFT_PAREN = '(',
    TT_RIGHT_PAREN = ')',
    TT_AMPERSAND = '&',
    TT_PIPE = '|',

    TT_OR,            // ||
    TT_AND,           // &&
    TT_APPEND_OUTPUT, // >>
    TT_HEREDOC,       // <<

};

enum e_lexical_error
{
    NO_LEX_ERR,
    UNCLOSED_DOUBLE_QUOTATION,
    UNCLOSED_SINGLE_QUOTATION
};

typedef struct s_token
{
    char *lexeme;
    size_t len;
    enum e_token_type type;
    struct s_token *next;
} t_token;

typedef struct s_token_stream
{
    t_token *start;
    t_token *end;
    size_t count;
} t_token_stream;

typedef struct s_lexer
{
    char *line;
    enum e_quote_state quote_state;
    int delim_state;
    int heredoc_state;
    size_t nparsed;
    enum e_lexical_error error;
    t_token_stream *token_stream;
} t_lexer;

t_lexer *lexer_init(char *line);
void lexer_run(t_lexer *lex);
void lexer_print_err(enum e_lexical_error lex_err);
void lexer_free(t_lexer *lex);

#endif