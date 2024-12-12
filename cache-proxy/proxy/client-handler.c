#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "client-handler.h"
#include "logger/logger.h"
#include "http-parser/picohttpparser.h"

#define TIMEOUT             10000
#define MAX_REQ_SIZE        4096
#define MAX_NUM_HEADERS     20
#define MAX_HOST_NAME       256
#define MAX_RESP_SIZE       (1024 * 50)

typedef struct phr_header phrHeader_t;

typedef struct {
    const char *method;
    const char *path;
    phrHeader_t headers[MAX_NUM_HEADERS];
    size_t methodLen;
    size_t pathLen;
    size_t numHeaders;
    int minorVersion;
} reqParse_t;

static int strEq(const char *s1, const char *s2, size_t len);
static int setTimeout(int sock, unsigned int ms);
static void disconnect(int sock);
static int connectToServ(const char *hostName);

static phrHeader_t *findHeader(phrHeader_t headers[], size_t numHeaders, const char *name);

static int sendData(int sock, const char *data, size_t len);
static ssize_t readAndParseRequest(int sock, char *buf, size_t maxLen, reqParse_t *parseData);
static ssize_t readAndParseResponse(int sock, char *buf, size_t maxLen, reqParse_t *parseData);

int handleClient(int sock) {
    char request[MAX_REQ_SIZE + 1] = {0};
    char response[MAX_RESP_SIZE + 1] = {0};
    ssize_t reqLen, respLen;
    reqParse_t reqParse;
    int err;

    err = setTimeout(sock, TIMEOUT);
    if (err) {
        loggerError("Failed to set timeout for client, error: %s", strerror(errno));
        disconnect(sock);
        return -1;
    }

    reqLen = readAndParseRequest(sock, request, MAX_REQ_SIZE, &reqParse);
    if (reqLen < 0) {
        loggerError("Failed to read client request");
        disconnect(sock);
        return -1;
    }

    if (strEq(reqParse.method, "GET", 3)) {
        loggerInfo("GET");
    } else if (strEq(reqParse.method, "PUT", 3) || strEq(reqParse.method, "POST", 4)) {
        loggerInfo("PUT | POST");
    } else {
        loggerError("Unsupported method");
        disconnect(sock);
        return -1;
    }

    phrHeader_t *hostHeader = findHeader(reqParse.headers, reqParse.numHeaders, "Host");
    if (!hostHeader) {
        loggerError("Failed to fetch host name");
        disconnect(sock);
        return -1;
    }
    char hostName[hostHeader->value_len + 1];
    memcpy(hostName, hostHeader->value, hostHeader->value_len);
    hostName[hostHeader->value_len] = 0;

    int sockToServ = connectToServ(hostName);
    if (sockToServ < 0) {
        disconnect(sock);
        return -1;
    }
    loggerInfo("Connected successfully");

    err = sendData(sockToServ, request, reqLen);
    if (err) {
        loggerError("Failed to send request to server, error: %s", strerror(errno));
        disconnect(sock);
        disconnect(sockToServ);
        return -1;
    }
    loggerInfo("Sent request");

    respLen = readAndParseResponse(sockToServ, response, MAX_RESP_SIZE, &reqParse);
    if (respLen < 0) {
        loggerError("Failed to get response, error: %s", strerror(errno));
        disconnect(sock);
        disconnect(sockToServ);
        return -1;
    }

    err = sendData(sock, response, respLen);
    if (err) {
        loggerError("Failed to send back to client");
        disconnect(sock);
        disconnect(sockToServ);
        return -1;
    }

    disconnect(sock);
    disconnect(sockToServ);
    return 0;
}

static int strEq(const char *s1, const char *s2, size_t len) {
    return strncmp(s1, s2, len) == 0;
}

static int setTimeout(int sock, unsigned int ms) {
    struct timeval timeout;
    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = ms % 1000 * 1000;

    int err1 = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    int err2 = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    if (err1 || err2) {
        return -1;
    }
    return 0;
}

static void disconnect(int sock) {
    shutdown(sock, SHUT_RDWR);
    close(sock);
}

static int connectToServ(const char *hostName) {
    struct addrinfo *addrInfo;
    int err;

    int sockToServ = socket(AF_INET, SOCK_STREAM, 0);
    if (!sockToServ) {
        loggerError("connectToServ: failed to create new socket");
        return -1;
    }

    err = setTimeout(sockToServ, TIMEOUT);
    if (err) {
        loggerError("connectToServ: failed to set timeout, error: %s", strerror(errno));
        close(sockToServ);
        return -1;
    }

    err = getaddrinfo(hostName, "http", NULL, &addrInfo);
    if (err) {
        loggerError("connectToServ: failed to resolve server address");
        close(sockToServ);
        return -1;
    }

    err = connect(sockToServ, addrInfo->ai_addr, addrInfo->ai_addrlen);
    if (err) {
        loggerError("connectToServ: failed to connect, error: %s", strerror(errno));
        close(sockToServ);
        freeaddrinfo(addrInfo);
        return -1;
    }

    freeaddrinfo(addrInfo);
    return sockToServ;
}

