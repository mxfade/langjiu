#include <iostream>
#include "Communicatio.h"
#include <windows.h>
#include <string>

using namespace std;
int vv=0;
void aaa(int e,bool a)
{
	vv = e;
	printf("%d%s\n",vv,a?"true":"false");
}

int main()
{
	UINT32 a = 55;
	Modbus_Tcp m_modbus;
	Communication m_plc("169.254.17.200");
	m_plc.Activate(aaa, m_plc.NG_Start_Addr_1, m_plc.NG_Over_Addr_1);
	system("pause");
}
