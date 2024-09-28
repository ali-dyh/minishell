#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

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
    NO_ERR,
    UNCLOSED_QUOTATION = 12,
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
    size_t nparsed;
    enum e_lexical_error error;
    t_token_stream *token_stream;
} t_lexer;

int ft_is_quote(char c)
{
    return (c == '\'' || c == '"');
}
int ft_is_space(char c)
{
    return (c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' ');
}

int ft_is_seperator(char c)
{
    return (c == '<' || c == '>' || c == '(' || c == ')' || c == '&' || c == '|');
}

int token_lexeme_append(t_lexer *lex, size_t len)
{
    size_t i;
    t_token *token;

    i = 0;
    token = lex->token_stream->end;
    /// TODO: replace realloc with ft_realloc
    if (token->lexeme)
        token->lexeme = (char *)realloc(token->lexeme, token->len + len + 1);
    else
        token->lexeme = (char *)malloc(len + 1);
    if (!token->lexeme)
        return -1;
    while (i < len)
    {
        token->lexeme[token->len] = lex->line[lex->nparsed];
        token->len++;
        lex->nparsed++;
        i++;
    }
    token->lexeme[token->len] = '\0';
    return 0;
}
void append_token(t_lexer *lex)
{
    token_lexeme_append(lex, 1);
}
void push_token(t_lexer *lex, enum e_token_type type, size_t len)
{
    t_token *token = (t_token *)malloc(sizeof(t_token));
    /// TODO: handle the malloc failure
    // if (!token)
    token->lexeme = NULL;
    token->next = NULL;
    token->type = type;
    token->len = 0;
    if (!lex->token_stream->start)
        lex->token_stream->start = token;
    else
        lex->token_stream->end->next = token;
    lex->token_stream->end = token;
    lex->token_stream->count++;
    token_lexeme_append(lex, len);
}
void handle_quote(t_lexer *lex)
{
    if (lex->quote_state == lex->line[lex->nparsed])
    {
        if (!lex->token_stream->end || !lex->token_stream->end->len)
            push_token(lex, TT_WORD, 0);
        lex->quote_state = NONE;
    }
    else
        lex->quote_state = lex->line[lex->nparsed];
    lex->nparsed++;
}

void handle_space(t_lexer *lex)
{
    lex->delim_state = 1;
    lex->nparsed++;
}

void handle_seperator(t_lexer *lex)
{
    if (lex->line[lex->nparsed] == TT_PIPE && lex->line[lex->nparsed + 1] == TT_PIPE)
        push_token(lex, TT_OR, 2);
    else if (lex->line[lex->nparsed] == TT_AMPERSAND && lex->line[lex->nparsed + 1] == TT_AMPERSAND)
        push_token(lex, TT_AND, 2);
    else if (lex->line[lex->nparsed] == TT_REDIRECT_INPUT && lex->line[lex->nparsed + 1] == TT_REDIRECT_INPUT)
        push_token(lex, TT_HEREDOC, 2);
    else if (lex->line[lex->nparsed] == TT_REDIRECT_OUTPUT && lex->line[lex->nparsed + 1] == TT_REDIRECT_OUTPUT)
        push_token(lex, TT_APPEND_OUTPUT, 2);
    else if (lex->line[lex->nparsed] == TT_PIPE)
        push_token(lex, TT_PIPE, 1);
    else if (lex->line[lex->nparsed] == TT_AMPERSAND)
        push_token(lex, TT_AMPERSAND, 1);
    else if (lex->line[lex->nparsed] == TT_REDIRECT_INPUT)
        push_token(lex, TT_REDIRECT_INPUT, 1);
    else if (lex->line[lex->nparsed] == TT_REDIRECT_OUTPUT)
        push_token(lex, TT_REDIRECT_OUTPUT, 1);
    else if (lex->line[lex->nparsed] == TT_LEFT_PAREN)
        push_token(lex, TT_LEFT_PAREN, 1);
    else if (lex->line[lex->nparsed] == TT_RIGHT_PAREN)
        push_token(lex, TT_RIGHT_PAREN, 1);
    lex->delim_state = 1;
}

t_lexer *lexer_init(char *line)
{
    t_lexer *lexer = (t_lexer *)malloc(sizeof(t_lexer));
    if (!lexer)
        return (NULL);
    lexer->nparsed = 0;
    lexer->quote_state = NONE;
    lexer->delim_state = 0;
    lexer->line = line;
    lexer->error = NO_ERR;
    lexer->token_stream = (t_token_stream *)malloc(sizeof(t_token_stream));
    // TODO: free lexer on token_stream failure and return NULL
    lexer->token_stream->start = NULL;
    lexer->token_stream->end = NULL;
    lexer->token_stream->count = 0;
    return (lexer);
}

