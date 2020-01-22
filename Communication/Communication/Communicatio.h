#pragma once

#include <stdio.h>
#include<winsock.h>
#pragma comment(lib, "Ws2_32.lib")
#include <string>
#include <thread>

using namespace std;

class Tools//�򵥹�����
{
public:
	void Dec_To_Bin(UINT32 Dec, bool *Bin);
	void Dec_Set_Bin(UINT32 *Value, int x, bool Bin);
};

class Socket_Client
{
public:
	Socket_Client(int timeout = 200);
	~Socket_Client();

	bool Connect(const char* Addr, int Port);
	void Close();
	bool SendMsg(char* Sendmsg, int Send_len);
	bool ReceiveMsg(char* RecvMsg, int Recv_len);
private:

	SOCKET m_Scoket;
	int port;
	const char* address;
};

class S7_Net//Modbus_TcpЭ��
{
public:
	S7_Net();
	~S7_Net();
	//����
	bool Connect(const char* Addr, int Port = 102);
	//�ر�
	void Close();
	//��ȡ�Ĵ���
	bool Read(int addr, UINT32* value);
	//д��Ĵ���
	bool Write(int addr, UINT32 value);

private:
	//����TCP��
	bool SendMsg(char* Sendmsg, char* RecvMsg, int Send_len, int Recv_len);

	//Socket
	Socket_Client *m_socket;

	//��һ�����ֱ���
	unsigned char plcHead1[22] =
	{
		0x03,0x00,0x00,0x16,0x11,0xE0,0x00,0x00,
		0x00,0x01,0x00,0xC1,0x02,0x10,0x00,0xC2,
		0x02,0x03,0x01,0xC0,0x01,0x0A
	};
	//�ڶ������ֱ���
	unsigned char plcHead2[25] =
	{
		0x03,0x00,0x00,0x19,0x02,0xF0,0x80,0x32,
		0x01,0x00,0x00,0xCC,0xC1,0x00,0x08,0x00,
		0x00,0xF0,0x00,0x00,0x01,0x00,0x01,0x03,0xC0
	};
};


class Modbus_Tcp//Modbus_TcpЭ��
{
public:
	Modbus_Tcp();
	~Modbus_Tcp();

	//����
	bool Connect(const char* Addr, int Port = 502, int Id = 10);
	//�ر�
	void Close();
	//д��Ĵ���
	bool Write(int addr, UINT32 value);
	//��ȡ�Ĵ���
	bool Read(int addr, UINT32* value);

private:
	//����TCP��
	bool SendMsg(char* Sendmsg, char* RecvMsg, int Send_len, int Recv_len);
	//���ı��
	unsigned short flag;
	//Socket
	SOCKET m_Scoket;
	//����
	int port;
	const char* address;
	int id;
};

class Communication :public Tools//ͨ����
{
private:
	const int Front_Count_Addr_1 = 1044;
	const int Back_Count_Addr_1 = 1048;
	const int Front_Count_Addr_2 = 1144;
	const int Back_Count_Addr_2 = 1148;

	const int PLC_Control_Addr = 1000;

	const int IO_Input_Addr_1 = 1200;
	const int IO_Input_Addr_2 = 1204;
	const int IO_Output_Addr_1 = 1212;
	const int IO_Output_Addr_2 = 1216;
	const int Device_Alarm_Addr_1 = 1220;
	const int Device_Alarm_Addr_2 = 1224;

private:
	int Init_Fail_Addr;//��ʼ��ַ
	int Fail_Addr;//��ǰ��ַ
	int Max_Fail_Addr;//����ַ
	void(*CallBack)(int,bool);//�����쳣

private:
	S7_Net *m_Plc;
	thread *Read_Thread;
	thread *Conn_Thread;
	string Ip;
	int Port = 102;

	bool Error_Temp_1[32] = { false };//1�Ż����󻺴�
	bool Error_Temp_2[32] = { false };//2�Ż����󻺴�

	int Link_Error = 0;//�����쳣����

	int Id = 0;
	bool Thread_Flag = false;

private:
	void Read_Data();
	void Connect();

public:
	const int NG_Start_Addr_1 = 1024;//1�Ż����ϸ�ƿ�ӱ�Ŵ洢��ʼ��ַ
	const int NG_Over_Addr_1 = 1040; //1�Ż����ϸ�ƿ�ӱ�Ŵ洢��������ַ

	const int NG_Start_Addr_2 = 1124;//2�Ż����ϸ�ƿ�ӱ�Ŵ洢��ʼ��ַ
	const int NG_Over_Addr_2 = 1140; //2�Ż����ϸ�ƿ�ӱ�Ŵ洢��������ַ

	bool Link_Flag = false;	//PLC����״̬


