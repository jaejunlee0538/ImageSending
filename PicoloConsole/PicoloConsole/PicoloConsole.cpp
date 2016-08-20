// PicoloConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <MultiCamCpp.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <stdint.h>

namespace MC=Euresys::MultiCam;

#define SAVE_BITMAP_SEQ 0

class SerializeHelper{
public:
	SerializeHelper(void * outBuffer) 
		:outBuffer((uint8_t*)outBuffer){

	}

	//template <typename T>
	//void load(const T& data){
	//	memcpy(outBuffer, &data, sizeof(T));
	//	outBuffer += sizeof(T);
	//}

	void load(const uint32_t& data){
		memcpy(outBuffer, &data, sizeof(uint32_t));
		outBuffer += sizeof(uint32_t);
	}

	void load(const uint8_t& data){
		memcpy(outBuffer, &data, sizeof(uint8_t));
		outBuffer += sizeof(uint8_t);
	}

	void load(const void* data, const size_t& bytes){
		memcpy(outBuffer, data, bytes);
		outBuffer += bytes;
	}

protected:
	uint8_t * outBuffer;
};

class DeserialzeHelper{
public:
	DeserialzeHelper(void * inBuffer)
		:inBuffer((uint8_t*)inBuffer){

	}

	//template <typename T>
	//void unload(const T& data){
	//	memcpy(&data, inBuffer, sizeof(T));
	//	inBuffer += sizeof(T);
	//}

	void unload(uint8_t& data){
		memcpy(&data, inBuffer, sizeof(uint8_t));
		inBuffer += sizeof(uint8_t);
	}

	void unload(uint32_t& data){
		memcpy(&data, inBuffer, sizeof(uint32_t));
		inBuffer += sizeof(uint32_t);
	}

	void unload(void* data, const size_t& bytes){
		memcpy(data, inBuffer, bytes);
		inBuffer += bytes;
	}
protected:
	uint8_t * inBuffer;
};

struct ImagePacket{
	uint32_t len;//패킷 바이트수(len 포함)
	uint32_t width;
	uint32_t height;
	uint8_t bytes;
	uint8_t * buffer;
	uint32_t bufferLen;

	ImagePacket(){
		len = width = height = bufferLen = 0;
		bytes = 0;
		buffer = NULL;
	}

	ImagePacket(const uint32_t& width, const uint32_t& height, const uint8_t& bytes){
		this->init(width, height, bytes);
	}

	~ImagePacket(){
		delete[] buffer;
	}

	void init(const uint32_t& width, const uint32_t& height, const uint8_t& bytes){
		this->width = width;
		this->height = height;
		this->bytes = bytes;
		this->buffer = NULL;
		this->initBuffer();
	}

	void initBuffer(){
		uint32_t newBufferLen = sizeof(uint8_t)* bytes*width*height;
		if (newBufferLen == bufferLen)
			return;
		if (buffer){
			delete[] buffer;
		}
		bufferLen = newBufferLen;
		buffer = new uint8_t[bufferLen];
		len = sizeof(width) + sizeof(height) +
			sizeof(bytes) + bufferLen;
	}

	void load(void* out){
		SerializeHelper loader(out);
		loader.load(len);
		loader.load(width);
		loader.load(height);
		loader.load(bytes);
		loader.load(buffer, bufferLen);
	}

	void unload(void * in){
		DeserialzeHelper unloader(in);
		unloader.unload(len);
		unloader.unload(width);
		unloader.unload(height);
		unloader.unload(bytes);
		initBuffer();
		unloader.unload(buffer, bufferLen);
	}
};

class PicoloY8ImageSender{
public:
	PicoloY8ImageSender() :channel(NULL), img(NULL), imgCount(0), packetBuffer(NULL){
	
	}

	~PicoloY8ImageSender(){
		cleanUp();
	}

	void cleanUp(){
		if (channel){
			delete channel; 
			channel = NULL;
		}
		if (img){
			delete img;
			img = NULL;
		}
		clearWinSock();
		if (packetBuffer){
			delete[] packetBuffer;
			packetBuffer = NULL;
		}
	}

	bool init(const char* Connector, const char* recvIP, const int& port){
		if (MC::Boards.GetCount() == 0){
			return false;
		}

		if (channel){
			delete channel; 
			channel = NULL;
		}

		channel = new MC::Channel(MC::Boards[0], Connector);

		initCamera();
		initCV();
		initWinSock(recvIP, port);

		imagePacket.init(sizeX, sizeY, 1);
		packetBuffer = new uint8_t[imagePacket.len];
#if SAVE_BITMAP_SEQ
		initBitmap();
#endif
		channel->RegisterCallback(this, &PicoloY8ImageSender::callback, MC_SIG_SURFACE_PROCESSING);
	}

	void initCV(){
		img = new cv::Mat(sizeY, sizeX, CV_8U);
	}

	void callback(MC::Channel & ch, MC::SignalInfo & sig){
		imgCount++;
		MC::Surface* suf = sig.Surf;
		suf->GetParam(MC_SurfaceAddr, pvoid);
#if SAVE_BITMAP_SEQ
		saveBMPSeq();
#endif
		memcpy(imagePacket.buffer, pvoid, imagePacket.bufferLen);
		imagePacket.load(packetBuffer);
		send(sock, (const char*)packetBuffer, imagePacket.len, 0);

		unsigned char * pBuf = static_cast<unsigned char *>(pvoid);
		for (int w = 0; w < sizeX; w++){
			for (int h = 0; h < sizeY; h++){
				img->at<unsigned char>(h, w) = pBuf[h*sizeX + w];
			}
		}
		cv::imshow("Picolo", *img);
	}

