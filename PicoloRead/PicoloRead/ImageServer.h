#ifndef IMAGE_SERVER_H_
#define IMAGE_SERVER_H_
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <queue>
#include <thread>
using boost::asio::ip::tcp;

class Session{
public:
	typedef std::vector<uint8_t> BufferType;
	typedef std::shared_ptr<BufferType> BufferTypePtr;

	Session(boost::asio::io_service& io_service)
		:m_socket(io_service){
		
	}

	tcp::socket& socket(){
		return m_socket;
	}

	void start(){
		m_sendThread.reset(new std::thread(&Session::taskThread, this));
	}

	void addTask(const BufferTypePtr& taskPtr){
		taskQ.push(taskPtr);
	}

private:
	void taskThread(){
		tcp::endpoint remote_ep = m_socket.remote_endpoint();
		printf("Image transport session started(to %s)\n", remote_ep.address().to_string().c_str());
		while (m_socket.is_open()){
			if (!taskQ.empty()){
				BufferTypePtr task = taskQ.front();
				taskQ.pop();
				m_socket.send(boost::asio::buffer(*task));
			}
		}
		printf("Image transport session ends(to %s)\n", remote_ep.address().to_string().c_str());
	}
private:
	std::queue<const BufferTypePtr> taskQ;
	tcp::socket m_socket;
	std::shared_ptr<std::thread> m_sendThread;
	bool running;
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

	}

	void stop(){
		m_io_service.stop();
	}

	void startAccept(){
		printf("Waiting for connection...\n");
		Session* session = new Session(m_io_service);
		m_acceptor.async_accept(session->socket(),
			boost::bind(&ImageServer::handleAccept, this, session,
			boost::asio::placeholders::error));
	}

	void addTask(const Session::BufferTypePtr& imageBufferPtr){
		for (size_t i = 0; i < n_sessions; i++){
			m_sessions[i]->addTask(imageBufferPtr);
		}
	}

	bool isSessionsFull() const{
		return n_sessions >= MAX_NUM_SESSIONS;
	}
private:
	void addSession(Session* session){
		if (n_sessions < MAX_NUM_SESSIONS){
			m_sessions[n_sessions] = session;
			n_sessions++;
		}
	}

	void removeSession(Session* session){
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

		startAccept();
	}

	void handleAccept(Session* session, const boost::system::error_code& err){
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
	Session* m_sessions[MAX_NUM_SESSIONS];
	size_t n_sessions;
};

#endif