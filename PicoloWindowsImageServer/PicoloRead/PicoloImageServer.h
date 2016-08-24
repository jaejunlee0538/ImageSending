#ifndef PICOLO_IMAGE_SERVER_H_
#define PICOLO_IMAGE_SERVER_H_
#include "PicoloImageRead.h"
#include <opencv2/opencv.hpp>
#include "ImageServer.h"
#include "ImagePacket.h"
#include "PreciseClock.h"

namespace MC = Euresys::MultiCam;

/*
setServer와 init 메서드는 setActive 메서드를 호출하여 이미지를 수집하기 전에 
반드시 호출하여야 한다.
*/
class PicoloImageSender :public PicoloImageReader{
public:

	PicoloImageSender() :m_serverPtr(NULL), m_id(-1){

	}
	
	/*
	이미지 서버의 포인터를 설정함.
	새로운 영상 정보가 갱신 될 때마다 이미지 서버에 영상 데이터를 넘겨줌.
	서버는 넘겨 받은 영상 정보를 네트워크로 전송함.
	*/
	void setServer(ImageServer* server){
		m_serverPtr = server;
	}

	/*
	사용자 지정 카메라 ID를 설정함.
	영상을 네트워크로 전송 시에 수신 측에서 각 영상의 소스(각각의 카메라)를 구별하기 위해서 사용함.
	*/
	void setID(const uint32_t& id){
		m_id = id;
	}

	/*
	Picolo board와 카메라를 초기화함.
	*/
	virtual bool init(const size_t& boardIdx, const char* connectorName, const char* camType, const int& pixelType);


protected:
	virtual void userCallback(std::shared_ptr<cv::Mat>& img, MC::Channel & ch, MC::SignalInfo & sig);

	std::string windowName;
	FrequencyEstimator freq;
	ImageServer * m_serverPtr;
	uint32_t m_id;//카메라 ID(User-Defined)
};
#endif