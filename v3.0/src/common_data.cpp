#include "../include/common_data.h"

char RPS_SERVER_IP[48] = {0,};	
int  RPS_SERVER_PORT = 6610;		
char REDIS_CENTER_IP[48] = {0,};
char REDIS_CENTER_LIST_IP[128] = {0,};
char REDIS_STATUS_IP[48] = {0,};
int REDIS_STATUS_PORT = 5126;	
int REDIS_AUTH_PORT = 5116;		
int REDIS_RECONN_INTERNAL = 5;
int REDIS_CHECKHEALTH_INTERNAL = 60;
int HEATER_BEAT_INTERNAL = 120;	
int HEART_BEAT_TIMEOUT = 300;		
int REDIS_EXPIRE_TIME = 600;

const char *
status_code_to_str(int code) {
    switch (code) {
        case HTTP_RES_200:
            return "OK";
        case HTTP_RES_300:
            return "Redirect";
        case HTTP_RES_400:
            return "Bad Request";
        case HTTP_RES_NOTFOUND:
            return "Not Found";
        case HTTP_RES_SERVERR:
            return "Internal Server Error";
        case HTTP_RES_FORBIDDEN:
            return "Forbidden";
        case HTTP_RES_MOVEDPERM:
            return "Moved Permanently";
        case HTTP_RES_CREATED:
            return "Created";
        case HTTP_RES_ACCEPTED:
            return "Accepted";
        case HTTP_RES_NAUTHINFO:
            return "No Auth Info";
        case HTTP_RES_NOCONTENT:
            return "No Content";
        case HTTP_RES_RSTCONTENT:
            return "Reset Content";
        case HTTP_RES_PARTIAL:
            return "Partial Content";
        case HTTP_RES_MSTATUS:
            return "Multi-Status";
        case HTTP_RES_IMUSED:
            return "IM Used";
        case HTTP_RES_FOUND:
            return "Found";
        case HTTP_RES_SEEOTHER:
            return "See Other";
        case HTTP_RES_NOTMOD:
            return "Not Modified";
        case HTTP_RES_USEPROXY:
            return "Use Proxy";
        case HTTP_RES_SWITCHPROXY:
            return "Switch Proxy";
        case HTTP_RES_TMPREDIR:
            return "Temporary Redirect";
        case HTTP_RES_UNAUTH:
            return "Unauthorized";
        case HTTP_RES_PAYREQ:
            return "Payment Required";
        case HTTP_RES_METHNALLOWED:
            return "Not Allowed";
        case HTTP_RES_NACCEPTABLE:
            return "Not Acceptable";
        case HTTP_RES_PROXYAUTHREQ:
            return "Proxy Authentication Required";
        case HTTP_RES_TIMEOUT:
            return "Request Timeout";
        case HTTP_RES_CONFLICT:
            return "Conflict";
        case HTTP_RES_GONE:
            return "Gone";
        case HTTP_RES_LENREQ:
            return "Length Required";
        case HTTP_RES_PRECONDFAIL:
            return "Precondition Failed";
        case HTTP_RES_ENTOOLARGE:
            return "Entity Too Large";
        case HTTP_RES_URITOOLARGE:
            return "Request-URI Too Long";
        case HTTP_RES_UNSUPPORTED:
            return "Unsupported Media Type";
        case HTTP_RES_RANGENOTSC:
            return "Requested Range Not Satisfiable";
        case HTTP_RES_EXPECTFAIL:
            return "Expectation Failed";
        case HTTP_RES_IAMATEAPOT:
            return "I'm a teapot";
        case HTTP_RES_NOTIMPL:
            return "Not Implemented";
        case HTTP_RES_BADGATEWAY:
            return "Bad Gateway";
        case HTTP_RES_SERVUNAVAIL:
            return "Service Unavailable";
        case HTTP_RES_GWTIMEOUT:
            return "Gateway Timeout";
        case HTTP_RES_VERNSUPPORT:
            return "HTTP Version Not Supported";
        case HTTP_RES_BWEXEED:
            return "Bandwidth Limit Exceeded";
    } 
    return "UNKNOWN";
}

