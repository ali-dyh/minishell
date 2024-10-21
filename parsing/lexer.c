#include "lexer.h"

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
    lexer->error = NO_LEX_ERR;
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
        // if (ft_is_quote(lex->line[lex->nparsed + env_len]))
        //     break;
        // if (ft_is_seperator(lex->line[lex->nparsed + env_len]))
        //     break;
        // if (ft_is_space(lex->line[lex->nparsed + env_len]))
        //     break;
        if (lex->line[lex->nparsed + env_len] == '_')
        {
            env_len++;
            continue;
        }
        if (!isalnum(lex->line[lex->nparsed + env_len]))
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