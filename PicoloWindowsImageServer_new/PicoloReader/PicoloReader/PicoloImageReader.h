#pragma once
#include <MultiCamCpp.h>
#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <vector>
namespace MC = Euresys::MultiCam;

/*
Picolo 그래버 보드로 부터 이미지를 획득하기 위한 인터페이스 제공.
본 클래스를 상속 받고 userCallback 메서드를 오버라이딩하여 사용한다.

사용시 주의 사항 : 
	1.	init 메서드는 반드시 Euresys::MultiCam::MultiCam::Initialize() 함수를 호출한 후에 호출되어야 한다.
	2.	cleanup()이나 ~PicoloImageReader()는 반드시 Euresys::MultiCam::MultiCam::Terminate()함수를 호출한 후에 호출되어야 한다.
*/
class PicoloImageReader{
public:
	struct BoardInfo{
		int boardIndex;
		std::string boardName;
		std::string boardType;
	};


	PicoloImageReader() :imgCount(0){

	}


	virtual ~PicoloImageReader(){
		cleanUp();
	}

	/*
	
	*/
	void cleanUp(){
		channel.reset();
	}

	/*
	현재 PC에 연결된 Picolo 보드의 개수를 반환한다.
	*/
	static size_t getBoardCount();

	/*
	실제 카메라와 OpenCV 라이브러리 관련 변수를 초기화 하고 콜백함수를 등록한다.
	반드시 Euresys::MultiCam::MultiCam::Initialize() 함수를 호출한 후에 호출되어야 한다.
	*/
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
		return true;
	}

	/*
	사용자 정의 콜백함수.
	본 클래스를 상속받은 클래스에서 오버라이딩하여 사용.
	예를 들어, 영상을 opencv 윈도우에 출력하고 싶다면 다음과 같이 작성한다(cv::waitKey는 별도로 호출).
	void userCallback(std::shared_ptr<cv::Mat>& img, MC::Channel & ch, MC::SignalInfo & sig){
		cv::imshow(img);
	}
	*/
	virtual void userCallback(std::shared_ptr<cv::Mat>& img, MC::Channel & ch, MC::SignalInfo & sig){}

	/*
	*/
	void setActive(){
		if (channel){
			channel->SetActive();
		}
	}

	/*
	*/
	void setIdle(){
		if (channel){
			channel->SetIdle();
		}
	}

	/*
	connector 접근자
	*/
	const std::string& getConnectorName()const;

	/*
	PC에 연결된 모든 Picolo Board의 정보를 boardsInfo 파라미터를 통해 반환.
	*/
	static void getBoardsInfo(std::vector<BoardInfo>& boardsInfo);
	
	/*
	PC에 연결된 모든 Picolo Board의 정보를 string으로 반환
	콘솔창이나 메시지 박스로의 출력을 간편하게 하기 위함.
	*/
	static std::string getBoardsInfoString();

private:
	/*
	새로운 영상 정보가 갱신될 때마다 호출 됨.
	영상 정보는 cv::Mat의 shared pointer로 변환되어 userCallback으로 넘겨진다.
	*/
	void callback(MC::Channel & ch, MC::SignalInfo & sig){
		imgCount++;
		MC::Surface* suf = sig.Surf;
		void* buffer;
		suf->GetParam(MC_SurfaceAddr, buffer);
		std::shared_ptr<cv::Mat> img(new cv::Mat(sizeY, sizeX, cvPixelType));
		memcpy(img->data, buffer, img->total()*img->elemSize());
		//printf("%d\n", imgCount);
		this->userCallback(img, ch, sig);
	}
protected:
	/*
	OpenCV 관련 변수 초기화
	*/
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

	/*
	카메라 초기화
	*/
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
