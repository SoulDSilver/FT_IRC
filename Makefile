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
HEADERS = $(addprefix includes/, Channel.hpp Client.hpp Server.hpp \
			irc.hpp Commands.hpp)
SRCS =  $(addprefix srcs/, main.cpp Commands.cpp Channel.cpp Client.cpp \
			Server.cpp commands_files/Registration.cpp \
			commands_files/ChannelCommands.cpp \
			commands_files/OperatorCommands.cpp \
			commands_files/Messaging.cpp )
OBJDIR = objs
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes
CXX = c++

all: $(NAME)
$(NAME): $(OBJS) $(HEADERS)
	@printf "$(GREEN)Linking $(NAME)...$(RESET)\n"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $@
	@printf "$(GREEN)$(NAME) ready.$(RESET)\n"

$(OBJDIR)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	@printf "$(BLUE)Compiling $<$(RESET)\n"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJDIR)
	@printf "$(YELLOW)Object files removed.$(RESET)\n"

fclean: clean
	@rm -f $(NAME)
	@printf "$(RED)Binary removed.$(RESET)\n"

re: fclean all

.PHONY: all clean fclean re 
