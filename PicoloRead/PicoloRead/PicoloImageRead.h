#pragma once
#include <MultiCamCpp.h>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
namespace MC = Euresys::MultiCam;

class PicoloImageReader{
public:
	PicoloImageReader() :imgCount(0){

	}

	virtual ~PicoloImageReader(){
		cleanUp();
	}

	void cleanUp(){
		channel.reset();
	}

	static size_t getBoardCount();

	//전체 초기화
	virtual bool init(const size_t& boardIdx ,const char* connector,const char* camType, const int& pixelType){
		this->boardIdx = boardIdx;
		this->connector = connector;
		this->CamType = camType;
		this->pixelType = pixelType;
		if (!initCamera()){
			return false;
		}
		if (!initOpenCV()){
			return false;
		}
		channel->RegisterCallback(this, &PicoloImageReader::callback, MC_SIG_SURFACE_PROCESSING);
	}

	virtual void userCallback(std::shared_ptr<cv::Mat>& img, MC::Channel & ch, MC::SignalInfo & sig){}

	//이미지 수집 시작
	void setActive(){
		if (channel){
			channel->SetActive();
		}
	}

	//이미지 수집 중지
	void setIdle(){
		if (channel){
			channel->SetIdle();
		}
	}
private:
	//콜백
	void callback(MC::Channel & ch, MC::SignalInfo & sig){
		imgCount++;
		MC::Surface* suf = sig.Surf;
		void* buffer;
		suf->GetParam(MC_SurfaceAddr, buffer);
		std::shared_ptr<cv::Mat> img(new cv::Mat(sizeY, sizeX, cvPixelType));
		memcpy(img->data, buffer, img->total()*img->elemSize());
		printf("%d\n", imgCount);
		this->userCallback(img, ch, sig);
	}
protected:
	//OpenCV 초기화
	bool initOpenCV(){
		switch (pixelType){
		case MC_ColorFormat_Y8:
			cvPixelType = CV_8UC1;
			break;
		case MC_ColorFormat_RGB24:
			cvPixelType = CV_8UC3;
			break;
		case MC_ColorFormat_RGB32:
			cvPixelType = CV_8UC4;
			break;
		default:
			return false;
		}
		return true;
	}

	//카메라 초기화
	bool initCamera(){
		channel.reset(new MC::Channel(MC::Boards[boardIdx], connector.c_str()));
		channel->SetParam(MC_CamFile, CamType.c_str());
		channel->SetParam(MC_ColorFormat, pixelType);
		channel->SetParam(MC_AcquisitionMode, MC_AcquisitionMode_VIDEO);
		channel->SetParam(MC_TrigMode, MC_TrigMode_IMMEDIATE);
		channel->SetParam(MC_NextTrigMode, MC_NextTrigMode_REPEAT);
		channel->SetParam(MC_SeqLength_Fr, MC_INDETERMINATE);
		channel->SetParam(MC_SurfaceCount, 6);
		channel->SetParam(MC_SignalEnable + MC_SIG_SURFACE_PROCESSING, MC_SignalEnable_ON);
		channel->SetParam(MC_SignalEnable + MC_SIG_ACQUISITION_FAILURE, MC_SignalEnable_ON);
		channel->SetParam(MC_SignalEnable + MC_SIG_END_CHANNEL_ACTIVITY, MC_SignalEnable_ON);
		channel->GetParam(MC_ImageSizeX, sizeX);
		channel->GetParam(MC_ImageSizeY, sizeY);
		channel->GetParam(MC_BufferPitch, bufferPitch);
		return true;
	}

	std::string CamType;
	std::string connector;
	int pixelType;
	size_t boardIdx;

	std::string windowName;
	int cvPixelType;
	
	int sizeX, sizeY, bufferPitch;
	std::shared_ptr<MC::Channel> channel;
	unsigned long imgCount;
};
