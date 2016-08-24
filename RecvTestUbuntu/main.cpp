#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "ImagePacket.h"
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <map>
#include <algorithm>
#include "PreciseClock.h"
#include "CVWindow.h"
using boost::asio::ip::tcp;
#define BUFFER_SIZE	4096


int main(int argc ,char** argv)
{
    FrequencyEstimator freq;
    try{
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::socket socket(io_service);
        boost::asio::connect(socket, resolver.resolve({ "192.168.2.9", "50000" }));

        printf("Starting loop...\n");
        std::vector<uint8_t> packet;
        size_t packPos = 0;

        uint8_t buffer[BUFFER_SIZE];
        size_t readLen = 0;
        CVImagePacket imagePacket;
        int key = -1;

        while (true){
            size_t nRecv = boost::asio::read(socket, boost::asio::buffer(&buffer[readLen], BUFFER_SIZE-readLen));

            if (nRecv){
                readLen += nRecv;
                if (readLen < sizeof(uint32_t)){
                    continue;
                }
                if (packPos == 0){
                    uint32_t len = *(uint32_t*)(&buffer[0]);
                    if (packet.size() != len){
                        printf("allocating %u bytes", len);
                        packet.resize(len);
                    }
                }

                size_t more = packet.size() - packPos;
                if (more < readLen){
                    memcpy(&packet[packPos], buffer, more);
                    packPos += more;
                    memcpy(&buffer[0], &buffer[more], readLen - more);
                    readLen -= more;
                    if (packPos == packet.size()){
                        freq.update(1);
//                        std::cerr<<freq.getFrequency()<<std::endl;
                        packPos = 0;
                        imagePacket.unload(&packet[0], packet.size());
                        CVUniqueWindows::imshow(imagePacket.id, *(imagePacket.cvImg));
                        //fprintf(stderr, "%d:%d\t%d X %d\n", imagePacket.id, imagePacket.seq, imagePacket.cvImg->rows, imagePacket.cvImg->cols);
                        int key_tmp = cv::waitKey(1);
                        if(key_tmp!= -1){
                            key= key_tmp;
                            printf("%c pressed\n", key);
                        }
                    }
                }
                else{
                    memcpy(&packet[packPos], buffer, readLen);
                    packPos += readLen;
                    readLen = 0;
                }
            }
            else{
                printf("Disconnected\n");
                break;
            }
            if ((char)key == 'q'){
                break;
            }
            key = -1;

        }

        socket.shutdown(tcp::socket::shutdown_both);
        socket.close();
    }
    catch (std::exception& e){
        std::cerr << "Exception : " << e.what() << std::endl;
    }

    return 0;
}

