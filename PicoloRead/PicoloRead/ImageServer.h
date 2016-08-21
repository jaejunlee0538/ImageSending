#ifndef IMAGE_SERVER_H_
#define IMAGE_SERVER_H_
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <queue>
#include <thread>
#include <mutex>
using boost::asio::ip::tcp;
class ImageServer;

class FakeMutex{
public:
	void lock(){}
	void unlock(){}
};
typedef FakeMutex MutexType;

class ImageTransportSession{
public:
	typedef std::vector<uint8_t> BufferType;
	typedef std::shared_ptr<BufferType> BufferTypePtr;

	ImageTransportSession(boost::asio::io_service& io_service, ImageServer* serverPtr);
	~ImageTransportSession(){

	}

	tcp::socket& socket(){
		return m_socket;
	}

	void start();

	void addTask(const BufferTypePtr& taskPtr){
		lock_task.lock();
		taskQ.push(taskPtr);
		lock_task.unlock();
	}

	void destroy();
private:
	void recvHandler(const boost::system::error_code& error,
		size_t bytes_transferred);

	void taskThread();
private:
	enum{ RECV_BUFFER_SIZE = 100 };

	std::queue<const BufferTypePtr> taskQ;
	tcp::socket m_socket;
	tcp::endpoint remote_ep;
	std::shared_ptr<std::thread> m_sendThread;
	bool running;
	char rcvBuffer[RECV_BUFFER_SIZE];
	ImageServer * m_serverPtr;
	MutexType lock_task;
};

class ImageServer
{
public:
	ImageServer(boost::asio::io_service& io_service, short port)
		:m_io_service(io_service), m_acceptor(io_service, tcp::endpoint(tcp::v4(), port))
		, n_sessions(0)
	{

	}

	~ImageServer(){
		m_acceptor.close();
	}

	void stop(){
		m_io_service.stop();
	}

	void startAccept(){
		printf("Waiting for connection...\n");
		ImageTransportSession* session = new ImageTransportSession(m_io_service, this);
		m_acceptor.async_accept(session->socket(),
			boost::bind(&ImageServer::handleAccept, this, session,
			boost::asio::placeholders::error));
		accepting = true;
	}

	void addTask(const ImageTransportSession::BufferTypePtr& imageBufferPtr){
		lock_sessions.lock();
		for (size_t i = 0; i < n_sessions; i++){
			m_sessions[i]->addTask(imageBufferPtr);
		}
		lock_sessions.unlock();
	}

	bool isSessionsFull() const{
		return n_sessions >= MAX_NUM_SESSIONS;
	}

	bool isConnected() const{
		return n_sessions > 0;
	}
private:
	void addSession(ImageTransportSession* session){
		lock_sessions.lock();
		if (n_sessions < MAX_NUM_SESSIONS){
			m_sessions[n_sessions] = session;
			n_sessions++;
		}
		lock_sessions.unlock();
	}

	void removeSession(ImageTransportSession* session){
		lock_sessions.lock();
		bool found = false;
		if (m_sessions[0] == session){
			found = true;
		}
		for (size_t i = 0; i<MAX_NUM_SESSIONS; i++){
			if (found){
				m_sessions[i - 1] = m_sessions[i];
			}else if (m_sessions[i] == session){
				found = true;
			}
		}
		if (found){
			m_sessions[MAX_NUM_SESSIONS - 1] = NULL;
		}
		n_sessions--;

		lock_sessions.unlock();
		if (!accepting){
			startAccept();
		}
	}

	void handleAccept(ImageTransportSession* session, const boost::system::error_code& err){
		accepting = false;

		if (!err){
			tcp::endpoint client = session->socket().remote_endpoint();
			printf("Port %d connected to %s\n", client.port(), client.address().to_string().c_str());
			session->start();
			addSession(session);
		}
		else{
			delete session;
		}
		if (!isSessionsFull()){
			startAccept();
		}
	}
private:
	enum { MAX_NUM_SESSIONS = 4 };

	boost::asio::io_service& m_io_service;
	boost::asio::ip::tcp::acceptor m_acceptor;
	ImageTransportSession* m_sessions[MAX_NUM_SESSIONS];
	size_t n_sessions;
	bool accepting;

	MutexType lock_sessions;

	friend class ImageTransportSession;
};

#endif