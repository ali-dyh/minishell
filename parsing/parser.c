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
    {
        if (par->current_token)
            printf("syntax error near unexpected token `%s'\n",par->current_token->lexeme);
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
    ast = parser_run(par, 0);
    if (par->error)
    {
        ast->syn_err = par->error;
        // ast->tok = par->current_token;
        lexer_free(lex);
        free(par);
        return ast;
    }
    lexer_free(lex);
    return ast;
}

void ast_print_err(t_ast *ast)
{
    if (ast->lex_err)
        lexer_print_err(ast->lex_err);
    else if (ast->syn_err)
        parser_print_err()
}
void ast_free(t_ast *ast)
{
   node_free(ast->root);
   free(ast);
}

// int main()
// {
//     while (1)
//     {
//         char *line = readline(">> ");
//         t_lexer *lex = lexer_init(line);
//         lexer_run(lex);
//         if (lex->error)
//         {
//             lexer_print_err(lex->error);
//             lexer_free(lex);
//             continue;
//             // exit(1);
//         }
//         cJSON *lexJSON = lex_to_json(lex);
//         char *lexStr = cJSON_Print(lexJSON);
//         printf("%s\n", lexStr);
//         cJSON_free(lexStr);
//         cJSON_Delete(lexJSON);
//         lexer_free(lex);

//         t_parser *par = parser_init(lex);
//         t_node *node = parser_run(par, 0);
//         if (par->error)
//         {
//             parser_print_err(par);
//             // printf("%d\n", par->error);
//             free(par);
//             node_free(node);
//             lexer_free(lex);
//             continue;
//             // exit(1);
//         }
//         // dump_cmd_node(node);
//         cJSON *ast = ast_to_json(node);
//         char *jsonStr = cJSON_Print(ast);
//         printf("%s\n", jsonStr);
//         cJSON_free(jsonStr);
//         cJSON_Delete(ast);
//         free(par);
//         node_free(node);
//         lexer_free(lex);
//     }
//     return (0);
// }
