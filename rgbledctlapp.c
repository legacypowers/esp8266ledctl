#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

int main(int argc, char *argv[]){
    int sockfd = 0, n = 0;
    uint8_t m = 0,r = 0, g = 0, b = 0, s = 0;
    int bytesWritten = 0;
    char buf[5];
    struct sockaddr_in serv_addr;
    
    memset(buf,'0',5*sizeof(char)); 
    if(argc < 6){
        printf("RGB LED Control cmdline control v:0.1 \n usage: %s <ip> <mode> <r> <g> <b> <s> \n modes : 0 = off , 1 = static, 2 = strobe, 3 = fade\n",argv[0]);
    }
    else{
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            printf("\n Error : Could not create socket \n");
            return 1;
        }
        printf("Assembling the packet: ");
        m = (uint8_t)atoi(argv[2]);
        r = (uint8_t)atoi(argv[3]);
        g = (uint8_t)atoi(argv[4]);
        b = (uint8_t)atoi(argv[5]);
        s = (uint8_t)atoi(argv[6]);
        printf("%x%x%x%x%x \t",m,r,g,b,s);
        buf[0]= (char) m;
        buf[1]= (char) r;
        buf[2]= (char) g;
        buf[3]= (char) b;
        buf[4]= (char) s;
        printf("[OK]\n");

        memset(&serv_addr, '0', sizeof(serv_addr)); 

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(1337); 
        if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0){
            printf("\n inet_pton error occured\n");
            return 1;
        } 
        printf("connecting to %s \t",argv[1]);
        if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
            printf("\n Error : Connect Failed \n");
            return 1;
        }
        printf("[OK]\n");
        printf("sending packet %x %x %x %x to %s \t",m,r,g,b,argv[1]);
        bytesWritten = write(sockfd,buf,sizeof(buf));
        printf("[OK]\n");
        printf("%d bytes sent.\n",bytesWritten);
        close(sockfd);
    }
    return 0;
}