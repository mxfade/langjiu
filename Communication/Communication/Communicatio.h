#pragma once

#include <stdio.h>
#include<winsock.h>
#pragma comment(lib, "Ws2_32.lib")
#include <string>
#include <thread>

using namespace std;

class Tools//简单工具类
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

class S7_Net//Modbus_Tcp协议
{
public:
	S7_Net();
	~S7_Net();
	//连接
	bool Connect(const char* Addr, int Port = 102);
	//关闭
	void Close();
	//读取寄存器
	bool Read(int addr, UINT32* value);
	//写入寄存器
	bool Write(int addr, UINT32 value);

private:
	//发送TCP包
	bool SendMsg(char* Sendmsg, char* RecvMsg, int Send_len, int Recv_len);

	//Socket
	Socket_Client *m_socket;

	//第一次握手报文
	unsigned char plcHead1[22] =
	{
		0x03,0x00,0x00,0x16,0x11,0xE0,0x00,0x00,
		0x00,0x01,0x00,0xC1,0x02,0x10,0x00,0xC2,
		0x02,0x03,0x01,0xC0,0x01,0x0A
	};
	//第二次握手报文
	unsigned char plcHead2[25] =
	{
		0x03,0x00,0x00,0x19,0x02,0xF0,0x80,0x32,
		0x01,0x00,0x00,0xCC,0xC1,0x00,0x08,0x00,
		0x00,0xF0,0x00,0x00,0x01,0x00,0x01,0x03,0xC0
	};
};


class Modbus_Tcp//Modbus_Tcp协议
{
public:
	Modbus_Tcp();
	~Modbus_Tcp();

	//连接
	bool Connect(const char* Addr, int Port = 502, int Id = 10);
	//关闭
	void Close();
	//写入寄存器
	bool Write(int addr, UINT32 value);
	//读取寄存器
	bool Read(int addr, UINT32* value);

private:
	//发送TCP包
	bool SendMsg(char* Sendmsg, char* RecvMsg, int Send_len, int Recv_len);
	//报文标记
	unsigned short flag;
	//Socket
	SOCKET m_Scoket;
	//参数
	int port;
	const char* address;
	int id;
};

class Communication :public Tools//通信类
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
	int Init_Fail_Addr;//起始地址
	int Fail_Addr;//当前地址
	int Max_Fail_Addr;//最大地址
	void(*CallBack)(int,bool);//触发异常

private:
	S7_Net *m_Plc;
	thread *Read_Thread;
	thread *Conn_Thread;
	string Ip;
	int Port = 102;

	bool Error_Temp_1[32] = { false };//1号机错误缓存
	bool Error_Temp_2[32] = { false };//2号机错误缓存

	int Link_Error = 0;//连接异常缓存

	int Id = 0;
	bool Thread_Flag = false;

private:
	void Read_Data();
	void Connect();

