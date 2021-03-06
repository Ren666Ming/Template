/******************************************************************************

                  版权所有 (C), 2000-2100, 天骄电子

 ******************************************************************************
  文 件 名   : touch_task.c
  版 本 号   : 初稿
  作    者   : RenM
  生成日期   : 2018年1月26日 星期五
  最近修改   :
  功能描述   : 建立一个触摸任务
							 TODO:带GUI的函数需要任务栈都比较大，为了节约空间，使用MESS
  函数列表   :
  修改历史   :
  1.日    期   : 2018年1月26日 星期五
    作    者   : RenM
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "touch_I2C.h"
#include "fsl_ft5406.h"
#include "cmsis_os.h"
#include "GUI.h"
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
 void touch_thread(void const *argument);
/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
static ft5406_handle_t touch_handle;
/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
osThreadId id_touch_thread; 								
osThreadDef (touch_thread, osPriorityNormal, 1, 0); 
/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 #define EXAMPLE_I2C_MASTER ((I2C_Type *)I2C2_BASE)

/*****************************************************************************
 函 数 名  : touch_thread
 功能描述  : 触摸任务具体执行
 输入参数  : void const *argument  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月26日 星期五
    作    者   : RenM
    修改内容   : 新生成函数

*****************************************************************************/
 void touch_thread(void const *argument) {
 	int cursorPosX = 0U;
	int cursorPosY = 0U;
	GUI_PID_STATE a_pid;  
	 
	 while(true)
	 {
		
  		touch_event_t touch_event;
	
		if(kStatus_Success==FT5406_GetSingleTouch(&touch_handle, &touch_event, &cursorPosX, &cursorPosY))
		{
			if ((touch_event == kTouch_Down) || (touch_event == kTouch_Contact))
			{
				a_pid.x = cursorPosY;
				a_pid.y = cursorPosX;
				a_pid.Pressed = 1;
				GUI_PID_StoreState(&a_pid); 
			}
			else
			{
				a_pid.Pressed = 0;
				GUI_PID_StoreState(&a_pid);
			}
		}
	 }
 }

/*****************************************************************************
 函 数 名  : touch_thread_init
 功能描述  : 触摸任务初始化
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月26日 星期五
    作    者   : RenM
    修改内容   : 新生成函数

*****************************************************************************/
void touch_thread_init( void )
{
	/* Touch panal Init */
	FT5406_Init(&touch_handle, EXAMPLE_I2C_MASTER);

	/* 建立一个触摸任务 */
	id_touch_thread = osThreadCreate( osThread(touch_thread) , NULL );
}



