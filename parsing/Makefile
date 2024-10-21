NAME = lexerparser.a
CC = cc
CFLAGS = -Wall -Wextra -Werror
CJSON = -lcjson
AR = ar rcs

HDRS = lexer.h parser.h
SRCS = lexer.c parser.c

OBJS := $(SRCS:.c=.o)

%.o: %.c $(HDRS)
	$(CC) -c -o $@ $< $(CJSON)

$(NAME): $(OBJS)
	$(AR) $@ $^

all : $(NAME)



clean :
	rm -f $(OBJS)

fclean:	clean
	rm -f $(NAME)