#include "PicoloReader\PicoloImageServer.h"

bool PicoloImageSender::init(const size_t& boardIdx, const char* connectorName, const char* camType, const int& pixelType){
	if (!PicoloImageReader::init(boardIdx, connectorName, camType, pixelType)){
		return false;
	}
	CVMatConfig config;
	config.cols = this->sizeX;
	config.rows = this->sizeY;
	config.type = this->cvPixelType;
	return true;
}



void PicoloImageSender::userCallback(std::shared_ptr<cv::Mat>& img, MC::Channel & ch, MC::SignalInfo & sig)
{
	freq.update(1);
	if (m_serverPtr && m_serverPtr->isConnected()){
		//image packet ��ü ����
		CVImagePacket packet;
		packet.cvImg = img;
		packet.seq = imgCount;
		packet.id = m_id;
		ImageTransportSession::BufferTypePtr buffer(new ImageTransportSession::BufferType());
		buffer->resize(packet.getPacketLength());
		packet.load(&(buffer->operator[](0)));// raw bytes ��Ŷ ����
		m_serverPtr->addTask(buffer);//image server���� ���� �Ѱ���.
	}
	std::ostringstream oss;
	oss << "FPS : " << freq.getFrequency() << " Hz";
	cv::putText(*img, oss.str().c_str(), cv::Point(10, 50), 2, 1.2, cv::Scalar::all(255));
	
	cv::imshow(this->connector.c_str(), *img);
}


