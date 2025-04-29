CXX = c++
CPPFLAGS = -Wall -Wextra -Werror
NAME = Webserv
SRCS = main.cpp Webserver.cpp confige_parser.cpp
OBJ = $(SRCS:.cpp=.o)
HEADER = Webserver.hpp

all: $(NAME)
$(NAME): $(OBJ)	$(HEADER)
	@$(CXX) $(CPPFLAGS) $(SRCS) -o $(NAME)

clean:
	@rm -rf $(OBJ)

fclean: clean
	@rm -rf $(NAME)

re: fclean all