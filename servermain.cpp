

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <list>

#define MYPORT "32859"    // the port users will be connecting to
#define APORT "30859" 
#define BPORT "31859" 


#define MAXBUFLEN 1024
#define BUF_SIZE 1024

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



int main(void)
{
    int sockfd;
    int sockfd1;
    int sockfd2;
    struct addrinfo hints, *servinfo,*servinfo1,*servinfo2, *p,*p1,*p2;
    int rv;
    int numbytes;
    std::string address="127.0.0.1";
    struct sockaddr_storage their_addr;
    char buf1[MAXBUFLEN];
    char buf2[MAXBUFLEN];
    socklen_t addr_len= 1024;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    // hints.ai_flags = AI_PASSIVE; // use my IP
    // std::cout<<2<<std::endl;

    //sendto serverA

    if ((rv = getaddrinfo(address.c_str(), APORT, &hints, &servinfo1)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p1 = servinfo1; p1 != NULL; p1 = p1->ai_next) {
        if ((sockfd1 = socket(p1->ai_family, p1->ai_socktype, p1->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        break;
    }

    if (p1 == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }



    // sendto serverB

    if ((rv = getaddrinfo(address.c_str(), BPORT, &hints, &servinfo2)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
        if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype,
                p2->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        break;
    }

    //receive from A and B

    if (p2 == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }



    if ((rv = getaddrinfo(address.c_str(), MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    printf("Main server is up and running.\n");

    //send signal to A and B: the server is running!

    std::string sig="1";

    if ((numbytes = sendto(sockfd1, sig.c_str(), strlen(sig.c_str()), 0, p1->ai_addr, p1->ai_addrlen)) == -1) {
        perror("serverA: sendto");
        exit(1);
    }

    if ((numbytes = recvfrom(sockfd, buf1, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    printf("Main server has received the department list from server A using UDP over port 32859\n");
    printf("Main server has received the department list from server B using UDP over port 32859\n");

    buf1[numbytes] = '\0';

    // store A's info
    std::unordered_map<std::string, int> department_backend_mapping;
    std::stringstream is(buf1);
    std::string depart_str;

    printf("Server A\n");

    while(getline(is,depart_str,',')){
        department_backend_mapping.insert(std::make_pair(depart_str,1));
        std::cout<<depart_str<<std::endl; 
    }

    if ((numbytes = sendto(sockfd2, sig.c_str(), strlen(sig.c_str()), 0, p2->ai_addr, p2->ai_addrlen)) == -1) {
        perror("serverB: sendto");
        exit(1);
    }
    

    if ((numbytes = recvfrom(sockfd, buf2, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    printf("\n");

    buf2[numbytes] = '\0';

    printf("Server B\n");

    //store B's info

    std::stringstream is2(buf2);
    while(getline(is2,depart_str,',')){
        department_backend_mapping.insert(std::make_pair(depart_str,2));
        std::cout<<depart_str<<std::endl; 
    }
    printf("\n");
    while(1){
        printf("Enter Department Name:\n");

        std::string department;

        std::cin>>department;
        if(department_backend_mapping.find(department)==department_backend_mapping.end()){
            printf("%s does not show up in server A&B\n",department.c_str());
        }
        else{
            if(department_backend_mapping[department]==1){
                printf("%s shows up in server A\n",department.c_str());
                printf("The Main Server has sent request for %s to server A using UDP over port 32859\n",department.c_str());
                printf("The Main server has received searching result(s) of %s from server A\n",department.c_str());
                if ((numbytes = sendto(sockfd1, department.c_str(), strlen(department.c_str()), 0, p1->ai_addr, p1->ai_addrlen)) == -1) {
                    perror("serverA: sendto");
                    exit(1);
                }

                if ((numbytes = recvfrom(sockfd, buf1, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
                buf1[numbytes] = '\0';
                printf("There are %s distinct students in %s.\n",buf1,department.c_str());
                if ((numbytes = recvfrom(sockfd, buf1, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
                buf1[numbytes-1] = '\0';
                printf("Their IDs are %s\n\n",buf1);
            }
            else{
                printf("%s shows up in server B\n",department.c_str());
                printf("The Main Server has sent request for %s to server B using UDP over port 32859\n",department.c_str());
                printf("The Main server has received searching result(s) of %s from server B\n",department.c_str());
                if ((numbytes = sendto(sockfd2, department.c_str(), strlen(department.c_str()), 0, p2->ai_addr, p2->ai_addrlen)) == -1) {
                    perror("serverB: sendto");
                    exit(1);
                }

                if ((numbytes = recvfrom(sockfd, buf2, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
                buf2[numbytes] = '\0';
                printf("There are %s distinct students in %s.\n",buf2,department.c_str());
                if ((numbytes = recvfrom(sockfd, buf2, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
                buf2[numbytes-1] = '\0';
                printf("Their IDs are %s\n",buf2);
            }
        }

        printf("-----Start a new query-----\n");
    }

    std::cout << "Received from client 1: " << buf1 << std::endl;
    std::cout << "Received from client 2: " << buf2 << std::endl;

    freeaddrinfo(servinfo);
    freeaddrinfo(servinfo1);
    freeaddrinfo(servinfo2);

    close(sockfd);
    close(sockfd1);
    close(sockfd2);


    return 0;
}