#include "Communicatio.h"

void Tools::Dec_To_Bin(UINT32 Dec, bool *Bin)
{
	int j = 0;
	while (Dec)
	{
		Bin[j] = (Dec % 2) == 1 ? true : false;
		Dec /= 2;
		j++;
	}
}

void Tools::Dec_Set_Bin(UINT32 *Value, int x, bool Bin)
{
	!Bin ? *Value = *Value &~(1 << x) : *Value = *Value | (1 << x);
}

Socket_Client::Socket_Client(int timeout)
{
	//设置阻塞和超时
	int timeOut = timeout;
	setsockopt(m_Scoket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeOut, sizeof(timeOut));
	setsockopt(m_Scoket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeOut, sizeof(timeOut));

}

Socket_Client::~Socket_Client()
{
	closesocket(m_Scoket);
	WSACleanup();
}

bool Socket_Client::Connect(const char* Addr, int Port)
{
	address = Addr;
	port = Port;

	int iErrMsg;
	//启动WinSock
	WSAData wsaData;
	iErrMsg = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (iErrMsg != NO_ERROR)
	{
		return false;
	}
	//创建Socket
	m_Scoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_Scoket == INVALID_SOCKET)
	{
		return false;
	}

	//目标服务器数据
	sockaddr_in sockaddrServer;
	sockaddrServer.sin_family = AF_INET;
	sockaddrServer.sin_port = htons(port);
	sockaddrServer.sin_addr.s_addr = inet_addr(address);
	//连接,sock与目标服务器连接
	iErrMsg = connect(m_Scoket, (sockaddr*)&sockaddrServer, sizeof(sockaddrServer));
	if (iErrMsg < 0)
	{
		return false;
	}
	return true;
}

void Socket_Client::Close()
{
	closesocket(m_Scoket);
}

bool Socket_Client::SendMsg(char* Sendmsg, int Send_len)
{
	int iErrMsg = 0;

	//发送消息，指定sock发送消息
	iErrMsg = send(m_Scoket, Sendmsg, Send_len, 0);
	if (iErrMsg < 0)
	{
		return false;
	}
	return true;
}

bool Socket_Client::ReceiveMsg(char* RecvMsg, int Recv_len)
{
	int iErrMsg = 0;
	iErrMsg = recv(m_Scoket, RecvMsg, Recv_len, 0);
	if (iErrMsg < 0)
	{
		return false;
	}
	return true;
}

S7_Net::S7_Net()
{
	m_socket = new Socket_Client();
}

S7_Net::~S7_Net()
{
	m_socket->Close();
	m_socket->~Socket_Client();
}


bool S7_Net::Connect(const char* Addr, int Port)
{
	char Recv_buf[1024] = { 0 };
	if (!m_socket->Connect(Addr, Port)) return false;
	if (!SendMsg((char*)plcHead1, Recv_buf, sizeof(plcHead1), sizeof(Recv_buf)) && !SendMsg((char*)plcHead2, Recv_buf, sizeof(plcHead2), sizeof(Recv_buf)))
	{
		return false;
	}
	return true;
}
void S7_Net::Close()
{
	m_socket->~Socket_Client();
}

bool S7_Net::SendMsg(char* Sendmsg, char* RecvMsg, int Send_len, int Recv_len)
{

	//发送消息
	if (!m_socket->SendMsg(Sendmsg, Send_len))
	{
		return false;
	}
	//同步接收消息
	if (!m_socket->ReceiveMsg(RecvMsg, Recv_len))
	{
		return false;
	}
	return true;
}

