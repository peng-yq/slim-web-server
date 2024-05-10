//
// Created by pyq on 5/10/24.
//
#pragma once
#ifndef SLIM_WEB_SERVER_HTTP_REQUEST_H
#define SLIM_WEB_SERVER_HTTP_REQUEST_H

#include <string>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <errno.h>
#include <mysql/mysql.h>
#include "../log/log.h"
#include "../buffer/buffer.h"
#include "../sql_connect/sql_connect.h"
#include "../sql_connect/sql_connect_raii.h"

// HttpRequest class handles parsing and storage of an HTTP request.
class HttpRequest {
public:
    // Enumerates the states of an HTTP request parsing process.
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADER,
        BODY,
        FINISH,
    };

    // Enumerates the HTTP code.
    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    HttpRequest() {Init();};

    ~HttpRequest() = default;

    // Initializes HttpRequest object, resetting all fields to default states.
    void Init();

    // Returns the path (e.g., /index.html) requested in the HTTP request.
    std::string Path() const;

    // Returns a modifiable reference to the path requested in the HTTP request.
    std::string& Path();

    // Returns the HTTP method (e.g., GET, POST) used in the request.
    std::string Method() const;

    // Returns the HTTP version specified in the request.
    std::string Version() const;

    // Retrieves the value associated with a key in the POST request body.
    
    std::string GetPost(const std::string& key) const;

    // Overloaded version of GetPost to handle C-style string keys.
    std::string GetPost(const char* key) const;

    // Determines whether the connection should be kept alive based on the "Connection" header.
    bool IsKeepAlive() const;

    // Parses the HTTP request from a buffer.
    bool ParseHttpRequest(Buffer& buff);

private:
    // Current state of the parsing process.
    PARSE_STATE state_;

    // Stores the method, path, version, and body of the HTTP request.
    std::string method_;
    std::string path_;
    std::string version_;
    std::string body_;
    std::unordered_map<std::string, std::string> header_;               // Stores header key-value pairs.
    std::unordered_map<std::string, std::string> post_;                 // Stores POST data key-value pairs.
    static const std::unordered_set<std::string> DEFAULT_HTML;          // Stores HttpRequest object by resetting all member variables.
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG; // Used to distinguish between login and registration according to the path_

    // Parses the request line to extract method, path, and version.
    bool ParseRequestLine_(const std::string& line);

    // Adjusts the path to handle default and HTML requests.
    void ParsePath_();

    // Parses a header line and stores the key-value pair in the header map.
    void ParseHeader_(const std::string& header);

    // Sets the body of the request and parses POST data if applicable.
    void ParseBody_(const std::string& body);

    // Parses the body for URL-encoded POST data.
    void ParsePostBody_();

    // Decodes URL-encoded strings into their original form.
    void ParseFromUrlEncoded_();

    // Converts a single hexadecimal character to its decimal equivalent.
    static int ConvertHexToDec(char ch);

    // Verifies user credentials for login or registration.
    static bool UserVerify (const std::string& name, const std::string& pwd, bool isLogin);
};

#endif //SLIM_WEB_SERVER_HTTP_REQUEST_H