void handle_word(t_lexer *lex)
{
    if (lex->delim_state || !lex->token_stream->end)
        push_token(lex, TT_WORD, 1);
    else
        append_token(lex);
    lex->delim_state = 0;
}

void lexer_run(t_lexer *lex)
{
    char c;
    // size_t line_len;

    // TODO: replace strlen width ft_strlen
    // line_len = strlen(lex->line);
    while (!lex->error && lex->line[lex->nparsed] != TT_EOF)
    {
        c = lex->line[lex->nparsed];
        if (ft_is_quote(c) && (!lex->quote_state || c == lex->quote_state))
            handle_quote(lex);
        else if (ft_is_space(c) && !lex->quote_state)
            handle_space(lex);
        else if (ft_is_seperator(c) && !lex->quote_state)
            handle_seperator(lex);
        else
            handle_word(lex);
    }
    if (lex->quote_state)
        lex->error = UNCLOSED_QUOTATION;
}

void print_token_type(enum e_token_type token_type)
{
    switch (token_type)
    {
    case TT_REDIRECT_INPUT:
        printf("REDIRECT_INPUT\n");
        break;
    case TT_REDIRECT_OUTPUT:
        printf("REDIRECT_OUTPUT\n");
        break;
    case TT_LEFT_PAREN:
        printf("LEFT_PAREN\n");
        break;
    case TT_RIGHT_PAREN:
        printf("RIGHT_PAREN\n");
        break;
    case TT_AMPERSAND:
        printf("AMPERSAND\n");
        break;
    case TT_OR:
        printf("OR\n");
        break;
    case TT_PIPE:
        printf("PIPE\n");
        break;
    case TT_AND:
        printf("AND\n");
        break;
    case TT_APPEND_OUTPUT:
        printf("APPEND_OUTPUT\n");
        break;
    case TT_HEREDOC:
        printf("HEREDOC\n");
        break;
    default:
        printf("WORD\n");
    }
}

void token_dump(t_lexer *lex)
{
    t_token *token = lex->token_stream->start;
    printf("%lu tokens in total\n", lex->token_stream->count);
    while (token)
    {
        printf("token <%s> | size: %ld | strlen: %ld", token->lexeme, token->len, strlen(token->lexeme));
        printf(" | type: ");
        print_token_type(token->type);
        token = token->next;
    }
}
// t_token *get_next_token(t_lexer *lex)
// {
//     static size_t count = 0;
//     size_t i;
//     t_token *token;

//     i = 0;
//     token = lex->token_stream->start;
//     if (count == lex->token_stream->count)
//         return NULL;
//     // WARNING: this will cause a SEG FAULT if token is NULL
//     while (i++ < count)
//         token = token->next;
//     count++;
//     return token;
// }

// void token_dump2(t_lexer *lex)
// {
//     t_token *token;
//     printf("%lu tokens in total\n", lex->token_stream->count);
//     while ((token = get_next_token(lex)))
//     {
//         printf("token <%s> | size: %ld | strlen: %ld", token->lexeme, token->len, strlen(token->lexeme));
//         printf(" | type: ");
//         print_token_type(token->type);
//     }
// }

/************************************* Parser *****************************************/

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

enum e_cmd_io_type
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
    enum e_cmd_io_type type;
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

t_node *new_node(enum e_node_type type)
{
    t_node *node;

    node = (t_node *)malloc(sizeof(t_node));
    if (!node)
        return (NULL);
    node->type = type;
    node->data = (union u_node_data *)malloc(sizeof(union u_node_data));
    if (node->data)
        return node;
    free(node);
    return NULL;
};

t_token *get_next_token(t_parser *par)
{
    if (!par->current_token)
        return NULL;
    par->current_token = par->current_token->next;
    return (par->current_token);
}

t_cmd_arg *last_cmd_arg(t_cmd_data *data)
{
    t_cmd_arg *arg;

    arg = data->arg;
    while (arg && arg->next)
        arg = arg->next;
    return arg;
}
t_cmd_io *last_cmd_io(t_cmd_data *data)
{
    t_cmd_io *io;

    io = data->io;
    while (io && io->next)
        io = io->next;
    return io;
}
t_cmd_arg *new_cmd_arg(char *arg_name)
{
    t_cmd_arg *arg;

    arg = (t_cmd_arg*)malloc(sizeof(t_cmd_arg));
    if (!arg)
        return (NULL);
    // TODO: replace strdup with ft_strdup and malloc check
    arg->name = strdup(arg_name);
    arg->next = NULL;
    return arg;
}
t_cmd_io *new_cmd_io(enum e_cmd_io_type io_type)
{
    t_cmd_io *io;

    io = (t_cmd_io*)malloc(sizeof(t_cmd_io));
    if (!io)
        return (NULL);
    io->type = io_type;
    io->file = NULL;
    io->next = NULL;
    return io;
}

