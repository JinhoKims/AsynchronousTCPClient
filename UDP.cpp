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
	boost::asio::ip::udp::endpoint m_SenderEndpoint; // ������ �ּ� (UDP Ư���� �������� ���� ���¿��� ����� ����ϱ⿡ ������ �ּҰ� �ʿ���)

	int m_nSeqNumber;
	std::array<char, 128> m_ReceiveBuffer;
	std::string m_WriteMessage;

	UDP_Client(boost::asio::io_service& io_service) :m_Socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), CLIENT_PORT_NUMBER)), m_nSeqNumber(0) {} // io_service�� Ŭ���̾�Ʈ ��Ʈ �Ҵ�

	void PostWrite() // ������ ����
	{
		if (m_nSeqNumber >= 7)
			return; // 1���� 7���� ����

		++m_nSeqNumber;

		char szMessage[128] = { 0, };
		sprintf_s(szMessage, 128 - 1, "%d - Send Message", m_nSeqNumber);

		m_WriteMessage = szMessage; // ���ۿ� �޽��� ����

		m_Socket.async_send_to(boost::asio::buffer(m_WriteMessage), boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(UDP_IP), SERVER_PORT_NUMBER), boost::bind(&UDP_Client::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		/*
				UDP�� ����(����ȭ)���� ���� ���¿��� �����͸� �ְ�ޱ� ������, ���� ���� �ּ�(_to)�� �˰� �־�� �Ѵ�.
				async_send_to�� ���ڴ� �߽ſ� ����, ���� �ּ�(endpoint), ���ε� �Լ��� �����ȴ�.
		*/
		PostReceive(); // ���� ������ ����
	}

	void PostReceive()
	{
		memset(&m_ReceiveBuffer, '\0', sizeof(m_ReceiveBuffer)); // ���ſ� ���� �ʱ�ȭ
		m_Socket.async_receive_from(boost::asio::buffer(m_ReceiveBuffer), m_SenderEndpoint, boost::bind(&UDP_Client::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		/*
			UDP�� Ư���� �������� ���� ���¿��� �����͸� �ְ�ޱ� ������, ��� �����͸� ���´��� �� �� ����.
			�׷��� m_SenderEndpoint�� ���� ���� �ּҸ� �̸� �����س��� �Ѵ�.

			async_receive_from�� UDP �󿡼� �����͸� ������ �� ���Ǵ� �Լ���.
			���ڴ� ���� ����, �߽��� IP(�����), ���� �Լ��� �����ȴ�
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
			const std::string strRecvMessage = m_ReceiveBuffer.data(); // ���ſ� ������ �޽��� ���
			cout << "�������� ���� �޽��� : " << strRecvMessage << ", ���� ũ�� : " << bytes_transferred << endl;

			PostWrite(); // ���� ����(�޽���) ����
		}
	}
};



int main()
{
	boost::asio::io_context io_service; // I/O �̺�Ʈ�� OS���� ����ġ�ϴ� ��ü

	UDP_Client client(io_service); // UDP �������� ���� �� ��Ʈ �Ҵ�
	client.PostWrite(); // ���� ���� �� ������ �߽�
	io_service.run();

	cout << "��Ʈ��ũ ���� ����" << endl;

	// UDP �������� Ư���� ������ accept�� Ŭ���̾�Ʈ�� connect�� �ʿ� ����, UDP�� �ۼ��� �Լ��� ����Ͽ� write�� read�� �����Ѵٴ� ���� Ư¡�̴�.

}