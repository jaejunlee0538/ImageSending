#ifndef PICOLO_IMAGE_SERVER_H_
#define PICOLO_IMAGE_SERVER_H_
#include "PicoloImageRead.h"
#include <opencv2/opencv.hpp>
#include "ImageServer.h"
#include "ImagePacket.h"
#include "PreciseClock.h"

extern CVImagesPacket imagesPacket;

namespace MC = Euresys::MultiCam;

class PicoloImageSender :public PicoloImageReader{
public:
	PicoloImageSender() :m_serverPtr(NULL), m_id(0){

	}

	PicoloImageSender(ImageServer* server, const uint32_t& id);

	void setServer(ImageServer* server){
		m_serverPtr = server;
	}

	void setID(const uint32_t& id){
		m_id = id;
	}

	virtual bool init(const size_t& boardIdx, const char* connector, const char* camType, const int& pixelType);

	const std::string& getConnectorName()const;
protected:
	virtual void userCallback(std::shared_ptr<cv::Mat>& img, MC::Channel & ch, MC::SignalInfo & sig);

	std::string windowName;
	FrequencyEstimator freq;
	ImageServer * m_serverPtr;
	uint32_t m_id;
};
#endif