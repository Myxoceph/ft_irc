NAME			=	ft_irc

CC				=	c++
CFLAGS			=	-std=c++98 #-Wall -Werror -Wextra

RM				=	rm -rf

SRCS_DIR		=	./srcs/
OBJS_DIR		=	./objs/
INCLUDES_DIR	=	./inc/

SRCS 			=	$(SRCS_DIR)main.cpp $(SRCS_DIR)Channel.cpp $(SRCS_DIR)Client.cpp $(SRCS_DIR)Server.cpp

OBJS			=	$(patsubst $(SRCS_DIR)%.cpp, $(OBJS_DIR)%.o, $(SRCS))

INCLUDES		=	-I$(INCLUDES_DIR)

COLOR_YELLOW	=	\033[0;33m
COLOR_GREEN		=	\033[0;32m
COLOR_RED		=	\033[0;31m
COLOR_END		=	\033[0m

all				:	$(NAME)

$(NAME)			:	$(OBJS)
					@$(CC) $(CFLAGS) $^ -o $(NAME)
					@echo "\n\e[1m$(COLOR_YELLOW)$(NAME)		$(COLOR_GREEN)[is ready!]\e[0m\n$(COLOR_END)"

$(OBJS_DIR)%.o	:	$(SRCS_DIR)%.cpp
					@mkdir -p $(@D)
					@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean			:
					@$(RM) -rf $(OBJS_DIR)
					@echo "\e[1m$(COLOR_YELLOW)objects		$(COLOR_RED)[are deleted!]\e[0m$(COLOR_END)"
			

fclean			:	clean
					@$(RM) $(NAME)
					@echo "\e[1m$(COLOR_YELLOW)$(NAME)		$(COLOR_RED)[is deleted!]\e[0m$(COLOR_END)"

re				:	fclean all

.PHONY			:	all clean fclean re
