#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "client-handler.h"
#include "logger/logger.h"
#include "http-parser/picohttpparser.h"

#define TIMEOUT             10
#define MAX_REQ_SIZE        4096
#define MAX_NUM_HEADERS     20

int handleClient(int sock) {
    char buf[MAX_REQ_SIZE + 1] = {0};
    const char *method, *path;
    int pret, minor_version;
    struct phr_header headers[MAX_NUM_HEADERS];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
    ssize_t rret;

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    int err = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (err) {
        loggerError("Failed to set timeout, error: %s", strerror(errno));
        return -1;
    }

    while (1) {
        /* read the request */
        while ((rret = read(sock, buf + buflen, sizeof(buf) - buflen)) == -1 && errno == EINTR);
        if (rret == -1) {
            loggerError("Receive error: %s", strerror(errno));
            return -1;
        }
        if (rret == 0) {
            loggerInfo("Peer socket shutdown");
            return 0;
        }

        prevbuflen = buflen;
        buflen += rret;
        /* parse the request */
        num_headers = sizeof(headers) / sizeof(headers[0]);
        pret = phr_parse_request(buf, buflen, &method, &method_len, &path, &path_len,
                                &minor_version, headers, &num_headers, prevbuflen);

        if (pret > 0) {
            loggerInfo("Parsed successfully");
            break; /* successfully parsed the request */
        }

        else if (pret == -1) {
            loggerError("Error parsing request");
            return -1;
        }

        /* request is incomplete, continue the loop */
        assert(pret == -2);
        if (buflen == MAX_REQ_SIZE) {
            loggerError("Request is too long");
            return -1;
        }
    }

    buf[MAX_REQ_SIZE] = 0;
    printf("%s", buf);

    return 0;
}
