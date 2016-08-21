#ifndef IMAGE_PACKET_H_
#define IMAGE_PACKET_H_
#include <stdlib.h>
#include <stdint.h>
#include "SerializeHelper.h"
#include <opencv2/core.hpp>
#include <vector>

/*
이미지 보내는 부분
	CVImagesPacket packet;
	packet.init({640,720,CV_8U}, 4);
	1. from cv::Mat
		cv::Mat img[4];
		//acquiring 4 images
		for(size_t i=0;i<4;i++){
			packet.setImage(i, img[i]);
		}
	2. from raw buffer
		uint8_t * buffer[4];
		//allocate and fill the buffers
		for(size_t i=0;i<4;i++){
			packet.setImage(i, buffer[i]);//
		}

	uint8_t * packetBuffer = packet.allocatePacketMemory();
	packet.load(packetBuffer);

	send(packetBuffer, packet.length()...);

	///////////////////////////////////////////////////////
이미지 받는 부분
	std::string buffer;
	//receive packet
	packet.unload(buffer.c_str(), buffer.length());
*/
struct CVMatConfig{
	int cols;
	int rows;
	int type;
};

class CVImagesPacket{
public:
	CVImagesPacket(){
		len = -1;
		seq = -1;
		std::string str;
	}

	~CVImagesPacket(){

	}

	uint8_t* allocatePacketMemory() const{
		return new uint8_t[len];
	}

	const uint32_t& length()const {
		return len;
	}

	void setSequence(const uint32_t& seq) {
		this->seq = seq;
	}

	void addImage(const CVMatConfig& config){
		images.push_back(cv::Mat());
		images.back().create(config.rows, config.cols, config.type);
		computePacketLength();
	}

	void init(const std::vector<CVMatConfig>& configs){
		images.resize(configs.size());
		for (size_t i = 0; i < configs.size(); i++){
			images[i].create(configs[i].rows, configs[i].cols, configs[i].type);
		}
		computePacketLength();
	}

	void init(const CVMatConfig& config, const uint32_t& nImg){
		images.resize(nImg);
		for (size_t i = 0; i < nImg; i++){
			images[i].create(config.rows, config.cols, config.type);
		}
		computePacketLength();
	}

	const cv::Mat& getImage(const size_t& idx) const{
		return images[idx];//out of range 에러 발생 가능.
	}

	/*
	호출 전에 반드시 init method를 통해 내부 images가 올바르게 초기화되어 있어야 한다.
	*/
	bool setImage(const size_t& idx, const uint8_t* buffer){
		if (idx >= images.size()){
			//out of index
			return false;
		}
		memcpy(images[idx].data, buffer, images[idx].total()*images[idx].elemSize());
	}

	bool setImage(const size_t& idx, const cv::Mat& img){
		if (idx >= images.size()){
			//out of index
			return false;
		}
		images[idx] = img;
		return true;
	}

	void load(void* out){
		uint32_t n = images.size();
		int32_t cols, rows, type;
		SerializeHelper loader(out);
		loader.load(len);
		loader.load(seq);
		loader.load(n);
		for (size_t i = 0; i < n; i++){
			cols = images[i].cols;
			rows = images[i].rows;
			type = images[i].type();
			loader.load(cols);
			loader.load(rows);
			loader.load(type);
			loader.load(images[i].data, cvBufferLength(images[i]));
		}
	}

	bool unload(const void * in, const uint32_t& packetLen){
		uint32_t n; //이미지 수
		int32_t cols, rows, type;

		DeserialzeHelper unloader(in);
		unloader.unload(len);
		if (len != packetLen){
			return false;
		}
		unloader.unload(seq);
		unloader.unload(n);
		images.resize(n);
		for (uint32_t i = 0; i < n; i++){
			unloader.unload(cols);
			unloader.unload(rows);
			unloader.unload(type);
			images[i].create(cols, rows, type);
			unloader.unload(images[i].data, cvBufferLength(images[i]));
		}
		return true;
	}
protected:
	size_t cvBufferLength(const cv::Mat& mat) const{
		return mat.total()*mat.elemSize();
	}

	void computePacketLength(){
		len = 0;
		len += sizeof(seq);
		for (size_t i = 0; i < images.size(); i++){
			len += 3 * sizeof(int32_t);//cols, rows, type 데이터
			len += cvBufferLength(images[i]); //버퍼 사이즈
		}
	}
protected:
	uint32_t len;//패킷 바이트수(len 포함)
	uint32_t seq;//packet seq
	std::vector<cv::Mat> images;//cv::Mat array
};
#endif