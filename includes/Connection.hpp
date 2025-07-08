/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/08 17:52:05 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Connection.hpp
#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <netinet/in.h>

enum RequestState {
    READING_HEADERS,
    READING_BODY_CONTENT_LENGTH,
    READING_BODY_CHUNKED,
    REQUEST_COMPLETE,
    // ERROR_STATE // Added for explicit error handling
};

class Connection {
private:
    int client_fd;
    sockaddr_in client_addr;
    std::string buffer;
    // bool headersDone; // anbdlo b enum 7sen 
    RequestState requestState;
    long contentLength; // Use long to match potential large sizes
    bool isChunked;
    HttpRequest currentRequest;
    // HttpRequest req;

public:
    Connection();
    Connection(int fd, const sockaddr_in& addr);
    ~Connection();

    int getFd() const;
    std::string readData(); // i remove const here to work with buffer in readData() function
    void writeData(const std::string& response) const;
    void closeConnection();
};

#endif



// void readAndProcessData() {
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
