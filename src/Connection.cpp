/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/08 18:03:39 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include "../includes/HttpServer.hpp"

// Implementation
Connection::Connection() : client_fd(-1), buffer(""), requestState(READING_HEADERS),
                         contentLength(0), isChunked(false){
    std::memset(&client_addr, 0, sizeof(client_addr));
}

Connection::Connection(int fd, const sockaddr_in& addr)
    : client_fd(fd), client_addr(addr), buffer(""), requestState(READING_HEADERS),
                         contentLength(0), isChunked(false){}

Connection::~Connection() {
    closeConnection();
}

int Connection::getFd() const {
    return client_fd;
}


std::string Connection::readData()
{
    char tmp[2048]; //? pay attantion to this i will change it later
    ssize_t bytesRead = recv(client_fd, tmp, sizeof(tmp) - 1, 0);
    std::cout << "bytes is ::" << bytesRead << std::endl;
    if (bytesRead < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now, this is not an error in non-blocking mode
            // Just return, poll atgol lina imta data atwsel tani
                return;
            } else {
                // chi haja khra error
                throw std::runtime_error("Failed to read from client socket");
            }
        } else if (bytesRead == 0) {
            // Client closed the connection
            // This is where you would handle closing the client_fd
            throw std::runtime_error("Client disconnected"); // Or handle gracefully
        }

        // zid data l buffer d request
        buffer.append(tmp, bytesRead);

    if (requestState == READING_HEADERS) {
            size_t headerEndPos = buffer.find("\r\n\r\n");
            if (headerEndPos != std::string::npos) {
                // Headers rah kamlin (done);
                std::string rawHeaders = buffer.substr(0, headerEndPos + 4);
                buffer.erase(0, headerEndPos + 4); // 7ythom mn buffer 3ndi f i have them in headers

                // Conceptual call (you'd need to adapt HttpRequest::parse)
                currentRequest.parseHeader(rawHeaders); // This function should populate headers map

                std::string cl_header = currentRequest.GetHeader("content-length");
                std::string te_header = currentRequest.GetHeader("transfer-encoding");

                if (!cl_header.empty() && !te_header.empty()) {
                    throw std::runtime_error("Both Content-Length and Transfer-Encoding present."); // Bad Request 400
                } else if (!cl_header.empty()) {
                    // Assuming GetHeader returns empty string if not found
                    // Convert Content-Length to long
                    std::istringstream ss(cl_header);
                    ss >> contentLength;
                    if (ss.fail() || contentLength < 0) {
                        throw std::runtime_error("Invalid Content-Length header."); // Bad Request 400
                    }
                    requestState = READING_BODY_CONTENT_LENGTH;
                } else if (!te_header.empty() && te_header == "chunked") {
                    isChunked = true;
                    requestState = READING_BODY_CHUNKED;
                } else {
                    // No Content-Length, no Transfer-Encoding. Request has no body.
                    requestState = REQUEST_COMPLETE;
                }
            }
        }
    // if (!headersDone) {
    //     size_t pos = buffer.find("\r\n\r\n");
    //     if (pos != std::string::npos) {
    //         headersDone = true;
    //     }
    // }
    std::cout << "-------------------------Received buffer:\n" << buffer << std::endl;
    std::cout << "-------------------------DONE: " << std::endl;
    return std::string(buffer);
}

void Connection::writeData(const std::string& response) const {
    ssize_t bytesSent = send(client_fd, response.c_str(), response.size(), 0);
    if (bytesSent < 0)
        throw std::runtime_error("Failed to write to client");
}

void Connection::closeConnection() {
    if (client_fd != -1) {
        close(client_fd);
        client_fd = -1;
    }
}


// Assuming this is part of your Connection or ClientConnection class
// and it has members like:
// int client_fd;
// std::string buffer; // This is the accumulation buffer for the entire request
// bool headersDone;    // To track if headers have been fully received
// long contentLength;  // Stores the Content-Length from headers, if present
// bool isChunked;      // True if Transfer-Encoding is chunked
// RequestState requestState; // An enum (READING_HEADERS, READING_BODY_CONTENT_LENGTH, READING_BODY_CHUNKED, REQUEST_COMPLETE)
// HttpRequest currentRequest; // To store the parsed request

// --- New/Modified Members for Connection Class (Conceptual) ---
// enum RequestState {
//     READING_HEADERS,
//     READING_BODY_CONTENT_LENGTH,
//     READING_BODY_CHUNKED,
//     REQUEST_COMPLETE,
//     ERROR_STATE // Added for explicit error handling
// };

// class Connection {
// public:
//     int client_fd;
//     std::string buffer;
//     RequestState requestState;
//     HttpRequest currentRequest;
//     long contentLength; // Use long to match potential large sizes
//     bool isChunked;
//     // ... other members you might need

