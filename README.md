# 🚀 NexWebserv - High-Performance HTTP Server

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/your-username/nexWebserv)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C++98-orange.svg)](https://en.cppreference.com/w/cpp/98)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://www.linux.org/)

A modern, high-performance HTTP/1.1 server implementation written in C++98. Built with event-driven architecture using the Reactor pattern for handling multiple concurrent connections efficiently.

## 📋 Table of Contents

- [Features](#-features)
- [Architecture](#-architecture)
- [Installation](#-installation)
- [Usage](#-usage)
- [Configuration](#-configuration)
- [API Documentation](#-api-documentation)
- [Testing](#-testing)
- [Contributing](#-contributing)
- [License](#-license)

## ✨ Features

### Core HTTP Features
- **HTTP/1.1 Compliance**: Full support for HTTP/1.1 protocol
- **Multiple HTTP Methods**: GET, POST, DELETE operations
- **Static File Serving**: Efficient static content delivery
- **Directory Listing**: Configurable directory browsing
- **Custom Error Pages**: Personalized error page support
- **File Upload/Download**: Handle file operations with size limits
- **URL Redirection**: Flexible redirect configuration

### Performance & Scalability
- **Event-Driven Architecture**: Non-blocking I/O using poll()
- **Reactor Pattern**: Efficient handling of multiple concurrent connections
- **Memory Management**: Optimized memory usage with proper cleanup
- **Connection Pooling**: Reuse of connections for better performance

### Security & Configuration
- **Request Size Limits**: Configurable maximum body size
- **Path Validation**: Secure file path handling
- **Configuration Validation**: Comprehensive config file validation
- **Access Control**: Method-based access restrictions

### Advanced Features
- **CGI Support**: Execute dynamic scripts (Python, PHP, Shell)
- **Multi-Server Support**: Multiple virtual servers on different ports
- **MIME Type Detection**: Automatic content type detection
- **Chunked Transfer Encoding**: Support for chunked request bodies

## 🏗️ Architecture

The server follows a modular, layered architecture designed for maintainability and scalability:

```
┌─────────────────────────────────────────────────────────────┐
│                    Configuration Layer                      │
│  ┌─────────────────┐  ┌─────────────────┐                  │
│  │ ConfigInterpreter│  │ ServerConfig    │                  │
│  │                 │  │ RouteConfig     │                  │
│  └─────────────────┘  └─────────────────┘                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      Network Layer                          │
│  ┌─────────────────┐  ┌─────────────────┐                  │
│  │   HttpServer    │  │    Reactor      │                  │
│  │                 │  │                 │                  │
│  └─────────────────┘  └─────────────────┘                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   Connection Layer                          │
│  ┌─────────────────┐  ┌─────────────────┐                  │
│  │   Connection    │  │   HttpRequest   │                  │
│  │                 │  │   HttpResponse  │                  │
│  └─────────────────┘  └─────────────────┘                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  Application Layer                          │
│  ┌─────────────────┐  ┌─────────────────┐                  │
│  │     Router      │  │RequestDispatcher│                  │
│  └─────────────────┘  └─────────────────┘                  │
│                              │                              │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │   GetHandler    │  │   PostHandler   │  │DeleteHandler│ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### Key Components

| Component | Responsibility |
|-----------|---------------|
| **ConfigInterpreter** | Parses configuration files and validates settings |
| **ServerConfig** | Holds server-specific configuration (host, port, routes) |
| **HttpServer** | Manages listening sockets and accepts connections |
| **Reactor** | Event loop using poll() for I/O multiplexing |
| **Connection** | Handles individual client connections |
| **Router** | Maps HTTP requests to appropriate routes |
| **RequestDispatcher** | Dispatches requests to method-specific handlers |
| **Method Handlers** | Implement specific HTTP method logic |

## 🛠️ Installation

### Prerequisites

- **Operating System**: Linux (Ubuntu 18.04+, CentOS 7+, or similar)
- **Compiler**: g++ with C++98 support
- **Build System**: GNU Make
- **Dependencies**: Standard C++ libraries only

### Build Instructions

1. **Clone the repository**
   ```bash
   git clone https://github.com/ajabrii/Webserver.git
   cd nexWebserv
   ```

2. **Compile the project**
   ```bash
   make
   ```

3. **Verify installation**
   ```bash
   ./Webserv --version
   ```

### Build Options

```bash
make clean      # Clean object files
make fclean     # Clean all generated files
make re         # Rebuild everything
```

## 🚀 Usage

### Basic Usage

```bash
# Start server with default configuration
./Webserv config/config.conf

# Start server with custom configuration
./Webserv /path/to/your/config.conf
```

### Quick Start Example

1. **Create a basic configuration file**
   ```nginx
   server {
       host = 127.0.0.1;
       port = 8080;
       server_name = localhost;
       client_max_body_size = 10M;

       error_page 404 /404.html;
       error_page 500 /500.html;

       route / {
           methods = GET POST;
           root = ./www;
           directory_listing = off;
       }
   }
   ```

2. **Create web content**
   ```bash
   mkdir -p www
   echo "<h1>Welcome to NexWebserv!</h1>" > www/index.html
   ```

3. **Start the server**
   ```bash
   ./Webserv config.conf
   ```

4. **Test the server**
   ```bash
   curl http://localhost:8080
   ```

## ⚙️ Configuration

The server uses a nginx-inspired configuration syntax for easy setup and maintenance.

### Configuration File Structure

```nginx
# Global settings can be defined here

server {
    # Server block configuration
    host = 127.0.0.1;                    # Listening IP address
    port = 8080;                         # Listening port
    port = 8081;                         # Additional ports (multi-port support)
    server_name = example.com www.example.com;  # Server names

    # Security settings
    client_max_body_size = 10M;          # Maximum request body size

    # Error pages
    error_page 404 /errors/404.html;
    error_page 500 /errors/500.html;

    # Route configuration
    route / {
        methods = GET POST DELETE;        # Allowed HTTP methods
        root = /var/www/html;            # Document root
        directory_listing = on;          # Enable directory listing
    }

    # File upload route
    route /uploads {
        methods = POST GET DELETE;
        root = /var/www/uploads;
        directory_listing = on;
    }

    # Redirect configuration
    route /old-page {
        methods = GET;
        redirect = https://example.com/new-page;
    }
}

# Multiple server blocks supported
server {
    host = 127.0.0.1;
    port = 9090;
    server_name = api.example.com;

    route /api {
        methods = GET POST PUT DELETE;
        root = /var/www/api;
        directory_listing = off;
    }
}
```

### Configuration Options

#### Server Block Options

| Option | Description | Example | Required |
|--------|-------------|---------|----------|
| `host` | IP address to bind to | `127.0.0.1` | Yes |
| `port` | Port number(s) to listen on | `8080` | Yes |
| `server_name` | Server name(s) for virtual hosting | `example.com www.example.com` | Yes |
| `client_max_body_size` | Maximum request body size | `10M` | Yes |
| `error_page` | Custom error page paths | `404 /404.html` | No |

#### Route Block Options

| Option | Description | Example | Required |
|--------|-------------|---------|----------|
| `methods` | Allowed HTTP methods | `GET POST DELETE` | Yes |
| `root` | Document root directory | `/var/www/html` | Yes |
| `directory_listing` | Enable directory browsing | `on` or `off` | Yes |
| `redirect` | Redirect URL | `https://example.com/new-page` | No |

## 📚 API Documentation

### HTTP Methods

#### GET Requests
- **Static Files**: Serves files from the configured document root
- **Directory Listing**: Shows directory contents when enabled
- **Error Handling**: Returns appropriate HTTP status codes

```bash
# Get a file
curl http://localhost:8080/index.html

# Get directory listing
curl http://localhost:8080/images/

# Get with custom headers
curl -H "User-Agent: MyClient/1.0" http://localhost:8080/api/data
```

#### POST Requests
- **File Upload**: Supports multipart/form-data uploads
- **Form Data**: Handles application/x-www-form-urlencoded
- **JSON Data**: Processes application/json content

```bash
# Upload a file
curl -X POST -F "file=@document.pdf" http://localhost:8080/uploads/

# Send JSON data
curl -X POST -H "Content-Type: application/json" \
     -d '{"name":"John","email":"john@example.com"}' \
     http://localhost:8080/api/users
```

#### DELETE Requests
- **File Deletion**: Removes files from the server
- **Authorization**: Respects configured access controls

```bash
# Delete a file
curl -X DELETE http://localhost:8080/uploads/old-file.txt
```

### Response Formats

#### Success Response
```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1234
Server: NexWebserv/1.0

<html>...</html>
```

#### Error Response
```http
HTTP/1.1 404 Not Found
Content-Type: text/html
Content-Length: 567
Server: NexWebserv/1.0

<html><body><h1>404 Not Found</h1></body></html>
```

## 🧪 Testing

### Manual Testing

1. **Basic Functionality**
   ```bash
   # Test GET request
   curl -v http://localhost:8080/

   # Test POST request
   curl -X POST -d "test=data" http://localhost:8080/api/test

   # Test file upload
   curl -X POST -F "file=@test.txt" http://localhost:8080/uploads/
   ```

2. **Load Testing**
   ```bash
   # Using Apache Bench
   ab -n 1000 -c 10 http://localhost:8080/

   # Using wrk
   wrk -t12 -c400 -d30s http://localhost:8080/
   ```

3. **Security Testing**
   ```bash
   # Test large request body
   curl -X POST -d "$(head -c 20M /dev/zero | tr '\0' 'x')" http://localhost:8080/

   # Test path traversal
   curl http://localhost:8080/../../../etc/passwd
   ```

### Test Environment Setup

```bash
# Create test directory structure
mkdir -p test/{www,uploads,cgi-bin}
echo "<h1>Test Page</h1>" > test/www/index.html

# Start server with test config
./Webserv test/config.conf
```

## 🤝 Contributing

We welcome contributions to NexWebserv! Please follow these guidelines:

### Development Setup

1. **Fork the repository**
2. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make your changes**
   - Follow the existing code style
   - Add tests for new functionality
   - Update documentation as needed

4. **Test your changes**
   ```bash
   make && ./test-script.sh
   ```

5. **Submit a pull request**

### Code Style Guidelines

- Use C++98 standard features only
- Follow Google C++ Style Guide
- Use descriptive variable and function names
- Add comments for complex logic
- Keep functions small and focused

### Reporting Issues

When reporting bugs, please include:
- Operating system and version
- Compiler version
- Steps to reproduce
- Expected vs actual behavior
- Configuration file (if relevant)

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by nginx configuration syntax
- Built for the 42 School webserv project
- Thanks to all contributors and testers

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/your-username/nexWebserv/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-username/nexWebserv/discussions)
- **Email**: your-email@example.com

---

<p align="center">
  <strong>Built with ❤️ by the NexWebserv Team</strong>
</p>
