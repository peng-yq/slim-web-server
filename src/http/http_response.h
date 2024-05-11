//
// Created by pyq on 5/10/24.
//
#pragma once
#ifndef SLIM_WEB_SERVER_HTTP_RESPONSE_H
#define SLIM_WEB_SERVER_HTTP_RESPONSE_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unordered_map>
#include "../log/log.h"
#include "../buffer/buffer.h"

// Class for handling HTTP responses, including file mapping, status management, and header content generation.
class HttpResponse {
public:
    // Constructor: Initializes response with default values.
    HttpResponse();

    // Destructor: Unmaps any mapped files.
    ~HttpResponse();

    // Initializes the response with specified directory, path, connection type, and status code.
    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    
    // Unmaps the file that was mapped into memory.
    void UnmapFile();
    
    // Constructs and sends the complete HTTP response including status line, headers, and body.
    void MakeResponse(Buffer& buff);
    
    // Returns a pointer to the file data mapped into memory.
    char* GetFile();

    // Returns the length of the mapped file.
    size_t GetFileLen() const;

    // Returns the HTTP status code.
    int GetCode() const;

    // Generates HTML content for error messages and appends it to the response buffer.
    void MakeErrorContent(Buffer& buff, std::string message);

private:
    int code_;                  // HTTP status code.
    bool isKeepAlive_;          // Flag to keep the connection alive.
    std::string path_;          // Path to the requested file.
    std::string srcDir_;        // Directory of the source files.
    char* mmFile_;              // Pointer to the memory-mapped file data.
    struct stat mmFileStat_;    // File status structure.
    static const std::unordered_map<std::string, std::string> CONTENT_TYPE;     // Map of file extensions to MIME types.
    static const std::unordered_map<int, std::string> CODE_STATUS;              // Map of status codes to messages.
    static const std::unordered_map<int, std::string> ERROR_CODE_PATH;          // Map of error codes to error document paths.

    // Adds the HTTP status line to the buffer.
    void AddStateLine_(Buffer& buff);

    // Adds HTTP headers to the buffer.
    void AddHeader_(Buffer& buff);

    // Adds the content of the response to the buffer.
    void AddContent_(Buffer& buff);

    // Sets the path for the error code document.
    void SetErrorCodePath_();

    // Determines the MIME type based on the file extension.
    std::string GetFileType_();
};

#endif //SLIM_WEB_SERVER_HTTP_RESPONSE_H
