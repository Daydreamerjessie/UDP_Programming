
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
#define MAXBUFLEN 1024

#define SERVERPORT "32859"    // the port users will be connecting to
#define APORT "30859" 

int main(int argc, char *argv[])
{
    int sockfd,sockfd_s;
    struct addrinfo hints, *servinfo, *servinfo_s, *pr, *ps;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len= 1024;
    std::string address="127.0.0.1";
    char buf[MAXBUFLEN];

    // if (argc != 3) {
    //     fprintf(stderr,"usage: talker hostname message\n");
    //     exit(1);
    // }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    //p_receive

    if ((rv = getaddrinfo(address.c_str(), APORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(pr = servinfo; pr != NULL; pr = pr->ai_next) {
        if ((sockfd = socket(pr->ai_family, pr->ai_socktype,
                pr->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        if (bind(sockfd, pr->ai_addr, pr->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (pr == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }
    printf("Server A is up and running using UDP on port 30859\n");

    //p_send

    if ((rv = getaddrinfo(address.c_str(), SERVERPORT, &hints, &servinfo_s)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    for(ps = servinfo_s; ps != NULL; ps = ps->ai_next) {
        if ((sockfd_s = socket(ps->ai_family, ps->ai_socktype,
                ps->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (ps == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }


    //Read dataA.txt


    std::unordered_map<std::string, int> department_stu;
    std::unordered_map<int, std::string> stu_department;
    std::ifstream file("dataA.txt");
    std::string data;
    std::string department;
    int flag=0;
    int num_stu;
    int stu;
    std::string stu_str;
    std::string depart_list;

    while(file>>data){
        if(flag==0){
            department=data;
            flag=1;
            num_stu=0;
            depart_list+=department;
            depart_list+=',';
        }
        else{
            flag=0;
            std::stringstream is(data);
            while(getline(is,stu_str,',')){
                stu=std::stoi(stu_str);
                if(stu_department.find(stu)==stu_department.end()){
                    stu_department.insert(std::make_pair(stu,department));
                    num_stu+=1;
                    // std::cout<<stu<<department<<std::endl; 
                }
            }
        }
        if(flag==0){
            if(num_stu>0){
                department_stu.insert(std::make_pair(department,num_stu));
                //  std::cout<<department<<num_stu<<std::endl; 
            }
        }
    }
    //waiting for the servermain
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    //send the department list
    if ((numbytes = sendto(sockfd_s, depart_list.c_str(), strlen(depart_list.c_str()), 0, ps->ai_addr, ps->ai_addrlen)) == -1) {
        perror("serverA: sendto");
        exit(1);
    }



    printf("Server A has sent a department list to Main Server\n");

    while(1){
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        buf[numbytes] = '\0';


        printf("Server A has received a request for %s\n", buf);
        std::string d=buf;
        std::string num = std::to_string(department_stu[d]);
        if ((numbytes = sendto(sockfd_s, num.c_str(), strlen(num.c_str()), 0, ps->ai_addr, ps->ai_addrlen)) == -1) {
            perror("serverA: sendto");
            exit(1);
        }
        std::string students;
        for(auto& i : stu_department){
            if(i.second==d){
                students+=std::to_string(i.first);
                students+=',';
            }
        }
        if ((numbytes = sendto(sockfd_s, students.c_str(), strlen(students.c_str()), 0, ps->ai_addr, ps->ai_addrlen)) == -1) {
            perror("serverA: sendto");
            exit(1);
        }
        students[strlen(students.c_str())-1]='\0';
        printf("Server A found %s distinct students for %s: %s\n",num.c_str(),buf,students.c_str());
        printf("Server A has sent the results to Main Server\n");

    }



    freeaddrinfo(servinfo);
    freeaddrinfo(servinfo_s);

    close(sockfd);
    close(sockfd_s);
    

    return 0;
}