bool S7_Net::Read(int addr, UINT32* value)
{
	char Recv_buf[1024] = { 0 };
	int readCount = 1;
	unsigned char _PLCCommand[31];
	//S7协议报文
	_PLCCommand[0] = 0x03;
	_PLCCommand[1] = 0x00;

	_PLCCommand[2] = sizeof(_PLCCommand) / 256;
	_PLCCommand[3] = sizeof(_PLCCommand) % 256;

	_PLCCommand[4] = 0x02;
	_PLCCommand[5] = 0xF0;
	_PLCCommand[6] = 0x80;
	_PLCCommand[7] = 0x32;

	_PLCCommand[8] = 0x01;
	_PLCCommand[9] = 0x00;

	_PLCCommand[10] = 0x00;
	_PLCCommand[11] = 0x00;
	_PLCCommand[12] = 0x01;

	_PLCCommand[13] = (sizeof(_PLCCommand) - 17) / 256;
	_PLCCommand[14] = (sizeof(_PLCCommand) - 17) % 256;

	_PLCCommand[15] = 0x00;
	_PLCCommand[16] = 0x00;

	_PLCCommand[17] = 0x04;
	_PLCCommand[18] = 0x01;

	_PLCCommand[19] = 0x12;

	_PLCCommand[20] = 0x0A;

	_PLCCommand[21] = 0x10;

	_PLCCommand[22] = 0x02;

	_PLCCommand[23] = 0x00;
	_PLCCommand[24] = 0x04;

	_PLCCommand[25] = 0x00;
	_PLCCommand[26] = 0x01;

	_PLCCommand[27] = 0x84;

	_PLCCommand[28] = (addr * 8) / 256 / 256 % 256;
	_PLCCommand[29] = (addr * 8) / 256 % 256;
	_PLCCommand[30] = (addr * 8) % 256;

	if (SendMsg((char*)_PLCCommand, Recv_buf, sizeof(_PLCCommand), sizeof(Recv_buf)))
	{
		*value = (unsigned char)Recv_buf[25] * 256 * 256 * 256 +
			(unsigned char)Recv_buf[26] * 256 * 256 +
			(unsigned char)Recv_buf[27] * 256 +
			(unsigned char)Recv_buf[28];
		return true;
	}
	else
	{
		return false;
	}
}
bool S7_Net::Write(int addr, UINT32 value)
{
	char Recv_buf[1024] = { 0 };
	unsigned char _PLCCommand[39];
	//S7协议报文
	_PLCCommand[0] = 0x03;
	_PLCCommand[1] = 0x00;

	_PLCCommand[2] = sizeof(_PLCCommand) / 256;
	_PLCCommand[3] = sizeof(_PLCCommand) % 256;

	_PLCCommand[4] = 0x02;
	_PLCCommand[5] = 0xF0;
	_PLCCommand[6] = 0x80;
	_PLCCommand[7] = 0x32;

	_PLCCommand[8] = 0x01;

	_PLCCommand[9] = 0x00;
	_PLCCommand[10] = 0x00;
	_PLCCommand[11] = 0x00;
	_PLCCommand[12] = 0x05;

	_PLCCommand[13] = 0x00;
	_PLCCommand[14] = 0x0E;

	_PLCCommand[15] = 0x00;
	_PLCCommand[16] = 0x08;

	_PLCCommand[17] = 0x05;

	_PLCCommand[18] = 0x01;

	_PLCCommand[19] = 0x12;
	_PLCCommand[20] = 0x0A;
	_PLCCommand[21] = 0x10;

	_PLCCommand[22] = 0x02;

	_PLCCommand[23] = 0x00;
	_PLCCommand[24] = 0x04;

	_PLCCommand[25] = 0x00;
	_PLCCommand[26] = 0x01;

	_PLCCommand[27] = 0x84;

	_PLCCommand[28] = (addr * 8) / 256 / 256 % 256;
	_PLCCommand[29] = (addr * 8) / 256 % 256;
	_PLCCommand[30] = (addr * 8) % 256;

	_PLCCommand[31] = 0x00;
	_PLCCommand[32] = 0x04;

	_PLCCommand[33] = sizeof(value) * 8 / 256;
	_PLCCommand[34] = sizeof(value) * 8 % 256;

	_PLCCommand[35] = value / 256 / 256 / 256;
	_PLCCommand[36] = value / 256 / 256 % 256;
	_PLCCommand[37] = value / 256;
	_PLCCommand[38] = value % 256;

	return SendMsg((char*)_PLCCommand, Recv_buf, sizeof(_PLCCommand), sizeof(Recv_buf));
}

Modbus_Tcp::Modbus_Tcp()
{
	flag = 0;
}

Modbus_Tcp::~Modbus_Tcp()
{
	closesocket(m_Scoket);
	WSACleanup();
}

void Modbus_Tcp::Close()
{
	closesocket(m_Scoket);
}

