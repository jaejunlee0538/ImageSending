#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <map>
#include <algorithm>
#include <picolo_image_client/ImagePacket.h>
#include <picolo_image_client/PreciseClock.h>
#include <picolo_image_client/PublishHelper.h>
#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/Image.h>
using boost::asio::ip::tcp;
#define BUFFER_SIZE	4096
inline bool isInSet(std::set<int>& mySet, const int& id ){
    std::set<int>::iterator iter = mySet.find(id);
    return iter != mySet.end();
}

void convert(cv::Mat& mat, sensor_msgs::ImagePtr& image, const std::string& frame_id, const ros::Time& stamp){
    std::string enc;
    switch(mat.type()){
        case CV_8UC1:
            enc = sensor_msgs::image_encodings::TYPE_8UC1;
            break;
        case CV_8UC2:
            enc = sensor_msgs::image_encodings::TYPE_8UC2;
            break;
        case CV_8UC3:
            enc = sensor_msgs::image_encodings::TYPE_8UC3;
            break;
        case CV_8UC4:
            enc = sensor_msgs::image_encodings::TYPE_8UC4;
            break;
        default:
        throw std::runtime_error("Undefined image encoding");
    }
    std_msgs::Header header;
    header.frame_id = frame_id;
    header.stamp = stamp;
    image = cv_bridge::CvImage(header, enc, mat).toImageMsg();
}

int main(int argc ,char** argv)
{
    ros::init(argc, argv, "PicoloImageClient");
    ros::NodeHandle nh;
    ros::NodeHandle p_nh("~");
    std::vector<ros::Publisher> imagePublishers;
    std::set<int> imageIDs;

    FrequencyEstimator freq;
    ClockTimeout timeout(0.5);
    PublishersManager<sensor_msgs::Image> pubManager;
    ros::Rate rate(200);
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
//                        std::cerr<<freq.getFrequency()<<std::endl;
                        packPos = 0;
                        imagePacket.unload(&packet[0], packet.size());
                        if(!isInSet(imageIDs, imagePacket.id)){
                            imageIDs.insert(imagePacket.id);
                            std::ostringstream oss;
                            oss<<"image/picolo/"<<imagePacket.id;
                            pubManager.addTopic(oss.str(), 10, imagePacket.id);
                        }
                        sensor_msgs::ImagePtr imagePtr;
                        convert(*(imagePacket.cvImg), imagePtr, pubManager.getTopic(imagePacket.id) ,ros::Time::now());
                        pubManager.publish(imagePacket.id, imagePtr);

                        rate.sleep();
                        if(timeout.isTimeout()){
                            std::ostringstream oss;
                            std::set<int>::iterator iter = imageIDs.begin();
                            for(; iter!=imageIDs.end();iter++){
                                oss<<pubManager.getTopic(*iter)<<":"<<pubManager.getFrequency(*iter)<<" Hz"<<std::endl;
                            }
                            oss<<std::endl;
                            std::cerr<<oss.str();
                            timeout.resetTimeout();
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
        }
        socket.shutdown(tcp::socket::shutdown_both);
        socket.close();
    }
    catch (std::exception& e){
        std::cerr << "Exception : " << e.what() << std::endl;
    }

    return 0;
}