//     Connection(int fd) : client_fd(fd), buffer(""), requestState(READING_HEADERS),
//                          contentLength(0), isChunked(false) {}

//     // You will call this function when poll/select indicates data is available
//     void readAndProcessData() {
//         char tmp[4096]; // Use a common buffer size like 4KB.
//                         // The subject mentions "client body size" limits,
//                         // but 4KB is a common temporary read buffer size.
//         ssize_t bytesRead = recv(client_fd, tmp, sizeof(tmp), 0); // Don't subtract 1 for null terminator here; append will handle size

//         if (bytesRead < 0) {
//             if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                 // No data available right now, this is not an error in non-blocking mode
//                 // Just return, poll/select will tell us again when data arrives
//                 return;
//             } else {
//                 // A real error occurred with recv
//                 throw std::runtime_error("Failed to read from client socket");
//             }
//         } else if (bytesRead == 0) {
//             // Client closed the connection
//             // This is where you would handle closing the client_fd
//             throw std::runtime_error("Client disconnected"); // Or handle gracefully
//         }

//         // Append the received data to the connection's buffer
//         buffer.append(tmp, bytesRead);

//         // --- State-based Processing ---
//         if (requestState == READING_HEADERS) {
//             size_t headerEndPos = buffer.find("\r\n\r\n");
//             if (headerEndPos != std::string::npos) {
//                 // Headers are complete!
//                 std::string rawHeaders = buffer.substr(0, headerEndPos + 4); // Include the \r\n\r\n
//                 // Remove headers from buffer, leaving only potential body data
//                 buffer.erase(0, headerEndPos + 4);

//                 // Now, parse only the headers to determine body handling
//                 // You will need to modify HttpRequest::parse to take rawHeaders
//                 // and just process headers, returning relevant info (Content-Length, Transfer-Encoding)
//                 // or setting members of currentRequest directly.

//                 // Conceptual call (you'd need to adapt HttpRequest::parse)
//                 currentRequest.parseHeadersOnly(rawHeaders); // This function should populate headers map

//                 std::string cl_header = currentRequest.GetHeader("content-length");
//                 std::string te_header = currentRequest.GetHeader("transfer-encoding");

//                 if (!cl_header.empty() && !te_header.empty()) {
//                     throw std::runtime_error("Both Content-Length and Transfer-Encoding present."); // Bad Request 400
//                 } else if (!cl_header.empty()) {
//                     // Assuming GetHeader returns empty string if not found
//                     // Convert Content-Length to long
//                     std::istringstream ss(cl_header);
//                     ss >> contentLength;
//                     if (ss.fail() || contentLength < 0) {
//                         throw std::runtime_error("Invalid Content-Length header."); // Bad Request 400
//                     }
//                     requestState = READING_BODY_CONTENT_LENGTH;
//                 } else if (!te_header.empty() && te_header == "chunked") {
//                     isChunked = true;
//                     requestState = READING_BODY_CHUNKED;
//                 } else {
//                     // No Content-Length, no Transfer-Encoding. Request has no body.
//                     requestState = REQUEST_COMPLETE;
//                 }
//             }
//         }

//         if (requestState == READING_BODY_CONTENT_LENGTH) {
//             if (buffer.length() >= contentLength) {
//                 currentRequest.body = buffer.substr(0, contentLength);
//                 buffer.erase(0, contentLength); // Remove body from buffer
//                 requestState = REQUEST_COMPLETE;
//             }
//             // Else, not enough body data yet, keep reading next time
//         }

//          if (requestState == READING_BODY_CHUNKED) {
//             // This is where it gets more complex. Your decodeChunked needs to work incrementally.
//             // It should consume chunks from 'buffer' and append to currentRequest.body.
//             // It needs to tell you if it reached the 0-byte chunk, signaling end of body.
//             // For now, let's assume currentRequest.decodeChunked can handle this:
//             bool finishedDecoding = currentRequest.decodeChunkedIncremental(buffer); // Modify this function
//             if (finishedDecoding) {
//                 requestState = REQUEST_COMPLETE;
//             }
//             // Else, not all chunks received yet, keep reading next time
//         }

//         // If REQUEST_COMPLETE, you can now process currentRequest
//         if (requestState == REQUEST_COMPLETE) {
//             // Process the complete request (e.g., route it, generate response)
//             // Example: processHttpRequest(currentRequest);

//             // After processing, reset the connection for the next request (if keep-alive)
//             buffer.clear();
//             currentRequest = HttpRequest(); // Reset request object
//             contentLength = 0;
//             isChunked = false;
//             requestState = READING_HEADERS; // Ready for next request on this connection
//         }
//     }
// };
