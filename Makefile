CXX = c++
CPPFLAGS = -Wall -Wextra -Werror #-std=c++98
NAME = Webserv
SRC =   main.cpp Webserver.cpp config_parser.cpp
OBJ = $(SRC:.cpp=.o)
HEADER = Webserver.hpp

all: $(NAME)

$(NAME): $(OBJ) $(HEADER)
	@$(CXX) $(CPPFLAGS) $(SRC) -o $(NAME)
clean: $(OBJ)
	@rm -rf $(OBJ)
fclean: clean
	@rm -rf $(NAME)
re: fclean all