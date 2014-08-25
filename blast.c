/***************************************************************************
*
* Copyright 2014 by Sean Conner.
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 3 of the License, or (at your
* option) any later version.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
* License for more details.
*  
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, see <http://www.gnu.org/licenses/>.
*
* Comments, questions and criticisms can be sent to: sean@conman.org
*
*************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>

#include <getopt.h>

#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <net/if.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

/*************************************************************************/

typedef union sockaddr_all
{
  struct sockaddr     sa;
  struct sockaddr_in  sin;
  struct sockaddr_in6 sin6;
  struct sockaddr_un  ssun;
} sockaddr_all__u;

/*************************************************************************/

static volatile sig_atomic_t g_sigalrm;
static char                  g_buffer[1024uL * 1024uL];

/*************************************************************************/

static void sigalrm_handler(int sig)
{
  g_sigalrm = sig;
}

/*************************************************************************/

int main(int argc,char *argv[])
{
  sockaddr_all__u   addr;
  socklen_t         alen;
  struct itimerval  interval;
  int               port;
  int               sock;
  size_t            size;
  size_t            idx = 0;
  unsigned long     packet_count = 0;
  int               c;
  FILE             *rnd;
  
  if (argc == 1)
  {
    fprintf(stderr,"usage: %s -a addr -p port -t time -s size [-h]\n",argv[0]);
    return EXIT_FAILURE;
  }
  
  rnd = fopen("/dev/urandom","rb");
  if (rnd == NULL)
  {
    perror("/dev/urandom");
    return EXIT_FAILURE;
  }
  
  fread(g_buffer,1,sizeof(g_buffer),rnd);
  fclose(rnd);
  
  while((c = getopt(argc,argv,"a:p:t:s:h")) != EOF)
  {
    switch(c)
    {
      case 'a':
           if (inet_pton(AF_INET,optarg,&addr.sin.sin_addr.s_addr))
           {
             addr.sin.sin_family = AF_INET;
             alen = sizeof(struct sockaddr_in);
           }
           else if (inet_pton(AF_INET6,optarg,&addr.sin6.sin6_addr.s6_addr))
           {
             addr.sin6.sin6_family = AF_INET6;
             alen = sizeof(struct sockaddr_in6);
           }
           else
           {
             memcpy(addr.ssun.sun_path,optarg,strlen(optarg)+1);
             addr.ssun.sun_family = AF_UNIX;
             alen = sizeof(struct sockaddr_un);
           }
           break;
           
      case 'p':
           port = strtoul(optarg,NULL,10);
           break;
           
      case 't':
           interval.it_interval.tv_sec  = 0;
           interval.it_interval.tv_usec = 0;
           interval.it_value.tv_sec     = strtoul(optarg,NULL,10);
           interval.it_value.tv_usec    = 0;
           break;
      
      case 's':
           size = strtoul(optarg,NULL,10);
           break;
           
      case 'h':
      default:
           fprintf(stderr,"usage: %s -a addr -p port -t time -s size [-h]\n",argv[0]);
           return EXIT_FAILURE;
    }
  }
  
  switch(addr.sa.sa_family)
  {
    case AF_INET:  addr.sin.sin_port   = htons(port); break;
    case AF_INET6: addr.sin6.sin6_port = htons(port); break;
    case AF_UNIX:
    default:       assert(0); break;
  }
  
  sock = socket(addr.sa.sa_family,SOCK_DGRAM,0);
  if (sock < 0)
  {
    perror("socket()");
    return EXIT_FAILURE;
  }

  if (signal(SIGALRM,sigalrm_handler) == SIG_ERR)
  {
    perror("signal()");
    return EXIT_FAILURE;
  }
  
  if (setitimer(ITIMER_REAL,&interval,NULL) < 0)
  {
    perror("setitimer()");
    return EXIT_FAILURE;
  }
  
  while(g_sigalrm == 0)
  {
    if (sendto(sock,&g_buffer[idx],size,0,&addr.sa,alen) < 0)
      perror("sendto()");
    else
    {
      idx++;
      if (idx + size > sizeof(g_buffer))
        idx = 0;
      
      packet_count ++;
    }
  }
  
  signal(SIGALRM,SIG_DFL);
  printf("%lu\n",packet_count);
  return EXIT_SUCCESS;
}

/*************************************************************************/
