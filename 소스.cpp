#include <sdkddkver.h>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include<iostream>
using namespace std;

const char SERVER_IP[] = "127.0.0.1";
const unsigned short PORT_NUMBER = 31400;

class TCP_Client // 클라이언트 클래스
{
public:
	boost::asio::io_service& m_io_service; // I/O 이벤트를 OS에서 디스패치하는 객체
	boost::asio::ip::tcp::socket m_Socket; // 클라이언트의 소켓(정보)
	
	int m_nSeqNumber;
	array<char, 128> m_ReceiveBuffer;
	string m_WriteMessage;

	TCP_Client(boost::asio::io_service& io_service) : m_io_service(io_service), m_Socket(io_service), m_nSeqNumber(0) {} // 클래스 생성자

	void Connect(boost::asio::ip::tcp::endpoint& endpoint) // 클라이언트가 서버에 접속하기위해 사용
	{
		m_Socket.async_connect(endpoint, boost::bind(&TCP_Client::handle_connect, this, boost::asio::placeholders::error)); // 비동기로 endpoint(서버)에 접속 실행 후 handle_connect 호출
	}

private:
	void PostWrite()
	{
		if (m_Socket.is_open() == false) // 소켓이 닫힐 경우 리턴
		{
			return;
		}
		if (m_nSeqNumber > 7) // 총 7번 발신
		{
			m_Socket.close();
			return;
		}

		++m_nSeqNumber;
		char szMessage[128] = { 0, };
		sprintf_s(szMessage, 128 - 1, "%d - Send Message", m_nSeqNumber); // 0부터 6까지 발신하기 위해 szMessage에 숫자(m_nSeqNumber) 삽입
		// 전역 async_write를 통해 비동기로 메시지 발신 후 handle_write 함수 실행
		boost::asio::async_write(m_Socket, boost::asio::buffer(m_WriteMessage), boost::bind(&TCP_Client::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		/*
		   비동기 write의 주의할 점은 write가 완료될 때까지 buffer의 든 데이터를 잘 보관해야 하는데
		   데이터의 전송 시점은 write 호출 때가 아닌 끝나고 완료 함수(handle_write)가 호출되야지 데이터가 모두 보내진 것으로 취급된다.

		   그래서 async_write를 멤버함수가 아닌 전역함수로 하였다. 이는 데이터를 다 보내기 전에 완료 함수(handle_write)함수가 호출되는 걸 방지하기 위해서다. 
		   
		   만약 Socket 객체의 멤버함수로 async_write 함수를 호출한다면 인스턴스의 멤버함수는 같은 메모리를 점유하기에
		   여러번 호출되어도 같은 메모리를 써야하는 관계로 먼저 실행 중이던 멤버함수안의 지역변수가 새 인자로 초기화된다.

		   그래서 연속적으로 write 함수를 호출하는 비동기 작업에선 버퍼의 데이터를 전송 끝까지 유지하기 위해
		   비교적 공용자재인 전역 함수를 통해 write하여 전역 함수 고유의 주소를 통해 데이터 전송을 보장하여 buffer의 안정성을 확보할 수 있다.
		*/
		PostReceive(); // 서버로부터 에코 데이터 수신
	}

	void PostReceive()
	{
		memset(&m_ReceiveBuffer, '\0', sizeof(m_ReceiveBuffer)); // 수신용 버퍼 초기화
		
		m_Socket.async_read_some(boost::asio::buffer(m_ReceiveBuffer), boost::bind(&TCP_Client::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		// 비동기로 데이터 수신(read_some) 후 handle_receive 함수 실행
	}

	void handle_connect(const boost::system::error_code& error) // 접속 완료 후 헨들링되는 함수
	{
		if (error) // 접속 실패
		{
			cout << "connect failed : " << error.message() << endl;
		}
		else // 접속 성공
		{
			cout << "conected" << endl;
			PostWrite(); // 데이터 발신 함수 실행
		}
	}

	void handle_write(const boost::system::error_code&, size_t)
	{
		cout << "전송함" << endl;
	}

	void handle_receive(const boost::system::error_code& error, size_t byte_transferred)
	{
		if (error) // 수신 중 에러가 있을 시
		{
			if (error == boost::asio::error::eof)
			{
				cout << "서버와 연결이 끊어졌습니다." << endl; // 연결 종료 시
			}
			else
			{
				cout << "error No: " << error.value() << " error Message : " << error.message() << endl; // 예외 발생 시
			}
		}
		else // 에러가 없이 정상적으로 수신될 시
		{
			const string strRecvmessage = m_ReceiveBuffer.data();
			cout << "서버에서 받은 메시지:" << strRecvmessage << ", 받은 크기: " << byte_transferred << endl; // 수신한 에코 메시지 출력
			PostWrite(); // 다시 ++메시지 발신(7번 반복)
		}
	}
};

int main()
{
	boost::asio::io_service io_service; // I/O 이벤트를 OS에서 디스패치하는 객체
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(SERVER_IP), PORT_NUMBER); // 접속할 서버의 IP(char*)와 포트 설정

	TCP_Client clinet(io_service);
	clinet.Connect(endpoint); // 접속할 서버의 IP를 인자로 async_connect() 함수 실행
	io_service.run(); // run() 함수를 호출하여 main() 종료를 방지
	cout << "네트워크 접속 종료" << endl;
}