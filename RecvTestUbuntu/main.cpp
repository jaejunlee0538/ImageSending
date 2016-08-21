#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "ImagePacket.h"
#define RECV_PORT 45000
#define BUFFER_SIZE 400000

void showErrorMessage(const char* msg){
    fprintf(stderr, "Error : %s\n", msg);
}

int main() {
    int sock;
    ImagePacket packet;
    socklen_t sender_adr_sz;
    sockaddr_in serv_adr, from_adr;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(sock == -1){
        showErrorMessage("socket() error");
        return 0;
    }
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(RECV_PORT);

    if(bind(sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1){
        showErrorMessage("bind() error");
        return 0;
    }
    cv::namedWindow("Picolo");
    cv::waitKey(1);

    fprintf(stderr, "Loop started\n");
    size_t n_recv;
    uint8_t buffer[BUFFER_SIZE];
    bool running = true;
    while(running){
        sender_adr_sz = sizeof(from_adr);
        n_recv = recvfrom(sock, buffer, BUFFER_SIZE, 0, (sockaddr*)&from_adr, &sender_adr_sz);
        packet.unload(buffer, n_recv);
        cv::Mat img(packet.height, packet.width, CV_8U);
        for(int w=0;w<packet.width;w++){
            for(int h=0;h<packet.height;h++){
                img.at<uint8_t>(h, w) = packet.buffer[packet.width*h+w];
            }
        }
        cv::imshow("Picolo", img);
        int key = cv::waitKey(1);

        switch((char)key){
            case 'q': case 'Q':
                running = false;
                printf("Stopping...\n");
                break;
            default:
                printf("default\n");
                break;
        }
    }

    return 0;
}