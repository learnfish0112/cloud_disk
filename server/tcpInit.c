#include "head.h"

int tcpInit(const char *ip, const char *port, int *psockfd) {
    //Normally only a single protocol exists to support a particular
    //socket  type  within a given protocol family, in which case protocol can be specified as 0.
    //AF_INET:IPV4 SOCK_STREAM: TCP
    *psockfd = socket(AF_INET, SOCK_STREAM, 0);

    //open the reuse of the port
    //reuse not zero means open
    int reuse = 1;
    int ret = setsockopt(*psockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    NETDISK_LOG_DEBUG(ret, -1, "setsockopt");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    //string -> int -> big endian
    addr.sin_port = htons(atoi(port));
    addr.sin_addr.s_addr = inet_addr(ip);
    //bind a name to socket
    ret = bind(*psockfd, (struct sockaddr *)&addr, sizeof(addr));
    NETDISK_LOG_DEBUG(ret, -1, "bind");

    //pending connection maxnum is 10
    ret = listen(*psockfd, PENDING_CONNECT_MAX_NUM);
    NETDISK_LOG_DEBUG(ret, -1, "listen");

    return SERVER_ROK;
}
