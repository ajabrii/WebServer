CXX       = c++
CPPFLAGS  = -Wall -Wextra -Werror -std=c++98 #-fsanitize=address -g3

NAME      = Webserv

SRCDIR    = src
INCDIR    = includes
OBJDIR    = obj

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
	$(SRCDIR)/CookieParser.cpp \
	$(SRCDIR)/SessionID.cpp \
	$(SRCDIR)/CgiState.cpp \
	$(SRCDIR)/SessionManager.cpp \
	$(SRCDIR)/ConfigServerParser.cpp \
	$(SRCDIR)/ResponseBuilder.cpp \
	$(SRCDIR)/SessionInfos.cpp


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
	$(INCDIR)/ResponseBuilder.hpp \
	$(INCDIR)/SessionID.hpp \
	$(INCDIR)/CgiState.hpp \
	$(INCDIR)/SessionManager.hpp\
	$(INCDIR)/SessionInfos.hpp

OBJ = $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ) $(HEADERS)
	@echo "\033[1;32mLinking $(NAME)...\033[0m"
	@$(CXX) $(CPPFLAGS) $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	@echo "\033[1;34mCompiling $<...\033[0m"
	@$(CXX) $(CPPFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	@echo "\033[1;33mCleaning object files...\033[0m"
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "\033[1;31mCleaning binary...\033[0m"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
