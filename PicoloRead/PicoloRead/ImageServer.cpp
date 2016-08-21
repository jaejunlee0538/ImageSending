#include "ImageServer.h"

ImageTransportSession::ImageTransportSession(boost::asio::io_service& io_service, ImageServer* serverPtr)
:m_socket(io_service), m_serverPtr(serverPtr){

}


void ImageTransportSession::start(){
	remote_ep = m_socket.remote_endpoint();
	m_sendThread.reset(new std::thread(&ImageTransportSession::taskThread, this));
	m_socket.async_receive(boost::asio::buffer(rcvBuffer, RECV_BUFFER_SIZE),
		boost::bind(&ImageTransportSession::recvHandler, this,
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void ImageTransportSession::destroy(){
	m_serverPtr->removeSession(this);
	m_socket.shutdown(tcp::socket::shutdown_both);
	m_socket.close();
	if (m_sendThread->joinable())
		m_sendThread->join();
	delete this;
}

void ImageTransportSession::recvHandler(const boost::system::error_code& error,
	size_t bytes_transferred){
	if (!error){
		if (bytes_transferred == 0){
			printf("Disconnected(%s)\n", remote_ep.address().to_string().c_str());
			destroy();
		}
	}
	else{
		printf("Error : %s(in %s)\n", error.message().c_str(), remote_ep.address().to_string().c_str());
		destroy();
	}
}

void ImageTransportSession::taskThread(){
	printf("Image transport session started(to %s)\n", remote_ep.address().to_string().c_str());
	while (m_socket.is_open()){
		BufferTypePtr task;
		lock_task.lock();
		if (!taskQ.empty()){
			task = taskQ.front();
			taskQ.pop();
		}
		lock_task.unlock();
		
		if (task){
			m_socket.send(boost::asio::buffer(*task));
		}
	}
	printf("Image transport session ends(to %s)\n", remote_ep.address().to_string().c_str());
}