	//ö�ٿ�������
	enum Command
	{
		/// �趨1�Ż�Ϊ�ֶ�ģʽ		/// �趨2�Ż�Ϊ�ֶ�ģʽ
		Set_Manual_1,				Set_Manual_2,
		/// �趨1�Ż�Ϊ�Զ�ģʽ		/// �趨2�Ż�Ϊ�Զ�ģʽ
		Set_Auto_1,					Set_Auto_2,
		/// 1�Ż����涨λ�������		/// 2�Ż����涨λ�������
		Front_Loca_Out_1,			Front_Loca_Out_2,
		/// 1�Ż����涨λ��������		/// 2�Ż����涨λ��������
		Front_Loca_In_1,			Front_Loca_In_2,
		/// 1�Ż������赲�������		/// 2�Ż������赲�������
		Front_Block_Out_1,			Front_Block_Out_2,
		/// 1�Ż������赲��������		/// 2�Ż������赲��������
		Front_Block_In_1,			Front_Block_In_2,
		/// 1�Ż����涨λ�������		/// 2�Ż����涨λ�������
		Back_Loca_Out_1,			Back_Loca_Out_2,
		/// 1�Ż����涨λ��������		/// 2�Ż����涨λ��������
		Back_Loca_In_1,				Back_Loca_In_2,
		/// 1�Ż������赲�������		/// 2�Ż������赲�������
		Back_Block_Out_1,			Back_Block_Out_2,
		/// 1�Ż������赲��������		/// 2�Ż������赲��������
		Back_Block_In_1,			Back_Block_In_2,
		/// 1�Ż������������			/// 2�Ż������������
		Front_Cam_Trig_1,			Front_Cam_Trig_2,
		/// 1�Ż������������			/// 2�Ż������������
		Back_Cam_Trig_1,			Back_Cam_Trig_2,
		/// 1�Ż��޳��������			/// 2�Ż��޳��������
		Remove_Out_1,				Remove_Out_2,
		/// 1�Ż��޳���������			/// 2�Ż��޳���������
		Remove_In_1,				Remove_In_2,
		/// 1�Ż������ת			    /// 2�Ż������ת
		Motor_FWD_1,				Motor_FWD_2,
		/// 1�Ż����ֹͣ				/// 2�Ż����ֹͣ
		Motor_Stop_1,				Motor_Stop_2,
	};

public:
	/// <summary>
	/// Communication���캯��
	/// </summary>
	/// <param name="ip">PLCͨ��IP��ַ</param>
	/// <param name="id">�趨����id,Ĭ��10,�������ID�����ظ�����ֹ���ݻ���</param>
	/// <param name="port">PLCͨ�Ŷ˿ڣ�502</param>
	Communication(string ip, int port = 102);
	~Communication();

	/// <summary>
	/// Communication�������к���������󽫿�ʼ�Զ�����PLC���������ݣ����ö��߳��첽������������ȴ��������쳣�ᴥ���ص�����
	/// </summary>
	/// <param name="pf(int Error)">�ص����������豸�����쳣ʱ���������������쳣���룩,���쳣״̬</param>
	/// <param name="start_addr">NG��ʼ��ַ���ο�120�й��ó�����1�Ż�����1�Ż���ַ��2�Ż�����2�Ż���ַ</param>
	/// <param name="over_addr">NG��������ַ���ο�120�й��ó�����1�Ż�����1�Ż���ַ��2�Ż�����2�Ż���ַ</param>
	/// <returns>���ؽ����True:���гɹ���False:����ʧ��</returns>
	bool Activate(void(*pf)(int Error, bool x), int start_addr, int over_addr);

	/// <summary>
	/// ֹͣ����ú������ж���PLC����������
	/// </summary>
	/// <returns>���ؽ����True:ִ�гɹ���False:ִ��ʧ��</returns>
	bool Inactivate();

	/// <summary>
	/// д��NG���
	/// </summary>
	/// <param name="x">NG���</param>
	/// <returns>���ؽ����True:д��ɹ���False:д��ʧ��</returns>
	bool Write_NG(UINT32 x);

	/// <summary>
	/// У��1�Ż��������
	/// </summary>
	/// <param name="Num">���մ���</param>
	/// <param name="Pos">����λ�ã�1�������棬2������</param>
	/// <returns>���ؽ������ֵ</returns>
	int Check_1(UINT32 Num, int Pos);

	/// <summary>
	/// У��2�Ż��������
	/// </summary>
	/// <param name="Num">���մ���</param>
	/// <param name="Pos">����λ�ã�1�������棬2������</param>
	/// <returns>���ؽ������ֵ</returns>
	int Check_2(UINT32 Num, int Pos);

	/// <summary>
	/// 1�Ż�������ͬ��
	/// </summary>
	/// <param name="front">�������ռ�����</param>
	/// <param name="back">�������ռ�����</param>
	/// <returns>���ؽ����True:ͬ���ɹ���False:ͬ��ʧ��</returns>
	bool Count_Sync_1(UINT32 *front, UINT32 *back);

