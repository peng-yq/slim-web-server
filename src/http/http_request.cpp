//
// Created by pyq on 5/10/24.
//
#include "http_request.h"

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML = {
    "/index",
    "/register",
    "/login",
    "/welcome",
    "/video",
    "/picture",
};

const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG = {
    {"/register.html", 0},
    {"/login.html", 1},
};

void HttpRequest::Init() {
    method_ = path_ = version_ = body_ = "";
    state_ =  REQUEST_LINE;
    header_.clear();
    post_.clear();
}

std::string HttpRequest::Path() const {
    return path_;
}

std::string& HttpRequest::Path() {
    return path_;
}

std::string HttpRequest::Method() const {
    return method_;
}

std::string HttpRequest::Version() const {
    return version_;
}

std::string HttpRequest::GetPost(const std::string& key) const {
    assert(key != "");
    if (post_.count(key) > 0) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key != "");
    if (post_.count(key) > 0) {
        return post_.find(key)->second;
    }
    return "";
}

bool HttpRequest::IsKeepAlive() const {
    if (header_.count("Connection") > 0) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1"; 
    }
    return false;
}

bool HttpRequest::ParseHttpRequest(Buffer& buff) {
    const char CRLF[] = "\r\n";
    if (buff.GetReadableBytes() <= 0) {
        return false;
    }
    while (buff.GetReadableBytes() && state_ != FINISH) {
        // lineEnd is a pointer to the beginning of the found CRLF sequence in buffer. 
        // If not found, lineEnd will point to the return value of buff.BeginWriteConst().
        const char* lineEnd = std::search(buff.BeginRead(), buff.BeginWriteConst(), CRLF, CRLF + 2);
        if (lineEnd == buff.BeginWriteConst()) {
            buff.AdvanceReadPointer(lineEnd - buff.BeginRead());
            break;
        }

        // line contains all characters from the beginning of the buffer to the first CRLF, excluding the CRLF itself.
        // parse the request according to state_
        std::string line(buff.BeginRead(), lineEnd);
        switch (state_) {
            case REQUEST_LINE:
                if (!ParseRequestLine_(line)) {
                    return false;
                }
                ParsePath_();
                break;
            case HEADER:
                ParseHeader_(line);
                if (buff.GetReadableBytes() <= 2) {
                    state_ = FINISH;
                }
                break;
            case BODY:
                ParseBody_(line);
                break;
            default:
                break;
        }
        buff.AdvanceReadPointer(lineEnd + 2 - buff.BeginRead());
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

bool HttpRequest::ParseRequestLine_(const std::string& line) {
    // use regular expressions to match and parse HTTP request lines
    // "GET /index.html HTTP/1.1" eg. 
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    // subMatch[0] stores the complete string matched by the entire expression 
    if (std::regex_match(line, subMatch, pattern)) {
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADER;
        return true;
    }
    LOG_ERROR("Parse RequestLine Error!");
    return false;
}

void HttpRequest::ParsePath_() {
    if (path_ == "/") {
        path_ = "/index.html";
    } else {
        for (auto &item : DEFAULT_HTML) {
            if (path_ == item) {
                path_ += ".html";
                break;
            }
        }
    }
}

void HttpRequest::ParseHeader_(const std::string& header) {
    // use regular expressions to parse a single header line in an HTTP request. 
    // separate the key and value of the header and store them in a map.
    // Host: www.example.com
    // Content-Type: application/x-www-form-urlencoded
    // Content-Length: 27 
    // eg.
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if (std::regex_match(header, subMatch, pattern)) {
        header_[subMatch[1]] = subMatch[2];
    } else {
        state_ = BODY;
    }
}

void HttpRequest::ParseBody_(const std::string& body) {
    body_ = body;
    ParsePostBody_();
    state_ = FINISH;
    LOG_DEBUG("RequestBody:%s, Len:%d", body.c_str(), body.size());
}

void HttpRequest::ParsePostBody_() {
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlEncoded_();
        if (DEFAULT_HTML_TAG.count(path_)) {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag:%d", tag);
            if (tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                if (UserVerify(post_["username"], post_["password"], isLogin)) {
                    path_ = "/welcome.html";
                } else {
                    path_ = "/error.html";
                }
            }
        }
    }
}

void HttpRequest::ParseFromUrlEncoded_() {
    if (body_.size() == 0) {
        return;
    }
    // username=johndoe&password=12345%21 for key value eg.
    std::string key, value;
    int num = 0, i = 0, j = 0, n = body_.size();
    for (; i < n; ++i) {
        char ch = body_[i];
        switch (ch) {
            case '=':
                key = body_.substr(j, i - j);
                j = i + 1;
                break;
            case '+':
                body_[i] = ' ';
                break;
            case '%':
                // Convert hexadecimal to characters
                // %23 -> '#'
                // standard url encoding format is %xx
                num = ConvertHexToDec(body_[i + 1]) * 16 + ConvertHexToDec(body_[i + 2]);               
                body_[i] = num;
                body_.erase(body_.begin() + i + 1);
                body_.erase(body_.begin() + i + 1);
                n -= 2;
                break;
            case '&':
                value = body_.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
        assert(j <= i);
        // add the last key-value pair
        // only if key does not exist and value is not empty
        if (post_.count(key) == 0 && j < i) {
            value = body_.substr(j, i - j);
            post_[key] = value;
        }
    }
}

int HttpRequest::ConvertHexToDec(char ch) {
    // ConvertHexToDec is only called when processing hexadecimal
    // so does not handle the situation 
    // where ch is not a hexadecimal character
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
}

bool HttpRequest::UserVerify (const std::string& name, const std::string& pwd, bool isLogin) {
    if (name == "" || pwd == "") {
        return false;
    }
    LOG_INFO("Verify User: Name:%s", name.c_str());
    
    // RAII must use named objects. 
    // anonymous objects will be destructed 
    // and resources will be released after this line of code is executed.
    MYSQL* sqlConn;
    SqlConnRAII sqlConnRAII(&sqlConn, SqlConnPool::Instance());
    assert(sqlConn);

    std::string query;
    if (isLogin) {
        // login request
        query = "SELECT password FROM user WHERE username='" + name + "' LIMIT 1";
    } else {
        // register request
        query = "SELECT username FROM user WHERE username='" + name + "' LIMIT 1";
    }
    LOG_DEBUG("SQL Query: %s", query.c_str());
    
    // send SQL statements to the MySQL server and execute 
    if (mysql_query(sqlConn, query.c_str())) {
        LOG_ERROR("SQL Error: %s", mysql_error(sqlConn));
        return false;
    }

    // retrieve the complete result set returned by the SELECT, SHOW, DESCRIBE, or EXPLAIN statement 
    // executed by the most recent call to mysql_query
    MYSQL_RES* res = mysql_store_result(sqlConn);
    // retrieve data row by row from the result set returned by mysql_store_result. 
    // it returns a MYSQL_ROW structure, which is actually an array of strings 
    // where each element corresponds to a field value in a row in the result set.
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        // query result is empty
        mysql_free_result(res);
        if (isLogin) {
            // login request and the user does not exist
            LOG_DEBUG("User %s not found!", name.c_str());
            return false;
        } else {
            // register user (user name is not been used)
            query = "INSERT INTO user(username, password) VALUES('" + name + "','" + pwd + "')";
            LOG_DEBUG("SQL Query: %s", query.c_str());
            if (mysql_query(sqlConn, query.c_str())) {
                LOG_ERROR("SQL Insert Error: %s", mysql_error(sqlConn));
                return false;
            }
            LOG_DEBUG("Registered User %s", name.c_str());
            return true;
        }
    } else if (isLogin) {
        // check the login information
        std::string password(row[0]);
        mysql_free_result(res);
        if (pwd == password) {
            LOG_DEBUG("User %s Logged In Successfully", name.c_str());
            return true;
        } else {
            LOG_DEBUG("User %s Password Error", name.c_str());
            return false;
        }
    } else {
        // register request and user name already exists
        mysql_free_result(res);
        LOG_DEBUG("User %s Already Exists", name.c_str());
        return false;
    }
}