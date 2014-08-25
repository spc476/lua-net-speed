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

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

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

static volatile sig_atomic_t g_sigint;

/*************************************************************************/

static void sigint_handler(int sig)
{
  g_sigint = sig;
}

/*************************************************************************/

int main(int argc,char *argv[])
{
  sockaddr_all__u  addr;
  sockaddr_all__u  remote;
  socklen_t        alen;
  socklen_t        remlen;
  int              port;
  int              sock;
  unsigned long    packet_count = 0;
  int              c;
  ssize_t          bytes;
  char             buffer[65508u];
  lua_State       *L;
  int              rc;
  
  if (argc == 1)
  {
    fprintf(stderr,"usage: %s -a addr -p port -s script [-h]\n",argv[0]);
    return EXIT_FAILURE;
  }
  
  L = luaL_newstate();
  if (L == NULL)
  {
    perror("luaL_newstate()");
    return EXIT_FAILURE;
  }
  
  luaL_openlibs(L);
  
  while((c = getopt(argc,argv,"a:p:s:h")) != EOF)
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
           
      case 's':
           rc = luaL_loadfile(L,optarg);
           if (rc != 0)
           {
             fprintf(stderr,"lua_loadfile: %s",lua_tostring(L,-1));
             lua_close(L);
             return EXIT_FAILURE;
           }
           
           rc = lua_pcall(L,0,LUA_MULTRET,0);
           if (rc != 0)
           {
             fprintf(stderr,"lua_pcall(): %s",lua_tostring(L,-1));
             lua_close(L);
             return EXIT_FAILURE;
           }
           break;
           
      case 'h':
      default:
           fprintf(stderr,"usage: %s -a addr -p port -s script [-h]\n",argv[0]);
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

  if (bind(sock,&addr.sa,alen) < 0)
  {
    perror("bind()");
    return EXIT_FAILURE;
  }
  
  if (signal(SIGINT,sigint_handler) == SIG_ERR)
  {
    perror("signal()");
    return EXIT_FAILURE;
  }
  
  while(g_sigint == 0)
  {
    bytes = recvfrom(sock,buffer,sizeof(buffer),0,&remote.sa,&remlen);
    if (bytes < 0)
      perror("recvfrom()");
    else
    {
      packet_count ++;
      lua_getglobal(L,"main");
      lua_pushlstring(L,buffer,bytes);
      rc = lua_pcall(L,1,0,0);
      if (rc != 0)
        fprintf(stderr,"lua_pcall(): %s",lua_tostring(L,-1));
    
    }
  }
  
  signal(SIGINT,SIG_DFL);
  printf("%lu\n",packet_count);
  printf("%d\n",lua_gc(L,LUA_GCCOUNT,0));
  lua_close(L);
  return EXIT_SUCCESS;
}

/*************************************************************************/
