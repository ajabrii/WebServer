# webserv

```
webserv/
├── Makefile
├── config/
│   └── default.conf                # Sample configuration file
├── includes/
│   ├── HttpServer.hpp (done)
│   ├── Reactor.hpp (done)
│   ├── Connection.hpp (done)
│   ├── RequestDispatcher.hpp (done)
│   ├── Router.hpp (done)
│   ├── Controller.hpp (no need)
│   ├── IHttpMethodHandler.hpp (done)
│   ├── GetHandler.hpp (done)
│   ├── PostHandler.hpp (done)
│   ├── DeleteHandler.hpp (done)
│   ├── ConfigInterpreter.hpp (done)
│   ├── ServerConfig.hpp (done)
│   ├── RouteConfig.hpp (done)
│   ├── HttpRequest.hpp (done)
│   ├── HttpResponse.hpp (done)
│   ├── HttpResponseBuilder.hpp
│   ├── Logger.hpp (not now)
│   └── utils.hpp (no need)
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
```

## Layers & Classes

| Layer / Class                                | Role (in your design)                                                                               |
| -------------------------------------------- | --------------------------------------------------------------------------------------------------- |
| **ConfigInterpreter**                        | Reads & parses the config file (e.g. config.conf). Stores server blocks & route configs in memory.  |
| **ServerConfig & RouteConfig**               | Data structures holding parsed config: host, port, routes, allowed methods, etc.                    |
| **HttpServer**                               | Represents a single server (listening socket). Knows its config. Accepts new connections.           |
| **Reactor**                                  | Manages all active fds (poll). Detects new connections & client events. Calls accept or read/write. |
| **Connection**                               | Represents a single client TCP connection. Owns client fd. Reads data, writes response.             |
| **Router**                                   | Given request & server config → finds matching RouteConfig (by path & method).                      |
| **RequestDispatcher**                        | Uses Router result & request method → dispatches to right handler (GET, POST, DELETE).              |
| **GetHandler / PostHandler / DeleteHandler** | Implement logic: serve file, delete file, handle upload, etc.                                       |
| **HttpRequest**                              | Parses raw HTTP request: method, uri, headers, body.                                                |
| **HttpResponse**                             | Holds status, headers, body to send back.                                                           |
| **HttpResponseBuilder**                      | Formats final HTTP/1.1 string to send.                                                              |
| **Logger**                                   | Logs access / errors (not yet added).                                                               |

## Architecture Overview

```
+-------------------------------------------------------------+
|                    ConfigInterpreter                        |
|        | parses config.conf into configs + routes           |
+-------------------------------------------------------------+
                |
                v
+-------------------------------------------------------------+
|                    ServerConfig + RouteConfig               |
|       (hold host, port, routes, allowed methods, etc)       |
+-------------------------------------------------------------+
                |
                v
+-------------------------------------------------------------+
|                        HttpServer                           |
|  - owns listening socket (fd)                               |
|  - uses config                                              |
|  - accepts new connections                                  |
+-------------------------------------------------------------+
                |
                v
+-------------------------------------------------------------+
|                          Reactor                            |
|  - uses poll() to multiplex all sockets                     |
|  - detects new connections & data ready to read             |
+-------------------------------------------------------------+
                |
                v
+-------------------------------------------------------------+
|                        Connection                           |
|  - wraps single client fd                                   |
|  - reads raw data & sends response                          |
+-------------------------------------------------------------+
                |
                v
+-------------------------------------------------------------+
|                          Router                             |
|  - matches request URI & method against RouteConfig         |
+-------------------------------------------------------------+
                |
                v
+-------------------------------------------------------------+
|                   RequestDispatcher                         |
|  - calls correct handler based on method                    |
+--------------------+--------------+------------------------+
                     |              |
              +-------------+  +-------------+   +-------------+
              |  GetHandler |  | PostHandler |   | DeleteHandler|
              +-------------+  +-------------+   +-------------+
                     |
                     v
+-------------------------------------------------------------+
|                    HttpResponseBuilder                      |
|  - builds final HTTP/1.1 text response                      |
+-------------------------------------------------------------+
                     |
                     v
+-------------------------------------------------------------+
|                        HttpResponse                         |
|  - holds version, status, headers, body                     |
+-------------------------------------------------------------+
```