public:
	const int NG_Start_Addr_1 = 1024;//1号机不合格瓶子编号存储开始地址
	const int NG_Over_Addr_1 = 1040; //1号机不合格瓶子编号存储最大结束地址

	const int NG_Start_Addr_2 = 1124;//2号机不合格瓶子编号存储开始地址
	const int NG_Over_Addr_2 = 1140; //2号机不合格瓶子编号存储最大结束地址

	bool Link_Flag = false;	//PLC连接状态


	//枚举控制命令
	enum Command
	{
		/// 设定1号机为手动模式		/// 设定2号机为手动模式
		Set_Manual_1,				Set_Manual_2,
		/// 设定1号机为自动模式		/// 设定2号机为自动模式
		Set_Auto_1,					Set_Auto_2,
		/// 1号机正面定位气缸伸出		/// 2号机正面定位气缸伸出
		Front_Loca_Out_1,			Front_Loca_Out_2,
		/// 1号机正面定位气缸缩回		/// 2号机正面定位气缸缩回
		Front_Loca_In_1,			Front_Loca_In_2,
		/// 1号机正面阻挡气缸伸出		/// 2号机正面阻挡气缸伸出
		Front_Block_Out_1,			Front_Block_Out_2,
		/// 1号机正面阻挡气缸缩回		/// 2号机正面阻挡气缸缩回
		Front_Block_In_1,			Front_Block_In_2,
		/// 1号机背面定位气缸伸出		/// 2号机背面定位气缸伸出
		Back_Loca_Out_1,			Back_Loca_Out_2,
		/// 1号机背面定位气缸缩回		/// 2号机背面定位气缸缩回
		Back_Loca_In_1,				Back_Loca_In_2,
		/// 1号机背面阻挡气缸伸出		/// 2号机背面阻挡气缸伸出
		Back_Block_Out_1,			Back_Block_Out_2,
		/// 1号机背面阻挡气缸缩回		/// 2号机背面阻挡气缸缩回
		Back_Block_In_1,			Back_Block_In_2,
		/// 1号机正面相机触发			/// 2号机正面相机触发
		Front_Cam_Trig_1,			Front_Cam_Trig_2,
		/// 1号机背面相机触发			/// 2号机背面相机触发
		Back_Cam_Trig_1,			Back_Cam_Trig_2,
		/// 1号机剔除气缸伸出			/// 2号机剔除气缸伸出
		Remove_Out_1,				Remove_Out_2,
		/// 1号机剔除气缸缩回			/// 2号机剔除气缸缩回
		Remove_In_1,				Remove_In_2,
		/// 1号机电机正转			    /// 2号机电机正转
		Motor_FWD_1,				Motor_FWD_2,
		/// 1号机电机停止				/// 2号机电机停止
		Motor_Stop_1,				Motor_Stop_2,
	};

public:
	/// <summary>
	/// Communication构造函数
	/// </summary>
	/// <param name="ip">PLC通信IP地址</param>
	/// <param name="id">设定本机id,默认10,多个主机ID不可重复，防止数据混乱</param>
	/// <param name="port">PLC通信端口，502</param>
	Communication(string ip, int port = 102);
	~Communication();

	/// <summary>
	/// Communication激活运行函数，激活后将开始自动连接PLC并交换数据，采用多线程异步处理，不会产生等待，出现异常会触发回调函数
	/// </summary>
	/// <param name="pf(int Error)">回调函数，当设备发生异常时触发（参数传递异常代码）,及异常状态</param>
	/// <param name="start_addr">NG起始地址，参考120行公用常量，1号机传入1号机地址，2号机传入2号机地址</param>
	/// <param name="over_addr">NG最大结束地址，参考120行公用常量，1号机传入1号机地址，2号机传入2号机地址</param>
	/// <returns>返回结果：True:运行成功，False:运行失败</returns>
	bool Activate(void(*pf)(int Error, bool x), int start_addr, int over_addr);

	/// <summary>
	/// 停止活动，该函数将切断与PLC的所有连接
	/// </summary>
	/// <returns>返回结果：True:执行成功，False:执行失败</returns>
	bool Inactivate();

	/// <summary>
	/// 写入NG编号
	/// </summary>
	/// <param name="x">NG编号</param>
	/// <returns>返回结果：True:写入成功，False:写入失败</returns>
	bool Write_NG(UINT32 x);

	/// <summary>
	/// 校验1号机检测数量
	/// </summary>
	/// <param name="Num">拍照次数</param>
	/// <param name="Pos">拍照位置，1代表正面，2代表背面</param>
	/// <returns>返回结果：差值</returns>
	int Check_1(UINT32 Num, int Pos);

	/// <summary>
	/// 校验2号机检测数量
	/// </summary>
	/// <param name="Num">拍照次数</param>
	/// <param name="Pos">拍照位置，1代表正面，2代表背面</param>
	/// <returns>返回结果：差值</returns>
	int Check_2(UINT32 Num, int Pos);

	/// <summary>
	/// 1号机计数器同步
	/// </summary>
	/// <param name="front">正面拍照计数器</param>
	/// <param name="back">背面拍照计数器</param>
	/// <returns>返回结果：True:同步成功，False:同步失败</returns>
	bool Count_Sync_1(UINT32 *front, UINT32 *back);

	/// <summary>
	/// 2号机计数器同步
	/// </summary>
	/// <param name="front">正面拍照计数器</param>
	/// <param name="back">背面拍照计数器</param>
	/// <returns>返回结果：True:同步成功，False:同步失败</returns>
	bool Count_Sync_2(UINT32 *front, UINT32 *back);

	/// <summary>
	/// 发送操作命令，需要先发送手动操作命令后可使用其他命令,否则执行失败
	/// </summary>
	/// <param name="x">执行命令，使用枚举Command变量</param>
	/// <returns>返回结果：True:执行成功，False:执行失败</returns>
	bool Send_Control_Command(Command x);

	/// <summary>
	/// 1号机系统初始化完成，已就绪，允许设备工作（系统初始化完成后必须调用此函数以通知PLC允许运行）
	/// </summary>
	/// <returns>返回结果：True:使能成功，False:使能失败</returns>
	bool Enabled_Run_1();
	
	/// <summary>
	/// 2号机系统初始化完成，已就绪，允许设备工作（系统初始化完成后必须调用此函数以通知PLC允许运行）
	/// </summary>
	/// <returns>返回结果：True:使能成功，False:使能失败</returns>
	bool Enabled_Run_2();

	/// <summary>
	/// 系统发生异常事件，通知PLC停止运行
	/// </summary>
	/// <returns>返回结果：True:失能成功，False:失能失败</returns>
	bool Disenabled_Run();

	/// <summary>
	/// 1号机计数器同步清零
	/// </summary>
	/// <returns>返回结果：True:清除成功，False:清除失败</returns>
	bool Sync_Reset_1();

	/// <summary>
	/// 2号机计数器同步清零
	/// </summary>
	/// <returns>返回结果：True:清除成功，False:清除失败</returns>
	bool Sync_Reset_2();
};


