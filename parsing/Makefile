NAME = lexerparser.a
CC = cc
CFLAGS = -Wall -Wextra -Werror
TESTFLAGS = -lcjson -lreadline
AR = ar rcs

HDRS = lexer.h parser.h
SRCS = lexer.c parser.c

OBJS := $(SRCS:.c=.o)

%.o: %.c $(HDRS)
	$(CC) -c -o $@ $< $(CJSON)

$(NAME): $(OBJS)
	$(AR) $@ $^

all : $(NAME)

test : $(NAME)
	$(CC) lexer_test.c $< $(TESTFLAGS) -o lex
	$(CC) parser_test.c $< $(TESTFLAGS) -o par 

tclean : 
	rm -f lex par

clean :
	rm -f $(OBJS)

fclean:	clean tclean
	rm -f $(NAME)