bool Modbus_Tcp::Connect(const char* Addr, int Port, int Id)
{
	address = Addr;
	port = Port;
	id = Id;

	int iErrMsg;
	//启动WinSock
	WSAData wsaData;
	iErrMsg = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (iErrMsg != NO_ERROR)
	{
		return false;
	}
	//创建Socket
	m_Scoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//设置阻塞和超时,同步处理
	int timeOut = 200;
	setsockopt(m_Scoket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeOut, sizeof(timeOut));
	setsockopt(m_Scoket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeOut, sizeof(timeOut));

	if (m_Scoket == INVALID_SOCKET)
	{
		return false;
	}

	//目标服务器数据
	sockaddr_in sockaddrServer;
	sockaddrServer.sin_family = AF_INET;
	sockaddrServer.sin_port = htons(port);
	sockaddrServer.sin_addr.s_addr = inet_addr(address);
	//连接,sock与目标服务器连接
	iErrMsg = connect(m_Scoket, (sockaddr*)&sockaddrServer, sizeof(sockaddrServer));
	if (iErrMsg < 0)
	{
		return false;
	}
	return true;
}

bool Modbus_Tcp::SendMsg(char* Sendmsg, char* RecvMsg, int Send_len, int Recv_len)
{
	int iErrMsg = 0;



	//发送消息
	iErrMsg = send(m_Scoket, Sendmsg, Send_len, 0);
	if (iErrMsg < 0)
	{
		return false;
	}
	//接收返回报文
	iErrMsg = recv(m_Scoket, RecvMsg, Recv_len, 0);
	if (iErrMsg < 0)
	{
		return false;
	}
	return true;
}

