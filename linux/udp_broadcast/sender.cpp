#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <ifaddrs.h>

#include "constants.h"
#include "NetUtils.h"

static int create_sender(const char*  ipaddr, unsigned int port)  {
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(-1);
    }

    u_int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("reusing ADDR failed");
        exit(1);
    }

    int broadcast = 1;
    if (setsockopt(fd,SOL_SOCKET,SO_BROADCAST, &broadcast, sizeof(broadcast)) != 0) {
        perror("setsockopt - SO_SOCKET ");
        exit(-1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipaddr);
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("bind");
        exit(-1);
    }

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(-1);
    }
    return fd;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s [iface]\n", argv[0]);
        return -1;
    }
    char* iface = argv[1];

    NetUtils::Interface interface;
    if (NetUtils::findInterface(iface, &interface)) {
        fprintf(stderr, "cannot find interface %s\n", iface);
        return -1;
    }
    printf("using interface %s  addr=%s  netmask=%s  bcast=%s\n", interface.name.c_str(),
        interface.addr.c_str(), interface.netmask.c_str(), interface.bcast.c_str());

    int fd = create_sender(interface.bcast.c_str(), PORT);

    const char* data = "Hello BroadCast!";
    int len = (int)strlen(data) + 1;

    while (1) {
        printf("sending %d bytes\n", len);
        if (send(fd, data, len, 0) < 0) {
            perror("send");
            break;
        }
        sleep(1);
    }
    close(fd);
    return 0;
}



