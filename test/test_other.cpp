#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utility>

int main() {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("192.168.1.100");

    char ip_str[INET_ADDRSTRLEN]; // 存储IP地址的字符串

    if (inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN) != NULL) {
        printf("IP地址转化为字符串：%s\n", ip_str);
    } else {
        perror("inet_ntop");
        return 1;
    }

    return 0;
}