struct PLC_Data
{
	bool zhuanyinji_ing_1 = false;			/*转印正在放瓶		*/	bool zhuanyinji_ing_2 = false;
	bool zhuanyinji_over_1 = false;			/*放瓶结束			*/	bool zhuanyinji_over_2 = false;
	bool bianpinqi_error_1 = false;			/*变频器异常			*/	bool bianpinqi_error_2 = false;
	bool tichu_shenchu_1 = false;			/*剔除气缸伸出		*/  bool tichu_shenchu_2 = false;
	bool tichu_suohui_1 = false;			/*剔除气缸缩回		*/  bool tichu_suohui_2 = false;
	bool zhengmiandingwei_shenchu_1 = false;/*正面定位气缸伸出到位*/	bool zhengmiandingwei_shenchu_2 = false;
	bool zhengmiandingwei_suohui_1 = false;	/*正面定位气缸缩回到位*/	bool zhengmiandingwei_suohui_2 = false;
	bool zhengmianzudang_shenchu_1 = false;	/*正面阻挡气缸伸出到位*/	bool zhengmianzudang_shenchu_2 = false;
	bool zhengmianzudang_suohui_1 = false;	/*正面阻挡气缸缩回到位*/	bool zhengmianzudang_suohui_2 = false;
	bool fanmiandingwei_shenchu_1 = false;	/*反面定位气缸伸出到位*/	bool fanmiandingwei_shenchu_2 = false;
	bool fanmiandingwei_suohui_1 = false;	/*反面定位气缸缩回到位*/	bool fanmiandingwei_suohui_2 = false;
	bool fanmianzudang_shenchu_1 = false;	/*反面阻挡气缸伸出到位*/	bool fanmianzudang_shenchu_2 = false;
	bool fanmianzudang_suohui_1 = false;	/*反面阻挡气缸缩回到位*/	bool fanmianzudang_suohui_2 = false;
	bool jiting_kaiguan_1 = false;			/*急停开关状态		*/  bool jiting_kaiguan_2 = false;
	bool ruliaokou_chuanganqi_1 = false;	/*入料口传感器状态	*/	bool ruliaokou_chuanganqi_2 = false;
	bool zhengmian_chuanganqi_1 = false;	/*正面检测传感器状态	*/	bool zhengmian_chuanganqi_2 = false;
	bool fanmian_chuanganqi_1 = false;		/*反面检测传感器状态	*/	bool fanmian_chuanganqi_2 = false;
	bool tichu_chuanganqi_1 = false;		/*剔除位传感器状态	*/	bool tichu_chuanganqi_2 = false;
	bool chuliao_chuanganqi_1 = false;		/*出料口传感器状态	*/	bool chuliao_chuanganqi_2 = false;
};
static PLC_Data PLC_State;
