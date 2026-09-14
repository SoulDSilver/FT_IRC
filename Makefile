NAME = ircserv
HEADERS = $(addprefix includes/, irc.hpp)
SRCS =  $(addprefix srcs/, main.cpp )
OBJS = $(SRCS:.cpp=.o)
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
CXX = c++

all: $(NAME)
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@
$(OBJS): $(HEADERS)
clean:
	rm -rf $(OBJS)
fclean: clean
	rm -rf $(NAME)
re: fclean all

.PHONY: all clean fclean re 