void push_cmd_arg(t_parser *par, t_cmd_data *data)
{
    t_cmd_arg *arg;

    if (data->arg)
    {
        arg = last_cmd_arg(data);
        arg->next = new_cmd_arg(par->current_token->lexeme);
    }
    else
       data->arg = new_cmd_arg(par->current_token->lexeme);
    if (!data->arg || !arg->next)
        par->error = WTF;
}

enum e_cmd_io_type get_io_type(enum e_token_type ttype) {
    if (ttype == TT_REDIRECT_INPUT)
        return (IO_REDIRECT_INPUT);
    else if (ttype == TT_REDIRECT_OUTPUT)
        return (IO_REDIRECT_OUTPUT);
    else if (ttype == TT_APPEND_OUTPUT)
        return (IO_APPEND_OUTPUT);
    else if (ttype == TT_HEREDOC)
        return (IO_HEREDOC);
    return (0);
}

void push_cmd_io(t_parser *par, t_cmd_data *data)
{
    t_cmd_io *io;
    t_token *token;

    if (data->io)
    {
        io = last_cmd_io(data);
        io->next = new_cmd_io(get_io_type(par->current_token->type));
    }
    else
        data->io = new_cmd_io(get_io_type(par->current_token->type));
    // if (!data->io || !io->next) 
    // {
    //     par->error = WTF;
    //     return;
    // }
    token = get_next_token(par);
    if (token && token->type == TT_WORD)
    {
        io = last_cmd_io(data);
        io->file = strdup(token->lexeme);
    }
    else
        par->error = UNEXPECTED_TOKEN;
}


t_cmd_data *parse_cmd_data(t_parser *par)
{
    t_cmd_data *data;

    data = (t_cmd_data *)malloc(sizeof(t_cmd_data));
    if (!data)
        return (par->error = WTF, NULL);
    data->arg = NULL;
    data->io = NULL;
    while (par->current_token && !par->error)
    {
        if (par->current_token->type == TT_WORD)
            push_cmd_arg(par, data);
        else if (get_io_type(par->current_token->type))
            push_cmd_io(par, data);
        else
            break;
        get_next_token(par);
    }
    return (data);
}


// ls -alt > test.text

t_node *parse_cmd_node(t_parser *par)
{
    t_node *node;

    if (!par->current_token)
        return (par->error = UNEXPECTED_EOF, NULL);
    if (par->current_token->type != TT_WORD)
        return (par->error = UNEXPECTED_TOKEN, NULL);
    node = new_node(NT_CMD);
    if (!node)
        return (par->error = WTF, NULL);
    node->data->cmd_data = parse_cmd_data(par);
    return (node);
}


void dump_cmd_node(t_node *node)
{
    t_cmd_arg *arg;
    t_cmd_io *io;

    arg = node->data->cmd_data->arg;
    io = node->data->cmd_data->io;
    printf("|-type: %d\n", node->type);
    // printf("|--data:\n");
    printf("|--args: [");
    while (arg)
    {
        printf("%s", arg->name);
        arg = arg->next;
        if (arg)
            printf(", ");
    }
    printf("]\n");
    printf("|--ios: [");
    while (io)
    {
        printf("{type: %u, file: %s}", io->type, io->file);
        io = io->next;
        if (io)
            printf(", ");
    }
    printf("]\n");
}
t_node *parser_run(t_parser *par)
{
    t_node *left_node;
    t_node *right_node;

    left_node = parse_cmd_node(par);
    // while (1)
    // {
    //     // cmd1 | cmd2
    //     if (par->error || !par->current_token)
    //         break;
    //     // parse_op_node(par)
    //     right_node =  pa  
    // }
    return left_node;
}

t_parser *parser_init(t_lexer *lex)
{
    t_parser *par = (t_parser*)malloc(sizeof(t_parser));
    if (!par)
        return (NULL);
    par->current_token = lex->token_stream->start;
    par->error = NO_SYN_ERR;
    return par;
}

int main()
{
    while (1)
    {
        t_lexer *lex = lexer_init(readline(">> "));
        lexer_run(lex);
        if (lex->error)
            return (printf("lexical error %d", lex->error), 1);
        token_dump(lex);
        t_parser *par = parser_init(lex);
        t_node *node = parser_run(par);
        if (par->error)
            return (printf("parser error %d", par->error), 1);
        dump_cmd_node(node);
        // free_lexer(lex);
    }
    return (0);
}
