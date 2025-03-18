#include "head.h"

int epollAdd(int epfd, int fd) {
    struct epoll_event event;
    //EPOLLIN: the associated file is available for read(2) operations.
    event.events = EPOLLIN;
    event.data.fd = fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    NETDISK_LOG_DEBUG(ret, -1, "epoll_ctl");

    return 0;
}
