#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

enum e_syntax_error
{
    NO_SYN_ERR,
    UNEXPECTED_EOF,
    UNEXPECTED_TOKEN,
    WTF // malloc failure
};

enum e_node_type
{
    NT_CMD = 1,
    NT_OP
};

enum e_op_type
{
    OT_OR = 1,
    OT_AND,
    OT_PIPE,
};

enum e_io_type
{
    IO_REDIRECT_INPUT = 1,
    IO_REDIRECT_OUTPUT,
    IO_APPEND_OUTPUT,
    IO_HEREDOC,
};

typedef struct s_parser
{
    t_token *current_token;
    enum e_syntax_error error;
} t_parser;

typedef struct s_cmd_io
{
    char *file;
    enum e_io_type type;
    struct s_cmd_io *next;
} t_cmd_io;

typedef struct s_cmd_arg
{
    char *name;
    struct s_cmd_arg *next;
} t_cmd_arg;

typedef struct s_cmd_data
{
    t_cmd_arg *arg;
    t_cmd_io *io;
} t_cmd_data;

typedef struct s_node
{
    enum e_node_type type;
    union u_node_data *data;
} t_node;

typedef struct s_op_data
{
    enum e_op_type type;
    t_node *left;
    t_node *right;
} t_op_data;

union u_node_data
{
    t_cmd_data *cmd_data;
    t_op_data *op_data;
};

typedef struct s_ast
{
    t_node *root;
    enum e_lexical_error lex_err;
    enum e_syntax_error syn_err;
    char *token_err;
} t_ast;

/******* Testing only *********/

t_parser *parser_init(t_lexer *lex);
t_node *parser_run(t_parser *par, int op_prec);
void node_free(t_node *node);

/*****************************/

t_ast *ast_parse(char *line);
void ast_print_err(t_ast *ast);
void ast_free(t_ast *ast);

#endif