	/// <summary>
	/// 2�Ż�������ͬ��
	/// </summary>
	/// <param name="front">�������ռ�����</param>
	/// <param name="back">�������ռ�����</param>
	/// <returns>���ؽ����True:ͬ���ɹ���False:ͬ��ʧ��</returns>
	bool Count_Sync_2(UINT32 *front, UINT32 *back);

	/// <summary>
	/// ���Ͳ��������Ҫ�ȷ����ֶ�����������ʹ����������,����ִ��ʧ��
	/// </summary>
	/// <param name="x">ִ�����ʹ��ö��Command����</param>
	/// <returns>���ؽ����True:ִ�гɹ���False:ִ��ʧ��</returns>
	bool Send_Control_Command(Command x);

	/// <summary>
	/// 1�Ż�ϵͳ��ʼ����ɣ��Ѿ����������豸������ϵͳ��ʼ����ɺ������ô˺�����֪ͨPLC�������У�
	/// </summary>
	/// <returns>���ؽ����True:ʹ�ܳɹ���False:ʹ��ʧ��</returns>
	bool Enabled_Run_1();
	
	/// <summary>
	/// 2�Ż�ϵͳ��ʼ����ɣ��Ѿ����������豸������ϵͳ��ʼ����ɺ������ô˺�����֪ͨPLC�������У�
	/// </summary>
	/// <returns>���ؽ����True:ʹ�ܳɹ���False:ʹ��ʧ��</returns>
	bool Enabled_Run_2();

	/// <summary>
	/// ϵͳ�����쳣�¼���֪ͨPLCֹͣ����
	/// </summary>
	/// <returns>���ؽ����True:ʧ�ܳɹ���False:ʧ��ʧ��</returns>
	bool Disenabled_Run();

	/// <summary>
	/// 1�Ż�������ͬ������
	/// </summary>
	/// <returns>���ؽ����True:����ɹ���False:���ʧ��</returns>
	bool Sync_Reset_1();

	/// <summary>
	/// 2�Ż�������ͬ������
	/// </summary>
	/// <returns>���ؽ����True:����ɹ���False:���ʧ��</returns>
	bool Sync_Reset_2();
};


struct PLC_Data
{
	bool zhuanyinji_ing_1 = false;			/*תӡ���ڷ�ƿ		*/	bool zhuanyinji_ing_2 = false;
	bool zhuanyinji_over_1 = false;			/*��ƿ����			*/	bool zhuanyinji_over_2 = false;
	bool bianpinqi_error_1 = false;			/*��Ƶ���쳣			*/	bool bianpinqi_error_2 = false;
	bool tichu_shenchu_1 = false;			/*�޳��������		*/  bool tichu_shenchu_2 = false;
	bool tichu_suohui_1 = false;			/*�޳���������		*/  bool tichu_suohui_2 = false;
	bool zhengmiandingwei_shenchu_1 = false;/*���涨λ���������λ*/	bool zhengmiandingwei_shenchu_2 = false;
	bool zhengmiandingwei_suohui_1 = false;	/*���涨λ�������ص�λ*/	bool zhengmiandingwei_suohui_2 = false;
	bool zhengmianzudang_shenchu_1 = false;	/*�����赲���������λ*/	bool zhengmianzudang_shenchu_2 = false;
	bool zhengmianzudang_suohui_1 = false;	/*�����赲�������ص�λ*/	bool zhengmianzudang_suohui_2 = false;
	bool fanmiandingwei_shenchu_1 = false;	/*���涨λ���������λ*/	bool fanmiandingwei_shenchu_2 = false;
	bool fanmiandingwei_suohui_1 = false;	/*���涨λ�������ص�λ*/	bool fanmiandingwei_suohui_2 = false;
	bool fanmianzudang_shenchu_1 = false;	/*�����赲���������λ*/	bool fanmianzudang_shenchu_2 = false;
	bool fanmianzudang_suohui_1 = false;	/*�����赲�������ص�λ*/	bool fanmianzudang_suohui_2 = false;
	bool jiting_kaiguan_1 = false;			/*��ͣ����״̬		*/  bool jiting_kaiguan_2 = false;
	bool ruliaokou_chuanganqi_1 = false;	/*���Ͽڴ�����״̬	*/	bool ruliaokou_chuanganqi_2 = false;
	bool zhengmian_chuanganqi_1 = false;	/*�����⴫����״̬	*/	bool zhengmian_chuanganqi_2 = false;
	bool fanmian_chuanganqi_1 = false;		/*�����⴫����״̬	*/	bool fanmian_chuanganqi_2 = false;
	bool tichu_chuanganqi_1 = false;		/*�޳�λ������״̬	*/	bool tichu_chuanganqi_2 = false;
	bool chuliao_chuanganqi_1 = false;		/*���Ͽڴ�����״̬	*/	bool chuliao_chuanganqi_2 = false;
};
static PLC_Data PLC_State;
