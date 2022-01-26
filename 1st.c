#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
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

int main(int argc, char **argv)
{
  daemonize();

  openlog("test_daemon", LOG_PID, LOG_USER);
  syslog(LOG_INFO, "%s", "Hello World!");
  closelog();

  return 1;
}

void* thread_proc(void *arg);

int main(int argc, char *argv[])
{
    daemonize();
    struct sockaddr_in sAddr;
    int listensock;
    int newsock;
    int result;
    pthread_t thread_id;
    int val;
    
    listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    val = 1;
    result = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (result < 0) {
        perror("server4");
        return 0;
    }

    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(1972);
    sAddr.sin_addr.s_addr = INADDR_ANY;

    result = bind(listensock, (struct sockaddr *) &sAddr, sizeof(sAddr));
    if (result < 0) {
        perror("exserver4");
        return 0;
    }

    result = listen(listensock, 5);
    if (result < 0) {
        perror("exserver4");
        return 0;
    }

    while (1) {
        newsock = accept(listensock, NULL ,NULL);
	result = pthread_create(&thread_id, NULL, thread_proc, (void *) newsock);
	if (result != 0) {
	  printf("Could not create thread.\n");
	  return 0;
	}
	pthread_detach(thread_id);
	sched_yield();
    }
}

void* thread_proc(void *arg)
{
  int sock;
  char buffer[25];
  int nread;

  printf("child thread %i with pid %i created.\n", pthread_self(), getpid());
  sock = (int) arg;
  nread = recv(sock, buffer, 25, 0);
  buffer[nread] = '\0';
  printf("%s\n", buffer);
  send(sock, buffer, nread, 0);
  close(sock);
  printf("child thread %i with pid %i finished.\n", pthread_self(), getpid());
}