bool Modbus_Tcp::Write(int addr, UINT32 value)  //写寄存器
{
	unsigned char Send_buf[17];
	char Recv_buf[1024] = { 0 };
	Send_buf[0] = flag / 256;
	Send_buf[1] = flag % 256;
	Send_buf[2] = 0;
	Send_buf[3] = 0;
	Send_buf[4] = 0;//从ID开始到最后的字节数
	Send_buf[5] = 11;
	Send_buf[6] = id;//从机ID
	Send_buf[7] = 16;//命令代码
	Send_buf[8] = addr / 256;//开始的地址
	Send_buf[9] = addr % 256;
	Send_buf[10] = 0;//地址的长度
	Send_buf[11] = 2;
	Send_buf[12] = 4;//写入长度
	Send_buf[13] = value / 256 / 256 / 256;
	Send_buf[14] = value / 256 / 256 % 256;
	Send_buf[15] = value / 256;
	Send_buf[16] = value % 256;
	if (SendMsg((char*)Send_buf, Recv_buf, sizeof(Send_buf), sizeof(Recv_buf)))
	{
		flag++;
		if (flag >= 65535)
		{
			flag = 0;
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Modbus_Tcp::Read(int addr, UINT32* value)//读寄存器
{
	unsigned char Send_buf[12];
	char Recv_buf[1024] = { 0 };
	Send_buf[0] = flag / 256;
	Send_buf[1] = flag % 256;
	Send_buf[2] = 0;
	Send_buf[3] = 0;
	Send_buf[4] = 0;
	Send_buf[5] = 6;//命令代码
	Send_buf[6] = id;//从机ID
	Send_buf[7] = 3;
	Send_buf[8] = addr / 256;//开始的地址
	Send_buf[9] = addr % 256;
	Send_buf[10] = 0;
	Send_buf[11] = 2;
	if (SendMsg((char*)Send_buf, Recv_buf, sizeof(Send_buf), sizeof(Recv_buf)))
	{
		flag++;
		if (flag >= 65535)
		{
			flag = 0;
		}
		*value = (unsigned char)Recv_buf[9] * 256 * 256 * 256 +
			(unsigned char)Recv_buf[10] * 256 * 256 +
			(unsigned char)Recv_buf[11] * 256 +
			(unsigned char)Recv_buf[12];
		return true;
	}
	else
	{
		return false;
	}
}

Communication::Communication(string ip, int port)
{
	m_Plc = new S7_Net();
	Ip = ip;
	Port = port;
}

Communication::~Communication()
{
	delete m_Plc;
	Read_Thread->join();
}

bool Communication::Activate(void(*pf)(int Error, bool x), int start_addr, int over_addr)
{
	try
	{
		Init_Fail_Addr = start_addr;
		Max_Fail_Addr = over_addr;
		Fail_Addr = Init_Fail_Addr;
		CallBack = pf;
		Thread_Flag = true;
		Read_Thread = new thread(&Communication::Read_Data, this);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool Communication::Inactivate()
{
	try
	{
		Thread_Flag = false;
		Read_Thread->join();//结束线程
		m_Plc->Close();
		Link_Flag = false;
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

int Communication::Check_1(UINT32 Num, int Pos) {
	UINT32	plc_front=0, plc_back = 0;
	m_Plc->Read(Front_Count_Addr_1, &plc_front);
	m_Plc->Read(Back_Count_Addr_1, &plc_back);
	if (Pos == 1) {
		return plc_front - Num;
	}
	if (Pos == 2) {
		return plc_back - Num;
	}
	return-1;
}

int Communication::Check_2(UINT32 Num, int Pos) {
	UINT32	plc_front = 0, plc_back = 0;
	m_Plc->Read(Front_Count_Addr_2, &plc_front);
	m_Plc->Read(Back_Count_Addr_2, &plc_back);
	if (Pos == 1) {
		return plc_front - Num;
	}
	if (Pos == 2) {
		return plc_back - Num;
	}
	return-1;
}

bool Communication::Count_Sync_1(UINT32 *front, UINT32 *back)
{
	UINT32	plc_front = 0, plc_back = 0;
	m_Plc->Read(Front_Count_Addr_1, &plc_front);
	m_Plc->Read(Back_Count_Addr_1, &plc_back);

	*front = plc_front;
	*back = plc_back;
	return true;
}

bool Communication::Count_Sync_2(UINT32 *front, UINT32 *back)
{
	UINT32	plc_front = 0, plc_back = 0;
	m_Plc->Read(Front_Count_Addr_2, &plc_front);
	m_Plc->Read(Back_Count_Addr_2, &plc_back);

	*front = plc_front;
	*back = plc_back;

	return true;
}

bool Communication::Sync_Reset_1()
{
	UINT32	plc_front = 0, plc_back = 0;
	if (m_Plc->Write(Front_Count_Addr_1, 0) && m_Plc->Write(Back_Count_Addr_1, 0))
	{
		m_Plc->Read(Front_Count_Addr_1, &plc_front);
		m_Plc->Read(Back_Count_Addr_1, &plc_back);
		if (plc_front == 0 && plc_back == 0)
		{
			return true;
		}
		return false;
	}
	return false;
}

bool Communication::Sync_Reset_2()
{
	UINT32	plc_front = 0, plc_back = 0;
	if (m_Plc->Write(Front_Count_Addr_2, 0) && m_Plc->Write(Back_Count_Addr_2, 0))
	{
		m_Plc->Read(Front_Count_Addr_2, &plc_front);
		m_Plc->Read(Back_Count_Addr_2, &plc_back);
		if (plc_front == 0 && plc_back == 0)
		{
			return true;
		}
		return false;
	}
	return false;
}

bool Communication::Send_Control_Command(Command x)
{
	UINT32 value = 0;
	m_Plc->Read(PLC_Control_Addr, &value);
	switch (x)
	{
	case Communication::Set_Manual_1:Dec_Set_Bin(&value, 24, true);
		break;
	case Communication::Set_Manual_2:Dec_Set_Bin(&value, 8, true);
		break;
	case Communication::Set_Auto_1:Dec_Set_Bin(&value, 24, false);
		break;
	case Communication::Set_Auto_2:Dec_Set_Bin(&value, 8, false);
		break;
	case Communication::Front_Loca_Out_1:Dec_Set_Bin(&value, 28, true);
		break;
	case Communication::Front_Loca_Out_2:Dec_Set_Bin(&value, 12, true);
		break;
	case Communication::Front_Loca_In_1:Dec_Set_Bin(&value, 28, false);
		break;
	case Communication::Front_Loca_In_2:Dec_Set_Bin(&value, 12, false);
		break;
	case Communication::Front_Block_Out_1:Dec_Set_Bin(&value, 27, true);
		break;
	case Communication::Front_Block_Out_2:Dec_Set_Bin(&value, 11, true);
		break;
	case Communication::Front_Block_In_1:Dec_Set_Bin(&value, 27, false);
		break;
	case Communication::Front_Block_In_2:Dec_Set_Bin(&value, 11, false);
		break;
	case Communication::Back_Loca_Out_1:Dec_Set_Bin(&value, 31, true);
		break;
	case Communication::Back_Loca_Out_2:Dec_Set_Bin(&value, 12, true);
		break;
	case Communication::Back_Loca_In_1:Dec_Set_Bin(&value, 31, false);
		break;
	case Communication::Back_Loca_In_2:Dec_Set_Bin(&value, 12, false);
		break;
	case Communication::Back_Block_Out_1:Dec_Set_Bin(&value, 30, true);
		break;
	case Communication::Back_Block_Out_2:Dec_Set_Bin(&value, 11, true);
		break;
	case Communication::Back_Block_In_1:Dec_Set_Bin(&value, 30, false);
		break;
	case Communication::Back_Block_In_2:Dec_Set_Bin(&value, 11, false);
		break;
	case Communication::Front_Cam_Trig_1:Dec_Set_Bin(&value, 29, true);
		break;
	case Communication::Front_Cam_Trig_2:Dec_Set_Bin(&value, 13, false);
		break;
	case Communication::Back_Cam_Trig_1:Dec_Set_Bin(&value, 16, false);
		break;
	case Communication::Back_Cam_Trig_2:Dec_Set_Bin(&value, 0, false);
		break;
	case Communication::Remove_Out_1:Dec_Set_Bin(&value, 18, true);
		break;
	case Communication::Remove_Out_2:Dec_Set_Bin(&value, 2, true);
		break;
	case Communication::Remove_In_1:Dec_Set_Bin(&value, 18, false);
		break;
	case Communication::Remove_In_2:Dec_Set_Bin(&value, 2, false);
		break;
	case Communication::Motor_FWD_1:Dec_Set_Bin(&value, 26, true);
		break;
	case Communication::Motor_FWD_2:Dec_Set_Bin(&value, 10, true);
		break;
	case Communication::Motor_Stop_1:Dec_Set_Bin(&value, 26, false);
		break;
	case Communication::Motor_Stop_2:Dec_Set_Bin(&value, 10, false);
		break;
	default:
		return false;
	}
	m_Plc->Write(PLC_Control_Addr, value);
	//延时50ms脉冲地址清除
	Sleep(50);
	Dec_Set_Bin(&value, 29, false);
	Dec_Set_Bin(&value, 16, false);
	Dec_Set_Bin(&value, 13, false);
	Dec_Set_Bin(&value, 2, false);

	m_Plc->Write(PLC_Control_Addr, value);
	return true;
}

bool Communication::Enabled_Run_1()
{
	UINT32 value=0;
	m_Plc->Read(1000, &value);
	Dec_Set_Bin(&value, 19, true);
	Sleep(10);
	Id = 1;
	if (m_Plc->Write(PLC_Control_Addr, value))return true;
	return false;
}

bool Communication::Enabled_Run_2()
{
	UINT32 value = 0;
	m_Plc->Read(1000, &value);
	Dec_Set_Bin(&value, 3, true);
	Sleep(10);
	Id = 2;
	return m_Plc->Write(PLC_Control_Addr, value);
}

bool Communication::Disenabled_Run()
{
	UINT32 value = 0;
	m_Plc->Read(PLC_Control_Addr, &value);
	Dec_Set_Bin(&value, 4, false);
	Sleep(10);
	return m_Plc->Write(PLC_Control_Addr, value);
}

bool Communication::Write_NG(UINT32 x)
{
	if (m_Plc->Write(Fail_Addr, x))
	{
		Fail_Addr += 4;
		if (Fail_Addr > Max_Fail_Addr)
		{
			Fail_Addr = Init_Fail_Addr;
		}
		return true;
	}
	return false;
}

void Communication::Connect()
{
	if (m_Plc->Connect(Ip.c_str(), Port))
	{
		Link_Flag = true;
	}
}

void Communication::Read_Data()
{
m_connect:while (Thread_Flag && !Link_Flag)
{
	Link_Error = -1;
	Conn_Thread = new thread(&Communication::Connect, this);
	Link_Flag = false;
	Sleep(1000);
	if (Link_Error != -1 && !Link_Flag)
	{
		CallBack(-1, true);//连接失败触发回调，返回连接异常代码-1；
		Link_Error = -1;
	}
}
		  CallBack(-1, false);//连接失败触发回调，返回连接异常代码-1；
		  Link_Error = 0;
		  Link_Flag = true;
		  while (Thread_Flag)
		  {
			  Sleep(200);//0.2秒刷新一次数据
			  UINT32 Input_1 = 0;
			  UINT32 Input_2 = 0;
			  bool Temp_1[32] = { false };
			  bool Temp_2[32] = { false };

			  UINT32 Output_1 = 0;
			  UINT32 Output_2 = 0;
			  bool Temp_11[32] = { false };
			  bool Temp_22[32] = { false };
			  Enabled_Run_1();
			  if (m_Plc->Read(IO_Input_Addr_1, &Input_1) && m_Plc->Read(IO_Input_Addr_2, &Input_2))
			  {
				  Dec_To_Bin(Input_1, Temp_1);							 Dec_To_Bin(Input_2, Temp_2);
				  PLC_State.zhuanyinji_ing_1 = Temp_1[0];				 PLC_State.zhuanyinji_ing_2 = Temp_2[10];
				  PLC_State.zhuanyinji_over_1 = Temp_1[1];				 PLC_State.zhuanyinji_over_2 = Temp_2[11];
				  PLC_State.bianpinqi_error_1 = Temp_1[2];				 PLC_State.bianpinqi_error_2 = Temp_2[12];
				  PLC_State.tichu_shenchu_1 = Temp_1[8];				 PLC_State.tichu_shenchu_2 = Temp_2[8];
				  PLC_State.tichu_suohui_1 = Temp_1[9];					 PLC_State.tichu_suohui_2 = Temp_2[9];
				  PLC_State.zhengmiandingwei_shenchu_1 = Temp_1[16];	 PLC_State.zhengmiandingwei_shenchu_2 = Temp_2[16];
				  PLC_State.zhengmiandingwei_suohui_1 = Temp_1[17];		 PLC_State.zhengmiandingwei_suohui_2 = Temp_2[17];
				  PLC_State.zhengmianzudang_shenchu_1 = Temp_1[18];		 PLC_State.zhengmianzudang_shenchu_2 = Temp_2[18];
				  PLC_State.zhengmianzudang_suohui_1 = Temp_1[19];		 PLC_State.zhengmianzudang_suohui_2 = Temp_2[19];
				  PLC_State.fanmiandingwei_shenchu_1 = Temp_1[20];		 PLC_State.fanmiandingwei_shenchu_2 = Temp_2[20];
				  PLC_State.fanmiandingwei_suohui_1 = Temp_1[21];		 PLC_State.fanmiandingwei_suohui_2 = Temp_2[21];
				  PLC_State.fanmianzudang_shenchu_1 = Temp_1[22];		 PLC_State.fanmianzudang_shenchu_2 = Temp_2[22];
				  PLC_State.fanmianzudang_suohui_1 = Temp_1[23];		 PLC_State.fanmianzudang_suohui_2 = Temp_2[23];
				  PLC_State.jiting_kaiguan_1 = Temp_1[25];				 PLC_State.jiting_kaiguan_2 = Temp_2[25];
				  PLC_State.ruliaokou_chuanganqi_1 = Temp_1[27];		 PLC_State.ruliaokou_chuanganqi_2 = Temp_2[27];
				  PLC_State.zhengmian_chuanganqi_1 = Temp_1[28];		 PLC_State.zhengmian_chuanganqi_2 = Temp_2[28];
				  PLC_State.fanmian_chuanganqi_1 = Temp_1[29];			 PLC_State.fanmian_chuanganqi_2 = Temp_2[29];
				  PLC_State.tichu_chuanganqi_1 = Temp_1[30];			 PLC_State.tichu_chuanganqi_2 = Temp_2[30];
				  PLC_State.chuliao_chuanganqi_1 = Temp_1[31];			 PLC_State.chuliao_chuanganqi_2 = Temp_2[31];
			  }
			  else
			  {
				  goto m_connect;//读取异常将自动重连；
			  }
			 
			  if (m_Plc->Read(Device_Alarm_Addr_1, &Output_1) && m_Plc->Read(Device_Alarm_Addr_2, &Output_2))
			  {
				  if (Id == 1)
				  {
					  Dec_To_Bin(Output_1, Temp_11);
					  if (Temp_11[31] != Error_Temp_1[31])CallBack(431, Temp_11[31]);	if (Temp_11[30] != Error_Temp_1[30])CallBack(430, Temp_11[30]);
					  if (Temp_11[29] != Error_Temp_1[29])CallBack(429, Temp_11[29]);	if (Temp_11[28] != Error_Temp_1[28])CallBack(428, Temp_11[28]);
					  if (Temp_11[27] != Error_Temp_1[27])CallBack(427, Temp_11[27]);	if (Temp_11[26] != Error_Temp_1[26])CallBack(426, Temp_11[26]);
					  if (Temp_11[25] != Error_Temp_1[25])CallBack(425, Temp_11[25]);	if (Temp_11[24] != Error_Temp_1[24])CallBack(424, Temp_11[24]);
					  if (Temp_11[23] != Error_Temp_1[23])CallBack(423, Temp_11[23]);	if (Temp_11[22] != Error_Temp_1[22])CallBack(422, Temp_11[22]);
					  if (Temp_11[21] != Error_Temp_1[21])CallBack(421, Temp_11[21]);	if (Temp_11[20] != Error_Temp_1[20])CallBack(420, Temp_11[20]);
					  if (Temp_11[19] != Error_Temp_1[19])CallBack(419, Temp_11[19]);	if (Temp_11[18] != Error_Temp_1[18])CallBack(418, Temp_11[18]);
					  if (Temp_11[17] != Error_Temp_1[17])CallBack(417, Temp_11[17]);	if (Temp_11[16] != Error_Temp_1[16])CallBack(416, Temp_11[16]);
					  if (Temp_11[8] != Error_Temp_1[8])CallBack(408, Temp_11[8]);
					  Dec_To_Bin(Output_1, Error_Temp_1);
				  }

				  if (Id = 2)
				  {
					  Dec_To_Bin(Output_2, Temp_22);
					  if (Temp_22[31] != Error_Temp_2[31])CallBack(831, Temp_22[31]);	if (Temp_22[30] != Error_Temp_2[30])CallBack(830, Temp_22[30]);
					  if (Temp_22[29] != Error_Temp_2[29])CallBack(829, Temp_22[29]);	if (Temp_22[28] != Error_Temp_2[28])CallBack(828, Temp_22[28]);
					  if (Temp_22[27] != Error_Temp_2[27])CallBack(827, Temp_22[27]);	if (Temp_22[26] != Error_Temp_2[26])CallBack(826, Temp_22[26]);
					  if (Temp_22[25] != Error_Temp_2[25])CallBack(825, Temp_22[25]);	if (Temp_22[24] != Error_Temp_2[24])CallBack(824, Temp_22[24]);
					  if (Temp_22[23] != Error_Temp_2[23])CallBack(823, Temp_22[23]);	if (Temp_22[22] != Error_Temp_2[22])CallBack(822, Temp_22[22]);
					  if (Temp_22[21] != Error_Temp_2[21])CallBack(821, Temp_22[21]);	if (Temp_22[20] != Error_Temp_2[20])CallBack(820, Temp_22[20]);
					  if (Temp_22[19] != Error_Temp_2[19])CallBack(819, Temp_22[19]);	if (Temp_22[18] != Error_Temp_2[18])CallBack(818, Temp_22[18]);
					  if (Temp_22[17] != Error_Temp_2[17])CallBack(817, Temp_22[17]);	if (Temp_22[16] != Error_Temp_2[16])CallBack(816, Temp_22[16]);
					  if (Temp_22[8] != Error_Temp_2[8])CallBack(808, Temp_22[8]);
					  Dec_To_Bin(Output_2, Error_Temp_2);
				  }
			  }
			  else
			  {
				  goto m_connect;//读取异常将自动重连；
			  }
		  }
}