	void setActive(){
		if (channel){
			channel->SetActive();
		}
	}

	void setIdle(){
		if (channel){
			channel->SetIdle();
		}
	}

protected:
	void sendCurrentImage(){
		
	}

	void clearWinSock(){
		closesocket(sock);
		WSACleanup();
	}


	void initWinSock(const char* recvIP, const int& port){
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
			throw std::runtime_error("WSAStartup() error!");
		}
		sock = socket(PF_INET, SOCK_DGRAM, 0);
		if (sock == INVALID_SOCKET){
			throw std::runtime_error("socket() error!");
		}
		memset(&recvAddr, 0, sizeof(recvAddr));
		recvAddr.sin_family = AF_INET;
		recvAddr.sin_addr.S_un.S_addr = inet_addr(recvIP);
		recvAddr.sin_port = htons(port);

		connect(sock, (SOCKADDR*)&recvAddr, sizeof(recvAddr));
	}

	void initCamera(){
		channel->SetParam(MC_CamFile, "NTSC");
		channel->SetParam(MC_ColorFormat, MC_ColorFormat_Y8);
		channel->SetParam(MC_AcquisitionMode, MC_AcquisitionMode_VIDEO);
		channel->SetParam(MC_TrigMode, MC_TrigMode_IMMEDIATE);
		channel->SetParam(MC_NextTrigMode, MC_NextTrigMode_REPEAT);
		channel->SetParam(MC_SeqLength_Fr, MC_INDETERMINATE);
		channel->SetParam(MC_SurfaceCount, 10);
		channel->SetParam(MC_SignalEnable + MC_SIG_SURFACE_PROCESSING, MC_SignalEnable_ON);
		channel->SetParam(MC_SignalEnable + MC_SIG_ACQUISITION_FAILURE, MC_SignalEnable_ON);
		channel->SetParam(MC_SignalEnable + MC_SIG_END_CHANNEL_ACTIVITY, MC_SignalEnable_ON);
		channel->GetParam(MC_ImageSizeX, sizeX);
		channel->GetParam(MC_ImageSizeY, sizeY);
		channel->GetParam(MC_BufferPitch, bufferPitch);
	}
#if SAVE_BITMAP_SEQ
	void saveBMPSeq(){
		char fname[20];
		sprintf(fname, "imgs\\Seq%05d.bmp", imgCount);
		FILE* output;
		errno_t err = fopen_s(&output, fname, "wb");
		fwrite(&bmpHeader, sizeof(BITMAPFILEHEADER), 1, output);
		fwrite(&bmpInfo, sizeof(BITMAPINFOHEADER), 1, output);
		fwrite(pallete, sizeof(RGBQUAD), 256, output);
		fwrite(pvoid, 1, sizeX * sizeY, output);
		fclose(output);
	}

	void initBitmap(){
		memset(&bmpHeader, 0, sizeof(BITMAPFILEHEADER));
		memset(&bmpInfo, 0, sizeof(BITMAPINFOHEADER));

		bmpInfo.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.biWidth = sizeX;
		bmpInfo.biHeight = sizeY;
		bmpInfo.biCompression = BI_RGB;
		bmpInfo.biPlanes = 1; //This value must be set to 1.
		bmpInfo.biBitCount = 8; //number of bits per pixel
		bmpInfo.biSizeImage = 0;//zero when BI_RGB bitmaps.
		bmpInfo.biXPelsPerMeter = 0;
		bmpInfo.biYPelsPerMeter = 0;
		bmpInfo.biClrUsed = 0;//color table에서 실제로 사용하는 색의 개수. 0이면 biBitCount에 따라서 결정됨
		bmpInfo.biClrImportant = 0;

		bmpHeader.bfType = 0x4d42;
		bmpHeader.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+256 * sizeof(RGBQUAD)+sizeX*sizeY;
		bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+256 * sizeof(RGBQUAD);

		for (int i = 0; i < 256; i++){
			pallete[i].rgbBlue = i;
			pallete[i].rgbGreen = i;
			pallete[i].rgbRed = i;
			pallete[i].rgbReserved = 0;
		}
	}
	BITMAPFILEHEADER bmpHeader;
	BITMAPINFOHEADER bmpInfo;
	RGBQUAD pallete[256];
#endif
	int sizeX, sizeY, bufferPitch;
	
	void* pvoid;
	cv::Mat* img;

	MC::Channel * channel;

	unsigned long imgCount;

	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN recvAddr;

	ImagePacket imagePacket;
	uint8_t * packetBuffer;
};

int main(int argc, char** argv)
{
	MC::Initialize();
	PicoloY8ImageSender picolo;
	if (!picolo.init("VID1", "172.0.0.1", 50000)){
		printf("Failed to initialize Picolo Reader\n");
		return 0;
	}
	cv::namedWindow("Picolo", CV_WINDOW_AUTOSIZE);
	picolo.setActive();

	printf("Press q or Q to terminate.\n");
	bool running = true;
	while (running){
		int key = cv::waitKey(0);
		switch (key){
		case 'q':case 'Q':
			running = false;
			break;
		default:
			break;
		}
	}
	picolo.setIdle();
	::_sleep(2);
	picolo.cleanUp();
	printf("Bye Bye~");
	MC::Terminate();

	
	return 0;
}

