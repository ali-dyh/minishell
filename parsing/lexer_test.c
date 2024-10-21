#include "lexer.h"


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
    }
}