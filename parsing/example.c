#include "parser.h"

int main()
{
    // while (1)
    // {
    char *line = readline(">> ");
    t_ast *ast = ast_parse(line);
    if (ast->lex_err || ast->syn_err)
    {
        ast_print_err(ast);
        ast_free(ast);
        exit(1);
        // continue;
    }
    ast_dump(ast);
    ast_free(ast);
    // }
}