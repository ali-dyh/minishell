#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <cjson/cJSON.h>

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
        // if (!lex->token_stream->end || !lex->token_stream->end->len)
        if (lex->quote_state == lex->line[lex->nparsed - 1])
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

void handle_expand(t_lexer *lex)
{
    size_t env_len;
    char *env_str;
    char *env_val;
    
    env_len = 0;
    lex->nparsed++;
    while (lex->line[lex->nparsed + env_len])
    {
        if (ft_is_quote(lex->line[lex->nparsed + env_len]))
            break;
        if (ft_is_seperator(lex->line[lex->nparsed + env_len]))
            break;
        if (ft_is_space(lex->line[lex->nparsed + env_len]))
            break;
        env_len++;
    }
    env_str = strndup(lex->line + lex->nparsed, env_len);
    env_val = getenv(env_str);
    free(env_str);
    if (env_val)
    {
        if (lex->delim_state || !lex->token_stream->end)
        {
            t_token *token = (t_token *)malloc(sizeof(t_token));
            /// TODO: handle the malloc failure
            // if (!token)
            // TODO: replace strdup with ft_strdup
            token->lexeme = strdup(env_val);
            token->next = NULL;
            token->type = TT_WORD;
            token->len = strlen(env_val);
            if (!lex->token_stream->start)
                lex->token_stream->start = token;
            else
                lex->token_stream->end->next = token;
            lex->token_stream->end = token;
            lex->token_stream->count++;
        }
        else {
            // TODO: replace realloc and strlen with ft_
            lex->token_stream->end->lexeme = realloc(lex->token_stream->end->lexeme, lex->token_stream->end->len + strlen(env_val) + 1);
            strcat(lex->token_stream->end->lexeme, env_val);
            lex->token_stream->end->len += strlen(env_val);
        }
        lex->delim_state = 0;
    }
    lex->nparsed += env_len;
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
        if (c == '$' && lex->quote_state != SINGLE)
            handle_expand(lex);
        else if (ft_is_quote(c) && (!lex->quote_state || c == lex->quote_state))
            handle_quote(lex);
        else if (ft_is_space(c) && !lex->quote_state)
            handle_space(lex);
        else if (ft_is_seperator(c) && !lex->quote_state)
            handle_seperator(lex);
        else
            handle_word(lex);
    }
    if (lex->quote_state == SINGLE)
        lex->error = UNCLOSED_SINGLE_QUOTATION;
    if (lex->quote_state == DOUBLE)
        lex->error = UNCLOSED_DOUBLE_QUOTATION;
}

