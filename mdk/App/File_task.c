/******************************************************************************

                  ��Ȩ���� (C), 2000-2100, �콾����

 ******************************************************************************
  �� �� ��   : File_task.c
  �� �� ��   : ����
  ��    ��   : RenM
  ��������   : 2018��1��26�� ������
  ����޸�   :
  ��������   : ����һ��ʹ���ļ�ϵͳ������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2018��1��26�� ������
    ��    ��   : RenM
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "fsl_iocon.h"
#include "fsl_sd.h"
#include "ff.h"
#include "cmsis_os.h"
/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void File_check_thread(void const *argument);
/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
extern sd_card_t   g_sd;  /* sd card descriptor */

const  TCHAR driverNumberBuffer[3U] = {DEV_MMC + '0', ':', '/'};
static FATFS g_fileSystem; /* File system object */

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
osThreadId   id_File_Systemp_check; 								
osThreadDef (File_check_thread, osPriorityNormal, 1, 0); 
/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
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
 �� �� ��  : Board_InitSdifUnusedDataPin
 ��������  : SDIFδ�������ŵĳ�ʼ�����������ʼ�������޷��л�����
 �������  : void  
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��26�� ������
    ��    ��   : RenM
    �޸�����   : �����ɺ���

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
 �� �� ��  : SDcardHost_Init
 ��������  : SD��������ʼ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : status_t
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��26�� ������
    ��    ��   : RenM
    �޸�����   : �����ɺ���

*****************************************************************************/
status_t SDcardHost_Init ( void ) {
	 
	g_sd.host.base = SD_HOST_BASEADDR;			      //����SD���������ַ ���ñ��
	g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;	//����SD����ʱ��Ƶ�� ���ñ��
	/* card detect type */
	g_sd.usrParam.cd = &s_sdCardDetect; 		      //����SD�����ģʽΪ �����Զ����SD_CD��
												 
	if (SD_HostInit(&g_sd) != kStatus_Success)	  // ��ʼ��SD������ �������SD������һ����ʧ��
	{
		return kStatus_Fail;
	}	 
	  
	return kStatus_Success;
 }
/*****************************************************************************
 �� �� ��  : File_Systemp_init
 ��������  : �ļ�ϵͳ�ĳ�ʼ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : FRESULT
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��26�� ������
    ��    ��   : RenM
    �޸�����   : �����ɺ���

*****************************************************************************/
FRESULT File_Systemp_init( void ){

  FRESULT status = FR_OK;
	
	status = f_mount(&g_fileSystem, driverNumberBuffer, 0U);
  status = f_chdrive((char const *)&driverNumberBuffer[0U]);
   
	return status;         //�ı䵱ǰ������
}
 
/*****************************************************************************
 �� �� ��  : File_Systemp_check
 ��������  : �ļ�ϵͳ����������������ļ�ϵͳ�ĸ��ִ���
 �������  : void const *argument  
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��27�� ������
    ��    ��   : RenM
    �޸�����   : �����ɺ���

*****************************************************************************/
void File_check_thread(void const *argument) {
	static bool        s_cardInserted = false;
	
	//�ж��Ƿ񿨲���
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
				f_mount(NULL, driverNumberBuffer, 0U);  //ע��������
			}
		}
	}
}


/*****************************************************************************
 �� �� ��  : fs_task_init
 ��������  : �ļ����ϵͳ�ĳ�ʼ��������һ���ļ��������
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��1��27�� ������
    ��    ��   : RenM
    �޸�����   : �����ɺ���

*****************************************************************************/
void fs_task_init( void )
{
	Board_InitSdifUnusedDataPin();      //��ʼ��δ��������
  SDcardHost_Init();                  //SD������ʼ��
	
	/* ����һ��SD���ļ������ */
	id_File_Systemp_check = osThreadCreate( osThread(File_check_thread) , NULL );
}


