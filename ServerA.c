#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>


int daemonize()
{
  pid_t pid;
  long n_desc;
  int i;

  if ((pid = fork()) != 0) {
    exit(0);
  }

  setsid();

  if ((pid = fork()) != 0) {
    exit(0);
  }

  chdir("/");
  umask(0);

  n_desc = sysconf(_SC_OPEN_MAX);
  for (i = 0; i < n_desc; i++) {
    close(i);
  }

  return 1;
}


void sigchld_handler(int signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[])
{
    daemonize();
    struct sockaddr_in sAddr;
    int listensock;
    int newsock;
    char buffer[25];
    int result;
    int nread;
    int pid;
    int val;
    listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    val = 1;
    result = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (result < 0) {
        perror("server2");
        return 0;
    }
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(1972);
    sAddr.sin_addr.s_addr = INADDR_ANY;
    result = bind(listensock, (struct sockaddr *) &sAddr, sizeof(sAddr));
    if (result < 0) {
        perror("server2");
        return 0;
    }
    result = listen(listensock, 5);
    if (result < 0) {
        perror("server2");
        return 0;
    }
    signal(SIGCHLD, sigchld_handler);
    while (1) {
        newsock = accept(listensock, NULL, NULL);
        pid = fork();
        if (pid == 0) {
          printf("child process %i created.\n", getpid());
          close(listensock);
          nread = recv(newsock, buffer, 25, 0);
          buffer[nread] = '\0';
          printf("%s\n", buffer);
          send(newsock, buffer, nread, 0);
          close(newsock);
          printf("child process %i finished.\n", getpid());
          exit(0);
        }
        close(newsock);
      }
}