static phrHeader_t *findHeader(phrHeader_t headers[], size_t numHeaders, const char *name) {
    size_t len = strlen(name);
    for (size_t i = 0; i < numHeaders; i++) {
        if (headers[i].name_len == len && strEq(headers[i].name, name, len)) {
            return &headers[i];
        }
    }
    return NULL;
}

static int sendData(int sock, const char *data, size_t len) {
    ssize_t sentTotal = 0;
    ssize_t sent;

    while (sentTotal != len) {
        sent = write(sock, data + sentTotal, len - sentTotal);
        if (sent == -1) return -1;
        sentTotal += sent;
    }
    return 0;
}

static ssize_t readAndParseRequest(int sock, char *buf, size_t maxLen, reqParse_t *parseData) {
    int pret;
    ssize_t rret;
    size_t buflen = 0, prevbuflen = 0;

    while (1) {
        /* read the request */
        while ((rret = read(sock, buf + buflen, maxLen - buflen)) == -1 && errno == EINTR);
        if (rret == -1) {
            loggerError("Receive error: %s", strerror(errno));
            return -1;
        }
        if (rret == 0) {
            loggerInfo("Peer socket shutdown");
            return -1;
        }

        prevbuflen = buflen;
        buflen += rret;

        /* parse the request */
        parseData->numHeaders = sizeof(parseData->headers) / sizeof(parseData->headers[0]);
        pret = phr_parse_request(
            buf, buflen, &parseData->method, &parseData->methodLen, &parseData->path, &parseData->pathLen,
            &parseData->minorVersion, parseData->headers, &parseData->numHeaders, prevbuflen
        );

        if (pret > 0) {
            loggerInfo("Parsed successfully");
            break;
        }

        else if (pret == -1) {
            loggerError("Error parsing request");
            return -1;
        }

        /* request is incomplete, continue the loop */
        assert(pret == -2);
        if (buflen == maxLen) {
            loggerError("Request is too long");
            return -1;
        }
    }

    return buflen;
}

static ssize_t readAndParseResponse(int sock, char *buf, size_t maxLen, reqParse_t *parseData) {
    ssize_t recvd;
    size_t bufLen = 0, headerLen, msgLen;
    char *headerEnd;
    const char *msg;
    int status;
    int pret;

    do {
        recvd = read(sock, buf + bufLen, maxLen - bufLen);
        if (recvd == -1) {
            loggerError("Receive error: %s", strerror(errno));
            return -1;
        }
        if (recvd == 0) {
            loggerError("Receive error: server disconnected");
            return -1;
        }
        bufLen += recvd;
        buf[bufLen] = 0;
    } while ((headerEnd = strstr(buf, "\r\n\r\n")) == NULL);
    headerEnd += strlen("\r\n\r\n");
    headerLen = headerEnd - buf;

    parseData->numHeaders = sizeof(parseData->headers) / sizeof(parseData->headers[0]);
    pret = phr_parse_response(
        buf, headerLen, &parseData->minorVersion, &status, &msg, &msgLen,
        parseData->headers, &parseData->numHeaders, 0
    );
    if (pret < 0) {
        loggerError("Failed to parse response, error: %i", pret);
        return -1;
    }

    phrHeader_t *contLenHeader = findHeader(parseData->headers, parseData->numHeaders, "Content-Length");
    if (!contLenHeader) {
        loggerError("Content length is not specified");
        return -1;
    }
    char contentLength[contLenHeader->value_len + 1];
    memcpy(contentLength, contLenHeader->value, contLenHeader->value_len);
    contentLength[contLenHeader->value_len] = 0;

    int contLen;
    contLen = atoi(contentLength);
    if (contLen < 0) {
        loggerError("Invalid content length");
        return -1;
    }

    size_t reqLen = contLen + headerLen;
    while (bufLen != reqLen) {
        recvd = read(sock, buf + bufLen, reqLen - bufLen);
        if (recvd <= 0) {
            loggerError("Failed to receive body");
            loggerDebug("BUFLEN %ld", bufLen);
            loggerDebug("REQLEN %ld", reqLen);
            return -1;
        }
        bufLen += recvd;
    }
    buf[bufLen] = 0;

    loggerDebug(
        "Total recv: %ld\n"
        "Header: %ld\n"
        "Content: %ld", bufLen, headerLen, contLen
    );

    return bufLen;
}
