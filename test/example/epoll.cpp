#define MAX_EVENTS 5
#define READ_SIZE 10
#include <stdio.h>     // for fprintf()
#include <unistd.h>    // for close(), read()
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <string.h>    // for strncmp
#include <cstdio>
#include <cerrno>
#include <cstdlib>
 

void assertm(int a) {
  if (a < 0) {
    perror("error: ");
    exit(-1);
  }
}

int main()
{
  int epoll_fd = epoll_create1(0);
  assertm(epoll_fd);
 
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLOUT; // EPOLLET 可加上表示边沿触发
  event.data.fd = 0; // 也可以保存其他内容，比如Channel指针
  assertm(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event.data.fd, &event));
 
  char read_buffer[READ_SIZE + 1];
  struct epoll_event events[MAX_EVENTS];
  while(true)
  {
    printf("\nPolling for input...\n");
    int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
    printf("%d ready events\n", event_count);

    for(int i = 0; i < event_count; i++)
    {
      printf("Reading file descriptor '%d' -- ", events[i].data.fd);
      int bytes_read = read(events[i].data.fd, read_buffer, READ_SIZE);
      printf("%d bytes read.\n", bytes_read);
      read_buffer[bytes_read] = '\0';
      printf("Read '%s'\n", read_buffer);
 
      if(!strncmp(read_buffer, "stop\n", 5)) break;
    }
  }
 
  // assertm(close(epoll_fd));
  return 0;
}
