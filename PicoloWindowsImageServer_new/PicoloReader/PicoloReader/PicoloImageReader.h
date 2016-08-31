#pragma once
#include <MultiCamCpp.h>
#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <vector>
namespace MC = Euresys::MultiCam;

/*
Picolo �׷��� ����� ���� �̹����� ȹ���ϱ� ���� �������̽� ����.
�� Ŭ������ ��� �ް� userCallback �޼��带 �������̵��Ͽ� ����Ѵ�.

���� ���� ���� : 
	1.	init �޼���� �ݵ�� Euresys::MultiCam::MultiCam::Initialize() �Լ��� ȣ���� �Ŀ� ȣ��Ǿ�� �Ѵ�.
	2.	cleanup()�̳� ~PicoloImageReader()�� �ݵ�� Euresys::MultiCam::MultiCam::Terminate()�Լ��� ȣ���� �Ŀ� ȣ��Ǿ�� �Ѵ�.
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
	���� PC�� ����� Picolo ������ ������ ��ȯ�Ѵ�.
	*/
	static size_t getBoardCount();

	/*
	���� ī�޶�� OpenCV ���̺귯�� ���� ������ �ʱ�ȭ �ϰ� �ݹ��Լ��� ����Ѵ�.
	�ݵ�� Euresys::MultiCam::MultiCam::Initialize() �Լ��� ȣ���� �Ŀ� ȣ��Ǿ�� �Ѵ�.
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
	����� ���� �ݹ��Լ�.
	�� Ŭ������ ��ӹ��� Ŭ�������� �������̵��Ͽ� ���.
	���� ���, ������ opencv �����쿡 ����ϰ� �ʹٸ� ������ ���� �ۼ��Ѵ�(cv::waitKey�� ������ ȣ��).
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
	connector ������
	*/
	const std::string& getConnectorName()const;

	/*
	PC�� ����� ��� Picolo Board�� ������ boardsInfo �Ķ���͸� ���� ��ȯ.
	*/
	static void getBoardsInfo(std::vector<BoardInfo>& boardsInfo);
	
	/*
	PC�� ����� ��� Picolo Board�� ������ string���� ��ȯ
	�ܼ�â�̳� �޽��� �ڽ����� ����� �����ϰ� �ϱ� ����.
	*/
	static std::string getBoardsInfoString();

private:
	/*
	���ο� ���� ������ ���ŵ� ������ ȣ�� ��.
	���� ������ cv::Mat�� shared pointer�� ��ȯ�Ǿ� userCallback���� �Ѱ�����.
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
	OpenCV ���� ���� �ʱ�ȭ
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
	ī�޶� �ʱ�ȭ
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
