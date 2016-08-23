#ifndef IMAGE_SERVER_H_
#define IMAGE_SERVER_H_
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <queue>
#include <thread>
#include <mutex>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;
class ImageServer;

class FakeMutex{
public:
	void lock(){}
	void unlock(){}
};
typedef boost::shared_mutex MutexType;

class ImageTransportSession{
public:
	typedef std::vector<uint8_t> BufferType;
	typedef std::shared_ptr<BufferType> BufferTypePtr;

	ImageTransportSession(boost::asio::io_service& io_service, ImageServer* serverPtr);
	~ImageTransportSession();

	tcp::socket& socket(){
		return m_socket;
	}

	void start();

	void addTask(const BufferTypePtr& taskPtr);

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
	FakeMutex lock_task;
};


class ImageServer
{
public:
	ImageServer(boost::asio::io_service& io_service, short port)
		:m_io_service(io_service), m_acceptor(io_service, tcp::endpoint(tcp::v4(), port))
		, n_sessions(0)
	{
		for (size_t i = 0; i < MAX_NUM_SESSIONS; i++){
			m_sessions[i] = NULL;
		}
	}

	~ImageServer(){

	}

	void stop(){
		m_io_service.stop();
		removeAllSessions();
	}

	void startAccept(){
		printf("Waiting for connection...\n");
		ImageTransportSession* session = new ImageTransportSession(m_io_service, this);
		m_acceptor.async_accept(session->socket(),
			boost::bind(&ImageServer::handleAccept, this, session,
			boost::asio::placeholders::error));
		accepting = true;
	}

	void addCamera(){

	}

	void addTask(const ImageTransportSession::BufferTypePtr& imageBufferPtr){
		boost::unique_lock<boost::shared_mutex> uniqLock(lock_sessions);
		for (size_t i = 0; i < n_sessions; i++){
			if (m_sessions[i] != NULL){
				m_sessions[i]->addTask(imageBufferPtr);
			}
		}
	}

	bool isSessionsFull() const{
		boost::shared_lock<boost::shared_mutex> shrdLock(lock_sessions);
		return n_sessions >= MAX_NUM_SESSIONS;
	}

	bool isConnected() const{
		boost::shared_lock<boost::shared_mutex> shrdLock(lock_sessions);
		return n_sessions > 0;
	}
private:
	int findSession(ImageTransportSession* pSession) const {
		for (int i = 0; i < MAX_NUM_SESSIONS; i++){
			if (m_sessions[i] == pSession){
				return i;
			}
		}
		return -1;
	}

	bool addSession(ImageTransportSession* session){
		boost::upgrade_lock<boost::shared_mutex> shrdLock(lock_sessions);
		int emptySession = findSession(NULL);
		if (emptySession < 0){
			return false;
		}
		boost::upgrade_to_unique_lock<boost::shared_mutex> uniqLock(shrdLock);
		m_sessions[emptySession] = session;
		n_sessions++;
		return true;
	}

	void removeAllSessions(){
		boost::unique_lock<boost::shared_mutex> uniqLock(lock_sessions);
		for (size_t i = 0; i < n_sessions; i++){
			if (m_sessions[i]){
				delete m_sessions[i]; m_sessions[i] = NULL;
			}
		}
	}

	void removeSession(ImageTransportSession* session){
		boost::upgrade_lock<boost::shared_mutex> shrdLock(lock_sessions);
		int pos = findSession(session);
		if (pos < 0){
			return;
		}
		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> uniqLock(shrdLock);
			delete session;
			m_sessions[pos] = NULL;
		}
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
	//DummyClass2 __DC2; //아무런 의미 없는 변수이지만 없으면 알수 없는 에러 발생.
	ImageTransportSession* m_sessions[MAX_NUM_SESSIONS];

	size_t n_sessions;
	bool accepting;

	mutable boost::shared_mutex lock_sessions;

	friend class ImageTransportSession;
};

#endif