#include "parser.h"

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
    get_next_token(par);
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
    token = get_next_token(par);
    if (!token || token->type != TT_WORD)
    {
        par->error = UNEXPECTED_TOKEN;
        return;
    }
    io = last_cmd_io(data);
    io->file = strdup(token->lexeme);
    get_next_token(par);
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
    }
    return (data);
}

t_node *parse_cmd_node(t_parser *par)
{
    t_node *node;

    if (!par->current_token)
        return (par->error = UNEXPECTED_EOF, NULL);
    // NOTE: >> file is valid
    if (!(get_io_type(par->current_token->type)) && par->current_token->type != TT_WORD)
        return (par->error = UNEXPECTED_TOKEN, NULL);
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
void parser_print_err(enum e_syntax_error syn_err, char *token_err)
{
    if (syn_err == UNEXPECTED_EOF)
        printf("syntax error: unexpected end of file\n");
    else if (syn_err == UNEXPECTED_TOKEN)
    {
        if (token_err)
            printf("syntax error near unexpected token `%s'\n", token_err);
        else 
            printf("syntax error near unexpected token `newline'\n");
    }
        // printf("syntax error: unexpected end of file %s", par->current_token->lexeme);
}

t_ast *ast_init()
{
    t_ast *ast;

    ast = (t_ast*)malloc(sizeof(t_ast));
    if (!ast)
        return (NULL);
    ast->root = NULL;
    ast->lex_err = NO_LEX_ERR;
    ast->syn_err = NO_SYN_ERR;
    ast->token_err = NULL;
    return (ast);
}

t_ast *ast_parse(char *line)
{
    t_lexer *lex;
    t_parser *par;
    t_ast *ast;
    
    ast = ast_init();
    lex = lexer_init(line);
    lexer_run(lex);
    if (lex->error)
    {
        ast->lex_err = lex->error;
        lexer_free(lex);
        return ast;
    }
    par = parser_init(lex);
    ast->root = parser_run(par, 0);
    if (par->error)
    {
        ast->syn_err = par->error;
        // TODO: replace strdup with ft
        ast->token_err = par->current_token ? strdup(par->current_token->lexeme) : NULL;
    }
    free(par);
    lexer_free(lex);
    return ast;
}

void ast_print_err(t_ast *ast)
{
    if (ast->lex_err)
        lexer_print_err(ast->lex_err);
    else if (ast->syn_err)
        parser_print_err(ast->syn_err, ast->token_err);
}
void ast_free(t_ast *ast)
{
   node_free(ast->root);
   free(ast->token_err);
   free(ast);
}