char *print_token_type(enum e_token_type token_type)
{
    switch (token_type)
    {
    case TT_REDIRECT_INPUT:
        return ("REDIRECT_INPUT");
    case TT_REDIRECT_OUTPUT:
        return ("REDIRECT_OUTPUT");
    case TT_LEFT_PAREN:
        return ("LEFT_PAREN");
    case TT_RIGHT_PAREN:
        return ("RIGHT_PAREN");
    case TT_AMPERSAND:
        return ("AMPERSAND");
    case TT_OR:
        return ("OR");
    case TT_PIPE:
        return ("PIPE");
    case TT_AND:
        return ("AND");
    case TT_APPEND_OUTPUT:
        return ("APPEND_OUTPUT");
    case TT_HEREDOC:
        return ("HEREDOC");
    default:
        return ("WORD");
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

cJSON *lex_to_json(t_lexer *lex)
{
    cJSON *lexObj = cJSON_CreateObject();
    cJSON *tokenObj;
    cJSON *tokenArr = cJSON_CreateArray();

    cJSON_AddNumberToObject(lexObj, "number_of_tokens", lex->token_stream->count);
    t_token *token = lex->token_stream->start;
    while (token)
    {
        tokenObj = cJSON_CreateObject();
        cJSON_AddStringToObject(tokenObj, "token", token->lexeme);
        cJSON_AddStringToObject(tokenObj, "token_type", print_token_type(token->type));
        cJSON_AddNumberToObject(tokenObj, "token_length", token->len);
        cJSON_AddNumberToObject(tokenObj, "token_strlen", strlen(token->lexeme));
        cJSON_AddItemToArray(tokenArr, tokenObj);
        token = token->next;
    }
    cJSON_AddItemToObject(lexObj, "token_list", tokenArr);
    return lexObj;
}

void lexer_free(t_lexer *lex)
{
    t_token *token;
    t_token *token_tmp;

    token = lex->token_stream->start;
    while (token)
    {
        token_tmp = token;
        token = token->next;
        free(token_tmp->lexeme);
        free(token_tmp);
    }
    free(lex->token_stream);
    free(lex->line);
    free(lex);
}

void lexer_print_err(enum e_lexical_error lex_err)
{
    if (lex_err == UNCLOSED_SINGLE_QUOTATION)
        printf("unexpected EOF while looking for matching `''\n");
    if (lex_err == UNCLOSED_DOUBLE_QUOTATION)
        printf("unexpected EOF while looking for matching `\"'\n");
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

    arg = (t_cmd_arg *)malloc(sizeof(t_cmd_arg));
    if (!arg)
        return (NULL);
    // TODO: replace strdup with ft_strdup and malloc check
    arg->name = strdup(arg_name);
    arg->next = NULL;
    return arg;
}
t_cmd_io *new_cmd_io(enum e_io_type io_type)
{
    t_cmd_io *io;

    io = (t_cmd_io *)malloc(sizeof(t_cmd_io));
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
    // if (!data->arg || !arg->next)
    //     par->error = WTF;
}

enum e_io_type get_io_type(enum e_token_type ttype)
{
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

t_node *parse_cmd_node(t_parser *par)
{
    t_node *node;

    if (!par->current_token)
        return (par->error = UNEXPECTED_EOF, NULL);
    // NOTE: >> file is valid
    // if (par->current_token->type != TT_WORD)
    //     return (par->error = UNEXPECTED_TOKEN, NULL);
    node = new_node(NT_CMD);
    if (!node)
        return (par->error = WTF, NULL);
    node->data->cmd_data = parse_cmd_data(par);
    return (node);
}

enum e_op_type get_op_type(t_token *token)
{
    if (token->type == TT_AND)
        return (OT_AND);
    if (token->type == TT_OR)
        return (OT_OR);
    if (token->type == TT_PIPE)
        return (OT_PIPE);
    return (0);
}

t_node *new_op_node(enum e_op_type op_type, t_node *left_node, t_node *right_node)
{
    t_node *node = new_node(NT_OP);
    if (!node)
        return (NULL);
    node->data->op_data = (t_op_data *)malloc(sizeof(t_op_data));
    if (!node->data->op_data)
        return (NULL);
    node->data->op_data->type = op_type;
    node->data->op_data->left = left_node;
    node->data->op_data->right = right_node;
    return (node);
}

t_node *parser_run(t_parser *par, int op_prec)
{
    t_node *left_node;
    enum e_op_type op_type;

    if (par->current_token && par->current_token->type == TT_LEFT_PAREN)
    {
        get_next_token(par);
        left_node = parser_run(par, 0);
        if (!par->current_token || par->current_token->type != TT_RIGHT_PAREN)
            return (par->error = UNEXPECTED_EOF, left_node);
        get_next_token(par);
    }
    else 
        left_node = parse_cmd_node(par);
    while (!par->error && par->current_token)
    {
        if (par->current_token->type == TT_RIGHT_PAREN)
            break;
        op_type = get_op_type(par->current_token);
        // if the next token is not an specified op then what ?
        // TODO: this needs to be an error
        if (!op_type)
            return (par->error = UNEXPECTED_TOKEN, left_node);
            // par->error = UNEXPECTED_TOKEN;
        if (!(op_type >= op_prec))
            break;
        get_next_token(par);
        left_node = new_op_node(op_type, left_node, parser_run(par, op_type + 1));
    }
    return left_node;
}

t_parser *parser_init(t_lexer *lex)
{
    t_parser *par = (t_parser *)malloc(sizeof(t_parser));
    if (!par)
        return (NULL);
    par->current_token = lex->token_stream->start;
    par->error = NO_SYN_ERR;
    return par;
}

// void dump_cmd_node(t_node *node)
// {
//     t_cmd_arg *arg;
//     t_cmd_io *io;

//     arg = node->data->cmd_data->arg;
//     io = node->data->cmd_data->io;
//     printf("|-type: %d\n", node->type);
//     // printf("|--data:\n");
//     printf("|--args: [");
//     while (arg)
//     {
//         printf("%s", arg->name);
//         arg = arg->next;
//         if (arg)
//             printf(", ");
//     }
//     printf("]\n");
//     printf("|--ios: [");
//     while (io)
//     {
//         printf("{type: %u, file: %s}", io->type, io->file);
//         io = io->next;
//         if (io)
//             printf(", ");
//     }
//     printf("]\n");
// }

char *print_node_type(enum e_node_type node_type)
{
    switch (node_type)
    {
    case NT_CMD:
        return "CMD";
    default:
        return "OP";
    }
}

char *print_io_type(enum e_io_type io_type)
{
    switch (io_type)
    {
    case IO_REDIRECT_INPUT:
        return "REDIRECT INPUT";
    case IO_REDIRECT_OUTPUT:
        return "REDIRECT OUTPUT";
    case IO_APPEND_OUTPUT:
        return "APPEND OUTPUT";
    default:
        return "HEREDOC";
    }
}

char *print_op_type(enum e_op_type op_type)
{
    switch (op_type)
    {
    case OT_AND:
        return "AND";
    case OT_OR:
        return "OR";
    default:
        return "PIPE";
    }
}

cJSON *cmd_to_json(t_node *node)
{
    t_cmd_arg *arg = node->data->cmd_data->arg;
    t_cmd_io *io = node->data->cmd_data->io;
    cJSON *nodeObj = cJSON_CreateObject();
    cJSON *argArr = cJSON_CreateArray();
    cJSON *ioArr = cJSON_CreateArray();
    cJSON *ioObj;
    cJSON_AddStringToObject(nodeObj, "node_type", print_node_type(node->type));
    while (arg || io)
    {
        if (arg)
        {
            cJSON_AddItemToArray(argArr, cJSON_CreateString(arg->name));
            arg = arg->next;
        }
        if (io)
        {
            ioObj = cJSON_CreateObject();
            cJSON_AddStringToObject(ioObj, "io_type", print_io_type(io->type));
            cJSON_AddStringToObject(ioObj, "file_name", io->file);
            cJSON_AddItemToArray(ioArr, ioObj);
            io = io->next;
        }
    }
    cJSON_AddItemToObject(nodeObj, "args", argArr);
    cJSON_AddItemToObject(nodeObj, "ios", ioArr);
    return nodeObj;
}

cJSON *ast_to_json(t_node *node)
{
    if (node->type == NT_OP)
    {
        cJSON *nodeObj = cJSON_CreateObject();
        cJSON_AddStringToObject(nodeObj, "node_type", print_node_type(node->type));
        cJSON_AddStringToObject(nodeObj, "op_type", print_op_type(node->data->op_data->type));
        cJSON_AddItemToObject(nodeObj, "left_node", ast_to_json(node->data->op_data->left));
        cJSON_AddItemToObject(nodeObj, "right_node", ast_to_json(node->data->op_data->right));
        return nodeObj;
    }
    return cmd_to_json(node);
}



// void par_print_err()

void node_cmd_data_free(t_cmd_data *cmd_data)
{
    t_cmd_arg *arg;
    t_cmd_io *io;
    t_cmd_arg *arg_tmp;
    t_cmd_io *io_tmp;

    arg = cmd_data->arg;
    io = cmd_data->io;
    while (arg || io)
    {
        if (arg)
        {
           arg_tmp = arg; 
           arg = arg->next;
           free(arg_tmp->name);
           free(arg_tmp);
        }
        if (io)
        {
            io_tmp = io;
            io = io->next;
            free(io_tmp->file);
            free(io_tmp);
        }
    }
    free(cmd_data);
}


void node_cmd_free(t_node *node)
{
    node_cmd_data_free(node->data->cmd_data);
    free(node->data);
    free(node);
}


void node_free(t_node *node)
{
    if (!node)
        return;
    if (node->type == NT_OP)
    {
        node_free(node->data->op_data->right);
        node_free(node->data->op_data->left);
        free(node->data->op_data);
        free(node->data);
        free(node);
    }
    else 
        node_cmd_free(node);
}
void parser_print_err(t_parser *par)
{
    if (par->error == UNEXPECTED_EOF)
        printf("syntax error: unexpected end of file\n");
    else if (par->error == UNEXPECTED_TOKEN)
        printf("syntax error near unexpected token `%s'\n",par->current_token->lexeme);
        // printf("syntax error: unexpected end of file %s", par->current_token->lexeme);
}
int main()
{
    while (1)
    {
        char *line = readline(">> ");
        t_lexer *lex = lexer_init(line);
        lexer_run(lex);
        if (lex->error)
        {
            lexer_print_err(lex->error);
            lexer_free(lex);
            continue;
            // exit(1);
        }
        cJSON *lexJSON = lex_to_json(lex);
        char *lexStr = cJSON_Print(lexJSON);
        printf("%s\n", lexStr);
        cJSON_free(lexStr);
        cJSON_Delete(lexJSON);
        lexer_free(lex);

        // t_parser *par = parser_init(lex);
        // t_node *node = parser_run(par, 0);
        // if (par->error)
        // {
        //     parser_print_err(par);
        //     free(par);
        //     node_free(node);
        //     lexer_free(lex);
        //     continue;
        //     // exit(1);
        // }
        // // dump_cmd_node(node);
        // cJSON *ast = ast_to_json(node);
        // char *jsonStr = cJSON_Print(ast);
        // printf("%s\n", jsonStr);
        // cJSON_free(jsonStr);
        // cJSON_Delete(ast);
        // free(par);
        // node_free(node);
        // lexer_free(lex);
    }
    return (0);
}
