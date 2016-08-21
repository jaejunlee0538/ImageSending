// PicoloConsole.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <stdint.h>
#include "PicoloImageRead.h"
#include "ImagePacket.h"
#include "ImageServer.h"
#include "PicoloImageServer.h"

CVImagesPacket imagePacket;

int main(int argc, char** argv)
{
	boost::asio::io_service io_service;
	ImageServer server(io_service, 50000);

#define NCamera 1
	MC::Initialize();
	std::vector<std::string> conns{ "VID1", "VID3" };

	PicoloImageSender picolo[NCamera];
	for (int i = 0; i < NCamera; i++){
		picolo[i].setServer(&server);
		if (!picolo[i].init(0, conns[i].c_str(), "NTSC", MC_ColorFormat_Y8)){
			printf("Failed to initialize Picolo Reader\n");
			return 0;
		}
		cv::namedWindow(picolo[i].getConnectorName());
	}
	
	server.startAccept();

	for (int i = 0; i < NCamera; i++){
		picolo[i].setActive();
	}

	printf("Press q or Q to terminate.\n");
	io_service.run();
	printf("IO service stopped\n");
	for (int i = 0; i < NCamera; i++){
		picolo[i].setIdle();
	}
	::_sleep(2);
	for (int i = 0; i < NCamera; i++){
		picolo[i].cleanUp();
	}
	printf("Bye Bye~");
	MC::Terminate();
	return 0;
}

