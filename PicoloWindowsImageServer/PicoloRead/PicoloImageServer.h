#ifndef PICOLO_IMAGE_SERVER_H_
#define PICOLO_IMAGE_SERVER_H_
#include "PicoloImageRead.h"
#include <opencv2/opencv.hpp>
#include "ImageServer.h"
#include "ImagePacket.h"
#include "PreciseClock.h"

namespace MC = Euresys::MultiCam;

/*
setServer�� init �޼���� setActive �޼��带 ȣ���Ͽ� �̹����� �����ϱ� ���� 
�ݵ�� ȣ���Ͽ��� �Ѵ�.
*/
class PicoloImageSender :public PicoloImageReader{
public:

	PicoloImageSender() :m_serverPtr(NULL), m_id(-1){

	}
	
	/*
	�̹��� ������ �����͸� ������.
	���ο� ���� ������ ���� �� ������ �̹��� ������ ���� �����͸� �Ѱ���.
	������ �Ѱ� ���� ���� ������ ��Ʈ��ũ�� ������.
	*/
	void setServer(ImageServer* server){
		m_serverPtr = server;
	}

	/*
	����� ���� ī�޶� ID�� ������.
	������ ��Ʈ��ũ�� ���� �ÿ� ���� ������ �� ������ �ҽ�(������ ī�޶�)�� �����ϱ� ���ؼ� �����.
	*/
	void setID(const uint32_t& id){
		m_id = id;
	}

	/*
	Picolo board�� ī�޶� �ʱ�ȭ��.
	*/
	virtual bool init(const size_t& boardIdx, const char* connectorName, const char* camType, const int& pixelType);


protected:
	virtual void userCallback(std::shared_ptr<cv::Mat>& img, MC::Channel & ch, MC::SignalInfo & sig);

	std::string windowName;
	FrequencyEstimator freq;
	ImageServer * m_serverPtr;
	uint32_t m_id;//ī�޶� ID(User-Defined)
};
#endif