/******************************************************************************

                  版权所有 (C), 2000-2100, 天骄电子

 ******************************************************************************
  文 件 名   : File_task.c
  版 本 号   : 初稿
  作    者   : RenM
  生成日期   : 2018年1月26日 星期五
  最近修改   :
  功能描述   : 建立一个使用文件系统的任务
  函数列表   :
  修改历史   :
  1.日    期   : 2018年1月26日 星期五
    作    者   : RenM
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "fsl_iocon.h"
#include "fsl_sd.h"
#include "ff.h"
#include "cmsis_os.h"
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void File_check_thread(void const *argument);
/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
extern sd_card_t   g_sd;  /* sd card descriptor */

const  TCHAR driverNumberBuffer[3U] = {DEV_MMC + '0', ':', '/'};
static FATFS g_fileSystem; /* File system object */

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
osThreadId   id_File_Systemp_check; 								
osThreadDef (File_check_thread, osPriorityNormal, 1, 0); 
/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
/* SDcard Describe */
#define DATA_BLOCK_COUNT (5U)
#define DATA_BLOCK_START (2U)
#define DATA_BUFFER_SIZE (FSL_SDMMC_DEFAULT_BLOCK_SIZE * DATA_BLOCK_COUNT)
const sdmmchost_detect_card_t s_sdCardDetect = {
    .cdType = kSDMMCHOST_DetectCardByHostCD,
    .cdTimeOut_ms = (~0U),
    .cardInserted = NULL,
    .cardRemoved = NULL,
    };

					

/*****************************************************************************
 函 数 名  : Board_InitSdifUnusedDataPin
 功能描述  : SDIF未配置引脚的初始化，如果不初始化，则无法切换总线
 输入参数  : void  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月26日 星期五
    作    者   : RenM
    修改内容   : 新生成函数

*****************************************************************************/
static void Board_InitSdifUnusedDataPin(void)
{
	 IOCON_PinMuxSet(IOCON, 4, 29,
					 (IOCON_FUNC2 | IOCON_PIO_SLEW_MASK | IOCON_DIGITAL_EN | IOCON_MODE_PULLUP)); /* sd data[4] */
	 IOCON_PinMuxSet(IOCON, 4, 30,
					 (IOCON_FUNC2 | IOCON_PIO_SLEW_MASK | IOCON_DIGITAL_EN | IOCON_MODE_PULLUP)); /* sd data[5] */
	 IOCON_PinMuxSet(IOCON, 4, 31,
					 (IOCON_FUNC2 | IOCON_PIO_SLEW_MASK | IOCON_DIGITAL_EN | IOCON_MODE_PULLUP)); /* sd data[6] */
	 IOCON_PinMuxSet(IOCON, 5, 0,
					 (IOCON_FUNC2 | IOCON_PIO_SLEW_MASK | IOCON_DIGITAL_EN | IOCON_MODE_PULLUP)); /* sd data[7] */
 }


/*****************************************************************************
 函 数 名  : SDcardHost_Init
 功能描述  : SD卡主机初始化
 输入参数  : void  
 输出参数  : 无
 返 回 值  : status_t
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月26日 星期五
    作    者   : RenM
    修改内容   : 新生成函数

*****************************************************************************/
status_t SDcardHost_Init ( void ) {
	 
	g_sd.host.base = SD_HOST_BASEADDR;			      //设置SD主机外设地址 不用变更
	g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;	//设置SD主机时钟频率 不用变更
	/* card detect type */
	g_sd.usrParam.cd = &s_sdCardDetect; 		      //设置SD卡检测模式为 主机自动检测SD_CD脚
												 
	if (SD_HostInit(&g_sd) != kStatus_Success)	  // 初始化SD卡主机 如果插着SD卡，这一步会失败
	{
		return kStatus_Fail;
	}	 
	  
	return kStatus_Success;
 }
/*****************************************************************************
 函 数 名  : File_Systemp_init
 功能描述  : 文件系统的初始化
 输入参数  : void  
 输出参数  : 无
 返 回 值  : FRESULT
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月26日 星期五
    作    者   : RenM
    修改内容   : 新生成函数

*****************************************************************************/
FRESULT File_Systemp_init( void ){

  FRESULT status = FR_OK;
	
	status = f_mount(&g_fileSystem, driverNumberBuffer, 0U);
  status = f_chdrive((char const *)&driverNumberBuffer[0U]);
   
	return status;         //改变当前驱动器
}
 
/*****************************************************************************
 函 数 名  : File_Systemp_check
 功能描述  : 文件系统检测任务，用来处理文件系统的各种错误
 输入参数  : void const *argument  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月27日 星期六
    作    者   : RenM
    修改内容   : 新生成函数

*****************************************************************************/
void File_check_thread(void const *argument) {
	static bool        s_cardInserted = false;
	
	//判断是否卡插入
	while(true)
	{
		osDelay(100);
		
		if(s_cardInserted != SD_IsCardPresent(&g_sd))
		{
			s_cardInserted = SD_IsCardPresent(&g_sd);
			
			if(s_cardInserted)
			{
				File_Systemp_init();
			}
			else
			{
				f_mount(NULL, driverNumberBuffer, 0U);  //注销工作区
			}
		}
	}
}


/*****************************************************************************
 函 数 名  : fs_task_init
 功能描述  : 文件检测系统的初始化，创建一个文件检测任务
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月27日 星期六
    作    者   : RenM
    修改内容   : 新生成函数

*****************************************************************************/
void fs_task_init( void )
{
	Board_InitSdifUnusedDataPin();      //初始化未配置引脚
  SDcardHost_Init();                  //SD主机初始化
	
	/* 建立一个SD卡的检测任务 */
	id_File_Systemp_check = osThreadCreate( osThread(File_check_thread) , NULL );
}


