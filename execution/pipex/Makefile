NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror #-fsanitize=address -g3 
RM = rm -rf

MANDATORY = ft_split.c multipipe.c pipex.c
			
MANDATORY_O = ${MANDATORY:.c=.o}

${NAME}: ${MANDATORY_O}
		${CC} ${MANDATORY_O}  -o ${NAME} -lreadline

all:${NAME}

clean: 
		${RM} ${MANDATORY_O}
fclean:
		${RM} ${MANDATORY_O} ${NAME}

re: fclean all