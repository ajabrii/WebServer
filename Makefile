CXX = c++
CPPFLAGS = -Wall -Wextra -Werror  -std=c++98 #-fsanitize=address -g3 #-std=c++98
NAME = Webserv
SRC =   src/*.cpp
OBJ = $(SRC:.cpp=.o)
HEADER = includes/*.hpp

all: $(NAME)

$(NAME): $(OBJ) $(HEADER)
	@$(CXX) $(CPPFLAGS) $(SRC) -o $(NAME)
clean: $(OBJ)
	@rm -rf $(OBJ)
fclean: clean
	@rm -rf $(NAME)
re: fclean all