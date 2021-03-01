#include <sdkddkver.h>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <iostream>
using namespace std;

const char UDP_IP[] = "127.0.0.1";
const unsigned short SERVER_PORT_NUMBER = 31400;
const unsigned short CLIENT_PORT_NUMBER = 31401;

class UDP_Client // UDP Client
{
public:
	boost::asio::ip::udp::socket m_Socket;
	boost::asio::ip::udp::endpoint m_SenderEndpoint; // 서버의 주소 (UDP 특성상 연결하지 않은 상태에서 상대방과 통신하기에 상대방의 주소가 필요함)

	int m_nSeqNumber;
	std::array<char, 128> m_ReceiveBuffer;
	std::string m_WriteMessage;

	UDP_Client(boost::asio::io_service& io_service) :m_Socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), CLIENT_PORT_NUMBER)), m_nSeqNumber(0) {} // io_service에 클라이언트 포트 할당

	void PostWrite() // 데이터 전송
	{
		if (m_nSeqNumber >= 7)
			return; // 1부터 7까지 전송

		++m_nSeqNumber;

		char szMessage[128] = { 0, };
		sprintf_s(szMessage, 128 - 1, "%d - Send Message", m_nSeqNumber);

		m_WriteMessage = szMessage; // 버퍼에 메시지 복사

		m_Socket.async_send_to(boost::asio::buffer(m_WriteMessage), boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(UDP_IP), SERVER_PORT_NUMBER), boost::bind(&UDP_Client::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		/*
				UDP는 연결(동기화)되지 않은 상태에서 데이터를 주고받기 때문에, 보낼 곳의 주소(_to)를 알고 있어야 한다.
				async_send_to의 인자는 발신용 버퍼, 보낼 주소(endpoint), 바인딩 함수로 구성된다.
		*/
		PostReceive(); // 에코 데이터 수신
	}

	void PostReceive()
	{
		memset(&m_ReceiveBuffer, '\0', sizeof(m_ReceiveBuffer)); // 수신용 버퍼 초기화
		m_Socket.async_receive_from(boost::asio::buffer(m_ReceiveBuffer), m_SenderEndpoint, boost::bind(&UDP_Client::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		/*
			UDP의 특성상 연결하지 않은 상태에서 데이터를 주고받기 때문에, 어디서 데이터를 보냈는지 알 수 없다.
			그래서 m_SenderEndpoint에 보낸 측의 주소를 미리 저장해놔야 한다.

			async_receive_from는 UDP 상에서 데이터를 수신할 때 사용되는 함수다.
			인자는 수신 버퍼, 발신자 IP(저장용), 랩핑 함수로 구성된다
		*/
	}

	void handle_write(const boost::system::error_code&, size_t) {}

	void handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
	{
		if (error)
		{
			std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
		}
		else
		{
			const std::string strRecvMessage = m_ReceiveBuffer.data(); // 수신용 버퍼의 메시지 출력
			cout << "서버에서 받은 메시지 : " << strRecvMessage << ", 받은 크기 : " << bytes_transferred << endl;

			PostWrite(); // 다음 숫자(메시지) 전송
		}
	}
};



int main()
{
	boost::asio::io_context io_service; // I/O 이벤트를 OS에서 디스패치하는 객체

	UDP_Client client(io_service); // UDP 프로토콜 지정 및 포트 할당
	client.PostWrite(); // 서버 접속 및 데이터 발신
	io_service.run();

	cout << "네트워크 접속 종료" << endl;

	// UDP 프로토콜 특성상 서버의 accept와 클라이언트의 connect가 필요 없고, UDP용 송수신 함수를 사용하여 write와 read를 구현한다는 점이 특징이다.

}