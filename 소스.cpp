#include <sdkddkver.h>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include<iostream>
using namespace std;

const char SERVER_IP[] = "127.0.0.1";
const unsigned short PORT_NUMBER = 31400;

class TCP_Client // Ŭ���̾�Ʈ Ŭ����
{
public:
	boost::asio::io_service& m_io_service; // I/O �̺�Ʈ�� OS���� ����ġ�ϴ� ��ü
	boost::asio::ip::tcp::socket m_Socket; // Ŭ���̾�Ʈ�� ����(����)
	
	int m_nSeqNumber;
	array<char, 128> m_ReceiveBuffer;
	string m_WriteMessage;

	TCP_Client(boost::asio::io_service& io_service) : m_io_service(io_service), m_Socket(io_service), m_nSeqNumber(0) {} // Ŭ���� ������

	void Connect(boost::asio::ip::tcp::endpoint& endpoint) // Ŭ���̾�Ʈ�� ������ �����ϱ����� ���
	{
		m_Socket.async_connect(endpoint, boost::bind(&TCP_Client::handle_connect, this, boost::asio::placeholders::error)); // �񵿱�� endpoint(����)�� ���� ���� �� handle_connect ȣ��
	}

private:
	void PostWrite()
	{
		if (m_Socket.is_open() == false) // ������ ���� ��� ����
		{
			return;
		}
		if (m_nSeqNumber > 7) // �� 7�� �߽�
		{
			m_Socket.close();
			return;
		}

		++m_nSeqNumber;
		char szMessage[128] = { 0, };
		sprintf_s(szMessage, 128 - 1, "%d - Send Message", m_nSeqNumber); // 0���� 6���� �߽��ϱ� ���� szMessage�� ����(m_nSeqNumber) ����
		// ���� async_write�� ���� �񵿱�� �޽��� �߽� �� handle_write �Լ� ����
		boost::asio::async_write(m_Socket, boost::asio::buffer(m_WriteMessage), boost::bind(&TCP_Client::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		/*
		   �񵿱� write�� ������ ���� write�� �Ϸ�� ������ buffer�� �� �����͸� �� �����ؾ� �ϴµ�
		   �������� ���� ������ write ȣ�� ���� �ƴ� ������ �Ϸ� �Լ�(handle_write)�� ȣ��Ǿ��� �����Ͱ� ��� ������ ������ ��޵ȴ�.

		   �׷��� async_write�� ����Լ��� �ƴ� �����Լ��� �Ͽ���. �̴� �����͸� �� ������ ���� �Ϸ� �Լ�(handle_write)�Լ��� ȣ��Ǵ� �� �����ϱ� ���ؼ���. 
		   
		   ���� Socket ��ü�� ����Լ��� async_write �Լ��� ȣ���Ѵٸ� �ν��Ͻ��� ����Լ��� ���� �޸𸮸� �����ϱ⿡
		   ������ ȣ��Ǿ ���� �޸𸮸� ����ϴ� ����� ���� ���� ���̴� ����Լ����� ���������� �� ���ڷ� �ʱ�ȭ�ȴ�.

		   �׷��� ���������� write �Լ��� ȣ���ϴ� �񵿱� �۾����� ������ �����͸� ���� ������ �����ϱ� ����
		   ���� ���������� ���� �Լ��� ���� write�Ͽ� ���� �Լ� ������ �ּҸ� ���� ������ ������ �����Ͽ� buffer�� �������� Ȯ���� �� �ִ�.
		*/
		PostReceive(); // �����κ��� ���� ������ ����
	}

	void PostReceive()
	{
		memset(&m_ReceiveBuffer, '\0', sizeof(m_ReceiveBuffer)); // ���ſ� ���� �ʱ�ȭ
		
		m_Socket.async_read_some(boost::asio::buffer(m_ReceiveBuffer), boost::bind(&TCP_Client::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		// �񵿱�� ������ ����(read_some) �� handle_receive �Լ� ����
	}

	void handle_connect(const boost::system::error_code& error) // ���� �Ϸ� �� ��鸵�Ǵ� �Լ�
	{
		if (error) // ���� ����
		{
			cout << "connect failed : " << error.message() << endl;
		}
		else // ���� ����
		{
			cout << "conected" << endl;
			PostWrite(); // ������ �߽� �Լ� ����
		}
	}

	void handle_write(const boost::system::error_code&, size_t)
	{
		cout << "������" << endl;
	}

	void handle_receive(const boost::system::error_code& error, size_t byte_transferred)
	{
		if (error) // ���� �� ������ ���� ��
		{
			if (error == boost::asio::error::eof)
			{
				cout << "������ ������ ���������ϴ�." << endl; // ���� ���� ��
			}
			else
			{
				cout << "error No: " << error.value() << " error Message : " << error.message() << endl; // ���� �߻� ��
			}
		}
		else // ������ ���� ���������� ���ŵ� ��
		{
			const string strRecvmessage = m_ReceiveBuffer.data();
			cout << "�������� ���� �޽���:" << strRecvmessage << ", ���� ũ��: " << byte_transferred << endl; // ������ ���� �޽��� ���
			PostWrite(); // �ٽ� ++�޽��� �߽�(7�� �ݺ�)
		}
	}
};

int main()
{
	boost::asio::io_service io_service; // I/O �̺�Ʈ�� OS���� ����ġ�ϴ� ��ü
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(SERVER_IP), PORT_NUMBER); // ������ ������ IP(char*)�� ��Ʈ ����

	TCP_Client clinet(io_service);
	clinet.Connect(endpoint); // ������ ������ IP�� ���ڷ� async_connect() �Լ� ����
	io_service.run(); // run() �Լ��� ȣ���Ͽ� main() ���Ḧ ����
	cout << "��Ʈ��ũ ���� ����" << endl;
}