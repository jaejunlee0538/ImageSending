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

void runIoService(boost::asio::io_service& io_service){
	printf("io service starts running\n");
	io_service.run();
	printf("IO service stopped\n");
}

int main(int argc, char** argv)
{
	boost::asio::io_service io_service;
	ImageServer server(io_service, 50000);

#define NCamera 2
	MC::Initialize();
	std::vector<std::string> conns{ "VID1", "VID3" };

	PicoloImageSender picolo[NCamera];
	for (int i = 0; i < NCamera; i++){
		picolo[i].setServer(&server);
		picolo[i].setID(i);
		if (!picolo[i].init(0, conns[i].c_str(), "NTSC", MC_ColorFormat_RGB24)){
			printf("Failed to initialize Picolo Reader\n");
			return 0;
		}
		cv::namedWindow(picolo[i].getConnectorName());
	}
	server.startAccept();

	for (int i = 0; i < NCamera; i++){
		picolo[i].setActive();
	}

	std::thread ioThread(std::bind(runIoService, std::ref(io_service)));
	
	printf("Press q or Q to terminate.\n");
	bool running = true;
	while (running){
		int key = cv::waitKey(1);
		if (key == 'q'){
			running = false;
		}
	}
	for (int i = 0; i < NCamera; i++){
		picolo[i].setIdle();
	}
	server.stop();
	ioThread.join();
	
	for (int i = 0; i < NCamera; i++){
		picolo[i].cleanUp();
	}
	printf("Bye Bye~");
	MC::Terminate();
	return 0;
}

