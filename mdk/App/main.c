/******************************************************************************

                  版权所有 (C), 2000-2100, 天骄电子

 ******************************************************************************
  文 件 名   : main.c
  版 本 号   : 初稿
  作    者   : RenM
  生成日期   : 2018年2月2日 星期五
  最近修改   :
  功能描述   : 主函数建立任务
  函数列表   :
              main
  修改历史   :
  1.日    期   : 2018年2月2日 星期五
    作    者   : RenM
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "board.h"
#include "pin_mux.h"
#include "LCDconf.h"
#include "touch_I2C.h"
	 
#include "GUI.h"
#include "cmsis_os.h"
	 
#include "touch_task.h"
#include "File_task.h"
#include "GUI_Design.h"

#include <stdio.h>

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
/*****************************************************************************
 函 数 名  : main
 功能描述  : 主函数，报告初始化以及GUI功能
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年2月2日 星期五
    作    者   : RenM
    修改内容   : 新生成函数

*****************************************************************************/
int main(void)
{  
	WM_HWIN Template_Framewin;
	
	/* Set NXP config from MCUexpresso Config Tools */
	BOARD_InitPins();
	BOARD_InitBootClocks();

	/* 操作内核初始化 */	
	osKernelInitialize();
	
	/* 启动操作系统  */
	osKernelStart();
	
	/* Set SDRAM Init */
	BOARD_InitSDRAM();
	  
	/* Set the back light PWM. */
	BOARD_InitPWM();
	
	/* Set TFT work on TFT,BRG565 ,Also Turn on Cursor */ 
	LCD_TFT16565_Init();

	/* Set I2C mode */
	APP_I2C_Init();
		
	/* 建立任务 */
	touch_thread_init();   //触摸任务
	fs_task_init();        //文件系统任务
		
	/* GUI初始化 */
	GUI_Init();          
		
	/* GUI设计 */
	/*GUI任务栈比较大,MAIN任务也比较大,所以放在一起比较合适*/
	Template_Framewin = CreateWindow();//创建窗口
	WM_EnableMemdev(Template_Framewin);//使能存储器
		
	while (1)
	{
		/* GUI的执行函数 */
		GUI_Exec();
	}
}



