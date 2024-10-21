#include "parser.h"

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

void parser_print_err2(t_parser *par)
{
    if (par->error == UNEXPECTED_EOF)
        printf("syntax error: unexpected end of file\n");
    else if (par->error == UNEXPECTED_TOKEN)
    {
        if (par->current_token)
            printf("syntax error near unexpected token `%s'\n", par->current_token->lexeme);
        else 
            printf("syntax error near unexpected token `newline'\n");
    }
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
        // cJSON *lexJSON = lex_to_json(lex);
        // char *lexStr = cJSON_Print(lexJSON);
        // printf("%s\n", lexStr);
        // cJSON_free(lexStr);
        // cJSON_Delete(lexJSON);
        // lexer_free(lex);

        t_parser *par = parser_init(lex);
        t_node *node = parser_run(par, 0);
        if (par->error)
        {
            parser_print_err2(par);
            free(par);
            node_free(node);
            lexer_free(lex);
            continue;
            // exit(1);
        }
        // // dump_cmd_node(node);
        cJSON *ast = ast_to_json(node);
        char *jsonStr = cJSON_Print(ast);
        printf("%s\n", jsonStr);
        cJSON_free(jsonStr);
        cJSON_Delete(ast);
        free(par);
        node_free(node);
        lexer_free(lex);
    }
    return (0);
}