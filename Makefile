# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sjoao <sjoao@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/09/15 00:00:00 by sjoao            #+#    #+#              #
#    Updated: 2026/09/15 00:00:00 by sjoao           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

RESET = \033[0m
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
CYAN = \033[0;36m

NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes

SRCS = \
	srcs/main.cpp \
	srcs/Server.cpp \
	srcs/Client.cpp \
	srcs/Channel.cpp \
	srcs/commands_files/Registration.cpp \
	srcs/commands_files/ChannelCommands.cpp \
	srcs/commands_files/OperatorCommands.cpp \
	srcs/commands_files/Messaging.cpp

OBJDIR = obj
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	@printf "$(GREEN)Linking $(NAME)...$(RESET)\n"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@printf "$(GREEN)$(NAME) ready.$(RESET)\n"

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@printf "$(BLUE)Compiling $<$(RESET)\n"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJDIR)
	@printf "$(YELLOW)Object files removed.$(RESET)\n"

fclean: clean
	@rm -f $(NAME)
	@printf "$(RED)Binary removed.$(RESET)\n"

re: fclean all

.PHONY: all clean fclean re
