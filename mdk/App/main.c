/******************************************************************************

                  ��Ȩ���� (C), 2000-2100, �콾����

 ******************************************************************************
  �� �� ��   : main.c
  �� �� ��   : ����
  ��    ��   : RenM
  ��������   : 2018��2��2�� ������
  ����޸�   :
  ��������   : ��������������
  �����б�   :
              main
  �޸���ʷ   :
  1.��    ��   : 2018��2��2�� ������
    ��    ��   : RenM
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
/*****************************************************************************
 �� �� ��  : main
 ��������  : �������������ʼ���Լ�GUI����
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��2��2�� ������
    ��    ��   : RenM
    �޸�����   : �����ɺ���

*****************************************************************************/
int main(void)
{  
	WM_HWIN Template_Framewin;
	
	/* Set NXP config from MCUexpresso Config Tools */
	BOARD_InitPins();
	BOARD_InitBootClocks();

	/* �����ں˳�ʼ�� */	
	osKernelInitialize();
	
	/* ��������ϵͳ  */
	osKernelStart();
	
	/* Set SDRAM Init */
	BOARD_InitSDRAM();
	  
	/* Set the back light PWM. */
	BOARD_InitPWM();
	
	/* Set TFT work on TFT,BRG565 ,Also Turn on Cursor */ 
	LCD_TFT16565_Init();

	/* Set I2C mode */
	APP_I2C_Init();
		
	/* �������� */
	touch_thread_init();   //��������
	fs_task_init();        //�ļ�ϵͳ����
		
	/* GUI��ʼ�� */
	GUI_Init();          
		
	/* GUI��� */
	/*GUI����ջ�Ƚϴ�,MAIN����Ҳ�Ƚϴ�,���Է���һ��ȽϺ���*/
	Template_Framewin = CreateWindow();//��������
	WM_EnableMemdev(Template_Framewin);//ʹ�ܴ洢��
		
	while (1)
	{
		/* GUI��ִ�к��� */
		GUI_Exec();
	}
}


