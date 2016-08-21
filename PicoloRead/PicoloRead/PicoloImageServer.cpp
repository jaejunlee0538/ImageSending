#include "PicoloImageServer.h"
CVImagesPacket imagesPacket;

PicoloImageSender::PicoloImageSender(ImageServer* server)
:m_serverPtr(server){

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
	std::ostringstream oss;
	oss << "FPS : " << freq.getFrequency() << " Hz";
	cv::putText(*img, oss.str().c_str(), cv::Point(10, 50), 2, 1.2, cv::Scalar::all(255));
	cv::imshow(this->connector.c_str(), *img);
	
	//Stop boost io_service.
	char key = cv::waitKey(1);
	if (key == 'q'){
		printf("%c pressed\n", key);
		m_serverPtr->stop();
	}
}