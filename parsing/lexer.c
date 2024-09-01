#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>


enum e_quote_state
{
    NONE,
    SINGLE,
    DOUBLE,
};


enum e_token_type
{
    TT_EOF,
    TT_TMP, // this is tmp it needs to be more specific

    TT_REDIRECT_INPUT = '<',
    TT_REDIRECT_OUTPUT = '>',
    TT_LEFT_PAREN = '(',
    TT_RIGHT_PAREN = ')',
    TT_AMPERSAND = '&',
    TT_PIPE = '|', 

    TT_OR, // ||
    TT_AND, // &&
    TT_APPEND_OUTPUT, // >>
    TT_HEREDOC, // << 
    
};

enum e_lexical_error
{
    NO_ERR,
    UNCLOSED_QUOTATION = 12,
};

typedef struct s_token
{
    char *data;
    size_t len;
    enum e_token_type type;
    struct s_token *next;
} t_token;

typedef struct s_token_stream
{
    t_token *start;
    t_token *end;
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

void append_token(t_lexer *lex)
{
    // if (lex->token_stream->end)
    lex->token_stream->end->len++;
}
int push_token(t_lexer *lex, enum e_token_type type, size_t len)
{
    t_token *token = (t_token *)malloc(sizeof(t_token));
    if (!token)
        return (0);
    token->data = lex->line + lex->nparsed;
    token->next = NULL;
    token->type = type;
    token->len = len;
    if (!lex->token_stream->start)
        lex->token_stream->start = token;
    else 
        lex->token_stream->end->next = token;
    lex->token_stream->end = token;
    return (0);
}
void handle_quote(t_lexer *lex)
{
    char c = lex->line[lex->nparsed];

    if (lex->quote_state == NONE)
    {
        if (c == '\'')
            lex->quote_state = SINGLE;
        else if (c == '"')
            lex->quote_state = DOUBLE;
    }
    else if (lex->quote_state == SINGLE && c == '\'')
        lex->quote_state = NONE;
    else if (lex->quote_state == DOUBLE && c == '"')
        lex->quote_state = NONE;
    else if (lex->token_stream->end)
        append_token(lex);
    else
        push_token(lex, TT_TMP, 1) ;
}

void handle_space(t_lexer *lex)
{
    if (lex->quote_state && lex->delim_state)
    {
        push_token(lex, TT_TMP, 1);
        lex->delim_state = 0;
    }
    else if (lex->quote_state)
        append_token(lex);
    else
        lex->delim_state = 1;
}

void handle_seperator(t_lexer *lex)
{
    if (lex->line[lex->nparsed] == TT_PIPE)
    {
        if (lex->line[lex->nparsed + 1] == TT_PIPE)
        {
            push_token(lex, TT_OR, 2);
            lex->nparsed++;
        }
        else 
            push_token(lex, TT_PIPE, 1);
    }
    else if (lex->line[lex->nparsed] == TT_AMPERSAND)
    {
        if (lex->line[lex->nparsed + 1] == TT_AMPERSAND)
        {
            push_token(lex, TT_AND, 2);
            lex->nparsed++;
        }
        else 
            push_token(lex, TT_AMPERSAND, 1);
    }
    else if (lex->line[lex->nparsed] == TT_REDIRECT_INPUT)
    {
        if (lex->line[lex->nparsed + 1] == TT_REDIRECT_INPUT)
        {
            push_token(lex, TT_HEREDOC, 2);
            lex->nparsed++;
        }
        else 
            push_token(lex, TT_REDIRECT_INPUT, 1);
    }
    else if (lex->line[lex->nparsed] == TT_REDIRECT_OUTPUT)
    {
        if (lex->line[lex->nparsed + 1] == TT_REDIRECT_OUTPUT)
        {
            push_token(lex, TT_APPEND_OUTPUT, 2);
            lex->nparsed++;
        }
        else 
            push_token(lex, TT_REDIRECT_OUTPUT, 1);
    }
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
    return (lexer);
}

void handle_tmp(t_lexer *lex)
{
    if (lex->delim_state || !lex->token_stream->end)
        push_token(lex, TT_TMP, 1);
    else 
        append_token(lex);
    lex->delim_state = 0;
}

void lexer_run(t_lexer *lex)
{
    char c;
    size_t line_len;

    // TODO: replace strlen width ft_strlen
    line_len = strlen(lex->line);
    while (!lex->error && lex->nparsed < line_len)
    {
        c = lex->line[lex->nparsed];
        if (ft_is_space(c))
            handle_space(lex);
        else if (ft_is_quote(c))
            handle_quote(lex);
        else if (ft_is_seperator(c) && !lex->quote_state)
            handle_seperator(lex);
        else 
            handle_tmp(lex);
        lex->nparsed++;
    }
    if (lex->quote_state)
        lex->error = UNCLOSED_QUOTATION;
}

void token_dump(t_lexer *lex)
{
    t_token *token = lex->token_stream->start;
    while (token)
    {
        char *t = strndup(token->data, token->len);
        printf("token <%s> | size: %ld\n", t, token->len);
        token = token->next;
        free(t);
    }
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
        //free_lexer(lex);
    }
    return (0);
}
