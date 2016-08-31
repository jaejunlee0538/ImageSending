#include "PicoloReader\ImageServer.h"

ImageTransportSession::ImageTransportSession(boost::asio::io_service& io_service, ImageServer* serverPtr)
:m_socket(io_service), m_serverPtr(serverPtr){

}

ImageTransportSession::~ImageTransportSession(){
	//m_socket.shutdown(tcp::socket::shutdown_both);
	//m_socket.close();
	running = false;
	//if (m_sendThread->joinable())
	m_sendThread->join();
	printf("Session destroied(%s)\n", remote_ep.address().to_string().c_str());
}

void ImageTransportSession::start(){
	remote_ep = m_socket.remote_endpoint();
	m_sendThread.reset(new std::thread(&ImageTransportSession::taskThread, this));
	m_socket.async_read_some(boost::asio::buffer(rcvBuffer, RECV_BUFFER_SIZE),
		boost::bind(&ImageTransportSession::recvHandler, this,
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}


void ImageTransportSession::recvHandler(const boost::system::error_code& error,
	size_t bytes_transferred){
	if (!error){
		printf("%u bytes received(%s)\n", remote_ep.address().to_string().c_str());
	}
	else{
		if (error == boost::asio::error::eof){
			printf("Disconnected(%s) - message(%s)\n", remote_ep.address().to_string().c_str(), error.message().c_str());
		}
		else{
			printf("Error : %s(in %s)\n", error.message().c_str(), remote_ep.address().to_string().c_str());
		}
		m_serverPtr->removeSession(this);
	}
}

void ImageTransportSession::addTask(const BufferTypePtr& taskPtr){
	lock_task.lock();
	taskQ.push(taskPtr);
	lock_task.unlock();
}

void ImageTransportSession::taskThread(){
	printf("Image transport session started(to %s)\n", remote_ep.address().to_string().c_str());
	running = true;
	while (running){
		BufferTypePtr task;
		size_t n_remained;
		lock_task.lock();
		if (!taskQ.empty()){
			task = taskQ.front();
			taskQ.pop();
		}
		n_remained = taskQ.size();
		lock_task.unlock();
		
		if (task){
			try{
				m_socket.send(boost::asio::buffer(*task));
			}
			catch (std::exception& e){
				std::cerr << "Error while calling send : " << e.what() << std::endl;
				break;
			}
		}
		if (n_remained > 5){
			printf("%d frames are delayed to %s\n", n_remained, remote_ep.address().to_string().c_str());
		}
	}
	printf("Image transport session ends(to %s)\n", remote_ep.address().to_string().c_str());
}