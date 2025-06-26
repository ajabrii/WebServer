webserv/
├── Makefile
├── config/
│   └── default.conf                # Sample configuration file
├── includes/
│   ├── HttpServer.hpp
│   ├── Reactor.hpp
│   ├── Connection.hpp
│   ├── RequestDispatcher.hpp
│   ├── Router.hpp
│   ├── Controller.hpp
│   ├── IHttpMethodHandler.hpp
│   ├── GetHandler.hpp
│   ├── PostHandler.hpp
│   ├── DeleteHandler.hpp
│   ├── ConfigInterpreter.hpp
│   ├── ServerConfig.hpp
│   ├── RouteConfig.hpp
│   ├── HttpRequest.hpp
│   ├── HttpResponse.hpp
│   ├── HttpResponseBuilder.hpp
│   ├── Logger.hpp
│   └── utils.hpp
├── srcs/
│   ├── main.cpp
│   ├── HttpServer.cpp
│   ├── Reactor.cpp
│   ├── Connection.cpp
│   ├── RequestDispatcher.cpp
│   ├── Router.cpp
│   ├── Controller.cpp
│   ├── GetHandler.cpp
│   ├── PostHandler.cpp
│   ├── DeleteHandler.cpp
│   ├── ConfigInterpreter.cpp
│   ├── ServerConfig.cpp
│   ├── RouteConfig.cpp
│   ├── HttpRequest.cpp
│   ├── HttpResponse.cpp
│   ├── HttpResponseBuilder.cpp
│   ├── Logger.cpp
│   └── utils.cpp
├── cgi-bin/
│   └── test_cgi.py                 # Test CGI scripts
├── www/
│   └── index.html                  # Static site content
└── README.md
