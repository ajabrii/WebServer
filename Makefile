# Compiler and flags
CXX       = c++
CPPFLAGS  = -Wall -Wextra -Werror -std=c++98 #-fsanitize=address -g3
# Uncomment for debugging/sanitizers
# CPPFLAGS += -fsanitize=address -g3

# Project name
NAME      = Webserv

# Directories
SRCDIR    = src
INCDIR    = includes
OBJDIR    = obj

# Source files
SRC = \
	$(SRCDIR)/main.cpp \
	$(SRCDIR)/CgiHandler.cpp \
	$(SRCDIR)/CgiData.cpp \
	$(SRCDIR)/ConfigInterpreter.cpp \
	$(SRCDIR)/Connection.cpp \
	$(SRCDIR)/HttpRequest.cpp \
	$(SRCDIR)/HttpResponse.cpp \
	$(SRCDIR)/DeleteHandler.cpp \
	$(SRCDIR)/PostHandler.cpp \
	$(SRCDIR)/GetHandler.cpp \
	$(SRCDIR)/Errors.cpp \
	$(SRCDIR)/HttpServer.cpp \
	$(SRCDIR)/Reactor.cpp \
	$(SRCDIR)/Router.cpp \
	$(SRCDIR)/RequestDispatcher.cpp \
	$(SRCDIR)/RouteConfig.cpp \
	$(SRCDIR)/ServerConfig.cpp \
	$(SRCDIR)/Utils.cpp \
	$(SRCDIR)/ResponseBuilder.cpp

HEADERS = \
	$(INCDIR)/CgiHandler.hpp \
	$(INCDIR)/CgiData.hpp \
	$(INCDIR)/ConfigInterpreter.hpp \
	$(INCDIR)/Connection.hpp \
	$(INCDIR)/HttpRequest.hpp \
	$(INCDIR)/HttpResponse.hpp \
	$(INCDIR)/DeleteHandler.hpp \
	$(INCDIR)/PostHandler.hpp \
	$(INCDIR)/GetHandler.hpp \
	$(INCDIR)/Errors.hpp \
	$(INCDIR)/HttpServer.hpp \
	$(INCDIR)/Reactor.hpp \
	$(INCDIR)/Router.hpp \
	$(INCDIR)/RequestDispatcher.hpp \
	$(INCDIR)/RouteConfig.hpp \
	$(INCDIR)/ServerConfig.hpp \
	$(INCDIR)/Utils.hpp \
	$(INCDIR)/ResponseBuilder.hpp
# Object files
OBJ = $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Default target
all: $(NAME)

# Link objects into binary
$(NAME): $(OBJ) $(HEADERS)
	@echo "\033[1;32mLinking $(NAME)...\033[0m"
	@$(CXX) $(CPPFLAGS) $(OBJ) -o $(NAME)

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	@echo "\033[1;34mCompiling $<...\033[0m"
	@$(CXX) $(CPPFLAGS) -I$(INCDIR) -c $< -o $@

# Clean object files
clean:
	@echo "\033[1;33mCleaning object files...\033[0m"
	@rm -rf $(OBJDIR)

# Clean all (object files + binary)
fclean: clean
	@echo "\033[1;31mCleaning binary...\033[0m"
	@rm -f $(NAME)

# Rebuild
re: fclean all

.PHONY: all clean fclean re
