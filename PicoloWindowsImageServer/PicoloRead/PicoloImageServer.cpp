#include "PicoloImageServer.h"
CVImagesPacket imagesPacket;

PicoloImageSender::PicoloImageSender(ImageServer* server, const uint32_t& id)
:m_serverPtr(server),m_id(id){

}

bool PicoloImageSender::init(const size_t& boardIdx, const char* connector, const char* camType, const int& pixelType){
	if (!PicoloImageReader::init(boardIdx, connector, camType, pixelType)){
		return false;
	}
	CVMatConfig config;
	config.cols = this->sizeX;
	config.rows = this->sizeY;
	config.type = this->cvPixelType;
	imagesPacket.addImage(config);
}

const std::string& PicoloImageSender::getConnectorName() const {
	return this->connector;
}

void PicoloImageSender::userCallback(std::shared_ptr<cv::Mat>& img, MC::Channel & ch, MC::SignalInfo & sig)
{
	freq.update(1);
	if (m_serverPtr->isConnected()){
		//image packet 객체 생성
		CVImagePacket packet;
		packet.cvImg = img;
		packet.seq = imgCount;
		packet.id = m_id;
		ImageTransportSession::BufferTypePtr buffer(new ImageTransportSession::BufferType());
		buffer->resize(packet.getPacketLength());
		packet.load(&(buffer->operator[](0)));// raw bytes 패킷 생성
		m_serverPtr->addTask(buffer);//image server에게 버퍼 넘겨줌.
	}
	std::ostringstream oss;
	oss << "FPS : " << freq.getFrequency() << " Hz";
	cv::putText(*img, oss.str().c_str(), cv::Point(10, 50), 2, 1.2, cv::Scalar::all(255));
	
	cv::imshow(this->connector.c_str(), *img);
}

