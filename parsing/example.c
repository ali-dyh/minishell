#include "parser.h";

int main ()
{
    while (1)
    {
        char *line = readline(">> ");
        t_ast *ast = ast_parse(line);
        if (ast->lex_err || ast->syn_err)
        

        ast_free(ast);
    }
}