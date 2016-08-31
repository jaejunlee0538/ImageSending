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


#define PORT_NUMBER	50000	//�̹����� ������ ��Ʈ ��ȣ
std::vector<std::string> conns{ "VID2" };//�׷��� ��ȣ

void runIoService(boost::asio::io_service& io_service){
	printf("io service starts running\n");
	io_service.run();
	printf("IO service stopped\n");
}

int main(int argc, char** argv)
{
	boost::asio::io_service io_service;
	ImageServer server(io_service, PORT_NUMBER);
	char paramBuffer[100];

	MC::Initialize();

	std::cerr << PicoloImageSender::getBoardsInfoString() << std::endl;
	if (PicoloImageSender::getBoardCount() == 0){
		return 0;
	}
	std::vector<PicoloImageSender> picolo(conns.size());
	for (size_t i = 0; i < conns.size(); i++){
		picolo[i].setServer(&server);
		picolo[i].setID(i);
		if (!picolo[i].init(0, conns[i].c_str(), "NTSC", MC_ColorFormat_Y8)){
			printf("Failed to initialize Picolo Reader\n");
			return 0;
		}
		cv::namedWindow(picolo[i].getConnectorName());
	}

	server.startAccept(); 
	printf("Image server started\n");
	
	//���ݷ� �׷��� Ȱ��ȭ
	std::for_each(picolo.begin(), picolo.end(), [](PicoloImageSender& picolo){picolo.setActive(); });

	//boost::io_service run�� ���� ������ 
	std::thread ioThread(std::bind(runIoService, std::ref(io_service)));

	printf("Press q or Q to terminate.\n");
	bool running = true;
	while (running){
		int key = cv::waitKey(1);
		if (key == 'q'){
			running = false;
		}
	}
	//���ݷ� �׷��� ��Ȱ��ȭ
	std::for_each(picolo.begin(), picolo.end(), [](PicoloImageSender& picolo){picolo.setIdle(); });

	//ImageServer ����
	server.stop();

	//io_service ������ ���Ḧ ��ٸ�
	ioThread.join();

	//���ݷ� �׷��� ����
	std::for_each(picolo.begin(), picolo.end(), [](PicoloImageSender& picolo){picolo.cleanUp(); });

	printf("Bye Bye~");
	MC::Terminate();
	return 0;
}

