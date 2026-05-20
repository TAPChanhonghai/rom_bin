        //#define SUB_SOFT_VERSION               60//10
		//#define SUB_SOFT_VERSION               61//10  放松状态下，只响应速度小于当前速度的按键
		//#define SUB_SOFT_VERSION               62//10  运行状态，心率从无到有，触发左窗口显示心率
		//#define SUB_SOFT_VERSION               63//10  更改了和彩屏通信的故障码 增加了子故障码
		//#define SUB_SOFT_VERSION               64//10  和彩屏通信中断，LED进入锁屏
		//#define SUB_SOFT_VERSION               65//10  从4个模式启动跑步机，步数清零，原来没有清零
		//#define SUB_SOFT_VERSION               66//10  彩屏版本关掉BLE蓝牙模块广播，设备不连接舒华运动APP  
		//#define SUB_SOFT_VERSION                 68//10  和彩屏通信超时报故障码，减速停机过程中，右窗口显示电机运行速度强制为0
		//#define SUB_SOFT_VERSION                 69//10  通过组合键扬升6,9关闭和打开气囊故障码上报使能
		#define SUB_SOFT_VERSION                 70//10  和气囊控制板的通信故障也屏蔽掉(6.9组合键)
		#define SUB_SOFT_VERSION                 71//10    修正在按键板上选择4个跑步模式中的一个后，在彩屏上点开始无法启动的问题


SH_T9100T_LED_SelfTest_03  是增加了参数备份的boot 