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
#include <time.h> 
#include "RandomGenerator.h"
#include "ImagePacket.h"

namespace MC=Euresys::MultiCam;

#define SAVE_IMAGE_SEQ 0

class PicoloY8ImageSender{
public:
	PicoloY8ImageSender() :channel(NULL), img(NULL), imgCount(0), packetBuffer(NULL){
		srand(time(NULL));
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

		channel->RegisterCallback(this, &PicoloY8ImageSender::callback, MC_SIG_SURFACE_PROCESSING);
	}

	void initCV(){
		img = new cv::Mat(sizeY, sizeX, CV_8U);
	}

	void callback(MC::Channel & ch, MC::SignalInfo & sig){
		imgCount++;
		MC::Surface* suf = sig.Surf;
		suf->GetParam(MC_SurfaceAddr, pvoid);
#if SAVE_IMAGE_SEQ
		saveImageSeq();
#endif
		//memcpy(imagePacket.buffer, pvoid, imagePacket.bufferLen);
		/////////////////////////////////
		RandomGenerator rg;
		rg.initIntDistribution(0, 255);
		for (int i = 0; i < imagePacket.bufferLen; i++){
			imagePacket.buffer[i] = rg.randInt();
		}
		/////////////////////////////////
		imagePacket.load(packetBuffer);
		send(sock, (const char*)packetBuffer, imagePacket.len, 0);
		
		for (int w = 0; w < sizeX; w++){
			for (int h = 0; h < sizeY; h++){
				img->at<unsigned char>(h, w) = imagePacket.buffer[h*sizeX + w];
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
		//channel->GetParam(MC_ImageSizeX, sizeX);
		//channel->GetParam(MC_ImageSizeY, sizeY);
		sizeX = 200;
		sizeY = 300;
		channel->GetParam(MC_BufferPitch, bufferPitch);
	}
#if SAVE_IMAGE_SEQ
	void saveImageSeq(){
		char fname[20];
		sprintf(fname, "imgs\\Seq%05d.png", imgCount);
		cv::imwrite(fname, *img);
	}
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
	if (!picolo.init("VID1", "192.168.2.14", 45000)){
		printf("Failed to initialize Picolo Reader\n");
		return 0;
	}
	cv::namedWindow("Picolo", CV_WINDOW_AUTOSIZE);
	picolo.setActive();
	
	printf("Press q or Q to terminate.\n");
	bool running = true;
	while (running){
		int key = cv::waitKey(1);
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

