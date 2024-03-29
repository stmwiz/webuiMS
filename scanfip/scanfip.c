#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>    //socket() 
#include <netinet/in.h>   // inet_addr()
#include <arpa/inet.h>
#include <unistd.h>        // read()/ write
#include <sys/ioctl.h>
#include <netdb.h>
#include <net/if.h>

#include <errno.h>

extern int errno;
int get_local_ip(const char *eth_inf, char *ip)  
 {  
     int sd;  
    struct sockaddr_in sin;  
     struct ifreq ifr;  
   
     sd = socket(AF_INET, SOCK_DGRAM, 0);  
     if (-1 == sd)  
     {  
         printf("socket error: %s\n", strerror(errno));  
         return -1;        
     }  
   
     strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);  
     ifr.ifr_name[IFNAMSIZ - 1] = 0;  
       
     // if error: No such device  
     if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)  
     {  
         printf("ioctl error: %s\n", strerror(errno));  
         close(sd);  
         return -1;  
     }  
   
     memcpy(&sin, &ifr.ifr_addr, sizeof(sin));  
     snprintf(ip, 16, "%s", inet_ntoa(sin.sin_addr));  
    close(sd);  
	//printf("%s\n",ip);
     return 0;  
 }  

int main(int argc, char *argv[])
{ 
	char localhostip[24] = {'\0'};
	get_local_ip("eth0",localhostip);
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    fprintf(stderr, "socket id = %d\n", fd);
    // 地址复用
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    if(argc == 2)
        servaddr.sin_port =  htons(atoi(argv[1]));
    else
        servaddr.sin_port   = htons(9999);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0
    //sservaddr.sin_addr.s_addr = htonl(127.0.0.1); // 0.0.0.0
    int ret = bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret < 0){
        fprintf(stderr, "error:%s\n", strerror(errno));
        close(fd);
        return -1;
    }

    struct sockaddr_in peer;  // 保存对方的地址信息
    socklen_t size;
    char buff[1024];
    int cnt;

    while(1){
        bzero(&peer, sizeof(peer));
        size = sizeof(peer);
        //recv
        cnt = recvfrom(fd, buff, 1024, 0, (struct sockaddr *)&peer, &size);
        printf("ip:%s, port:%d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
        printf("recv: %s\n", buff);
        if(cnt <= 0){
            fprintf(stderr, "recv error or end\n");
        }else{
			if(NULL != strstr(buff,"getip")){
				get_local_ip("eth0",localhostip);
				//peer.sin_addr.s_addr = inet_addr(localhostip); // 0.0.0.0
        		peer.sin_port = htons(5440);
            	bzero(buff,1024);
            	sprintf(buff,"TBSWEBIP:%s",localhostip);
            	printf("sendto %s\n",buff);
            	sendto(fd, buff, strlen(buff)+1, 0, (struct sockaddr *)&peer, sizeof(peer));
			}
     		
        }
    }

    close(fd);
    return 0;

}
