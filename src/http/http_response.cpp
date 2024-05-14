//
// Created by pyq on 5/10/24.
//
#include "http_response.h"

const std::unordered_map<std::string, std::string> HttpResponse::CONTENT_TYPE = {
    {".html",  "text/html"},
    {".xml",   "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt",   "text/plain"},
    {".rtf",   "application/rtf"},
    {".pdf",   "application/pdf"},
    {".word",  "application/nsword"},
    {".png",   "image/png"},
    {".gif",   "image/gif"},
    {".jpg",   "image/jpeg"},
    {".jpeg",  "image/jpeg"},
    {".au",    "audio/basic"},
    {".mpeg",  "video/mpeg"},
    {".mpg",   "video/mpeg"},
    {".avi",   "video/x-msvideo"},
    {".gz",    "application/x-gzip"},
    {".tar",   "application/x-tar"},
    {".css",   "text/css"},
    {".js",    "text/javascript"},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const std::unordered_map<int, std::string> HttpResponse::ERROR_CODE_PATH = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

HttpResponse::HttpResponse() : code_(-1), isKeepAlive_(false), path_(""), srcDir_(""), mmFile_(nullptr), mmFileStat_({0}) {}

HttpResponse::~HttpResponse() {
    UnmapFile();
}

void HttpResponse::Init(const std::string& srcDir, std::string& path, bool isKeepAlive, int code) {
    assert(srcDir != "");
    if (mmFile_) {
        UnmapFile();
    }
    srcDir_ = srcDir;
    path_ = path;
    isKeepAlive_ = isKeepAlive;
    code_ = code;
    mmFile_ = nullptr;
    mmFileStat_ = {0};
}

void HttpResponse::UnmapFile() {
    if (mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}

void HttpResponse::MakeResponse(Buffer& buff) {
    // construct a response header and push to the buffer
    if (stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        // file does not exist or directory accessed
        code_ = 404;
        SetErrorCodePath_();
    } else if (!(mmFileStat_.st_mode & S_IROTH)) {
        // file is not readable
        code_ = 403;
        SetErrorCodePath_();
    } else if (code_ == -1) {
        // file is accessable
        code_ = 200;
    }
    AddStateLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
}

char* HttpResponse::GetFile() {
    return mmFile_;
}

size_t HttpResponse::GetFileLen() const {
    return mmFileStat_.st_size;
}

int HttpResponse::GetCode() const {
    return code_;
}

void HttpResponse::MakeErrorContent(Buffer& buff, std::string message) {
    // constructs an HTML response content containing error information 
    // and adds it to the given Buffer object
    // the content includes "Content-Length"
    std::string body, status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(code_) > 0) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += std::to_string(code_) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>Slim Web Server</em></body></html>";
    buff.Append("Content-Length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}

void HttpResponse::AddStateLine_(Buffer& buff) {
    std::string status;
    if (CODE_STATUS.count(code_) > 0) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        code_ = 400;
        status = "Bad Request";
    }
    buff.Append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::AddHeader_(Buffer& buff) {
    // "Content-Length" will be add at AddContent_
    buff.Append("Connection: ");
    if(isKeepAlive_) {
        buff.Append("keep-alive\r\n");
        buff.Append("Keep-Alive: max=6, timeout=120\r\n");  
    } else {
        buff.Append("close\r\n");
    }
    buff.Append("Content-Type: " + GetFileType_() + "\r\n"); 
}

void HttpResponse::AddContent_(Buffer& buff) {
    // actually we already check the file at the begin of MakeResponse
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
    if (srcFd < 0) {
        MakeErrorContent(buff, "File Not Found!");
        return;
    }
    LOG_DEBUG("File Path: %s", (srcDir_ + path_).data());
    // creates a copy-on-write private mapping to improve file access speed
    void* mmRet = mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if (mmRet == MAP_FAILED) {
        close(srcFd);
        MakeErrorContent(buff, "File Not Found!");
        return;
    }
    mmFile_ = (char*)mmRet;
    close(srcFd);
    buff.Append("Content-Length: " + std::to_string(mmFileStat_.st_size) + "\r\n\r\n");
}

void HttpResponse::SetErrorCodePath_() {
    if (ERROR_CODE_PATH.count(code_) > 0) {
        path_ = ERROR_CODE_PATH.find(code_)->second;
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}

std::string HttpResponse::GetFileType_() {
    std::string::size_type idx = path_.find_last_of('.');
    if (idx == std::string::npos) {
        // not find '.'
        return "text/plain";
    }
    std::string suffix = path_.substr(idx);
    if (CONTENT_TYPE.count(suffix) > 0) {
        return CONTENT_TYPE.find(suffix)->second;
    }
    return "text/plain";
}