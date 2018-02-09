/******************************************************************************

                  版权所有 (C), 2000-2100, 天骄电子

 ******************************************************************************
  文 件 名   : LCDconf.c
  版 本 号   : 初稿
  作    者   : RenM
  生成日期   : 2018年1月26日 星期五
  最近修改   :
  功能描述   : 该文件主要为LCD的配置方法，演示如何配置一个LCD和EMC
  函数列表   :
  修改历史   :
  1.日    期   : 2018年1月26日 星期五
    作    者   : RenM
    修改内容   : 创建文件

******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "fsl_lcdc.h"
#include "fsl_emc.h"
#include "fsl_sctimer.h"
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
/* The LCD timing. */
#define APP_LCD LCD
#define LCD_PANEL_CLK 9000000
#define LCD_PPL 480
#define LCD_HSW 2
#define LCD_HFP 8
#define LCD_HBP 43
#define LCD_LPP 272
#define LCD_VSW 10
#define LCD_VFP 4
#define LCD_VBP 12
#define LCD_POL_FLAGS kLCDC_InvertVsyncPolarity | kLCDC_InvertHsyncPolarity
#define IMG_HEIGHT 272
#define IMG_WIDTH 480
#define LCD_INPUT_CLK_FREQ CLOCK_GetFreq(kCLOCK_LCD)
#define APP_LCD_IRQHandler LCD_IRQHandler
#define APP_LCD_IRQn LCD_IRQn
#define APP_BIT_PER_PIXEL 2
#define APP_PIXEL_PER_BYTE 4
#define APP_PIXEL_MAX_VALUE 3
#define APP_PIXEL_MIN_VALUE 3

/* The SDRAM timing. */
#define SDRAM_REFRESHPERIOD_NS (64 * 1000000 / 4096) /* 4096 rows/ 64ms */
#define SDRAM_TRP_NS (18u)
#define SDRAM_TRAS_NS (42u)
#define SDRAM_TSREX_NS (67u)
#define SDRAM_TAPR_NS (18u)
#define SDRAM_TWRDELT_NS (6u)
#define SDRAM_TRC_NS (60u)
#define SDRAM_RFC_NS (60u)
#define SDRAM_XSR_NS (67u)
#define SDRAM_RRD_NS (12u)
#define SDRAM_MRD_NCLK (2u)
#define SDRAM_RAS_NCLK (2u)
#define SDRAM_MODEREG_VALUE (0x23u)
#define SDRAM_DEV_MEMORYMAP (0x09u) /* 128Mbits (8M*16, 4banks, 12 rows, 9 columns)*/

#define SDRAM_BASE_ADDR 0xa0000000
#define SDRAM_SIZE_BYTES (8 * 1024 * 1024)
#define SDRAM_EXAMPLE_DATALEN (SDRAM_SIZE_BYTES / 4)
#define SDRAM_TEST_PATTERN (2)


/*****************************************************************************
 函 数 名  : LCD_TFT16565_Init
 功能描述  : 初始化LCD，使用16位 BRG模式，并设定显存
 输入参数  : void  
 输出参数  : 无
 返 回 值  : extern
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月26日 星期五
    作    者   : RenM
    修改内容   : 新生成函数

*****************************************************************************/
void LCD_TFT16565_Init(void)
{
	lcdc_config_t lcdConfig;
	
	LCDC_GetDefaultConfig(&lcdConfig);

 	lcdConfig.panelClock_Hz = LCD_PANEL_CLK;
 	lcdConfig.ppl = LCD_PPL;
 	lcdConfig.hsw = LCD_HSW;
 	lcdConfig.hfp = LCD_HFP;
 	lcdConfig.hbp = LCD_HBP;
 	lcdConfig.lpp = LCD_LPP;
 	lcdConfig.vsw = LCD_VSW;
 	lcdConfig.vfp = LCD_VFP;
 	lcdConfig.vbp = LCD_VBP;
 	lcdConfig.polarityFlags = LCD_POL_FLAGS;
 	lcdConfig.upperPanelAddr = (uint32_t)SDRAM_BASE_ADDR;  //指向了显存位置
		
	lcdConfig.bpp = kLCDC_16BPP565;
	lcdConfig.display = kLCDC_DisplayTFT;
	lcdConfig.swapRedBlue = true;

	LCDC_Init(APP_LCD, &lcdConfig, LCD_INPUT_CLK_FREQ);
		
	LCDC_EnableInterrupts(APP_LCD, kLCDC_BaseAddrUpdateInterrupt);
	NVIC_EnableIRQ(APP_LCD_IRQn);
		
	LCDC_Start(APP_LCD);
	LCDC_PowerUp(APP_LCD);
}


/*****************************************************************************
 函 数 名  : BOARD_InitSDRAM
 功能描述  : 初始化EMC作为一个显存
 输入参数  : void  
 输出参数  : 无
 返 回 值  : extern
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年1月26日 星期五
    作    者   : RenM
    修改内容   : 新生成函数

*****************************************************************************/
void BOARD_InitSDRAM(void)
{
    emc_basic_config_t basicConfig;
    emc_dynamic_timing_config_t dynTiming;
    emc_dynamic_chip_config_t dynChipConfig;

    /* Basic configuration. */
    basicConfig.endian = kEMC_LittleEndian;
    basicConfig.fbClkSrc = kEMC_IntloopbackEmcclk;
    /* EMC Clock = CPU FREQ/2 here can fit CPU freq from 12M ~ 180M.
     * If you change the divide to 0 and EMC clock is larger than 100M
     * please take refer to emc.dox to adjust EMC clock delay.
     */
    basicConfig.emcClkDiv = 3;
    /* Dynamic memory timing configuration. */
    dynTiming.readConfig = kEMC_Cmddelay;
    dynTiming.refreshPeriod_Nanosec = SDRAM_REFRESHPERIOD_NS;
    dynTiming.tRp_Ns = SDRAM_TRP_NS;
    dynTiming.tRas_Ns = SDRAM_TRAS_NS;
    dynTiming.tSrex_Ns = SDRAM_TSREX_NS;
    dynTiming.tApr_Ns = SDRAM_TAPR_NS;
    dynTiming.tWr_Ns = (1000000000 / CLOCK_GetFreq(kCLOCK_EMC) + SDRAM_TWRDELT_NS); /* one clk + 6ns */
    dynTiming.tDal_Ns = dynTiming.tWr_Ns + dynTiming.tRp_Ns;
    dynTiming.tRc_Ns = SDRAM_TRC_NS;
    dynTiming.tRfc_Ns = SDRAM_RFC_NS;
    dynTiming.tXsr_Ns = SDRAM_XSR_NS;
    dynTiming.tRrd_Ns = SDRAM_RRD_NS;
    dynTiming.tMrd_Nclk = SDRAM_MRD_NCLK;
    /* Dynamic memory chip specific configuration: Chip 0 - MTL48LC8M16A2B4-6A */
    dynChipConfig.chipIndex = 0;
    dynChipConfig.dynamicDevice = kEMC_Sdram;
    dynChipConfig.rAS_Nclk = SDRAM_RAS_NCLK;
    dynChipConfig.sdramModeReg = SDRAM_MODEREG_VALUE;
    dynChipConfig.sdramExtModeReg = 0; /* it has no use for normal sdram */
    dynChipConfig.devAddrMap = SDRAM_DEV_MEMORYMAP;
    /* EMC Basic configuration. */
    EMC_Init(EMC, &basicConfig);
    /* EMC Dynamc memory configuration. */
    EMC_DynamicMemInit(EMC, &dynTiming, &dynChipConfig, 1);
}

/*****************************************************************************
 函 数 名  : BOARD_InitPWM
 功能描述  : 初始化PWM以调节显示亮度
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
void BOARD_InitPWM(void)
{
    sctimer_config_t config;
    sctimer_pwm_signal_param_t pwmParam;
    uint32_t event;

    CLOCK_AttachClk(kMCLK_to_SCT_CLK);

    CLOCK_SetClkDiv(kCLOCK_DivSctClk, 2, true);

    SCTIMER_GetDefaultConfig(&config);

    SCTIMER_Init(SCT0, &config);

    pwmParam.output = kSCTIMER_Out_5;
    pwmParam.level = kSCTIMER_HighTrue;
    pwmParam.dutyCyclePercent = 5;

    SCTIMER_SetupPwm(SCT0, &pwmParam, kSCTIMER_CenterAlignedPwm, 1000U, CLOCK_GetFreq(kCLOCK_Sct), &event);
}


/*****************************************************************************
 函 数 名  : APP_LCD_IRQHandler
 功能描述  : LCD基址中断
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
void APP_LCD_IRQHandler(void)
{
    uint32_t intStatus = LCDC_GetEnabledInterruptsPendingStatus(APP_LCD);

    LCDC_ClearInterruptsStatus(APP_LCD, intStatus);

    if (intStatus & kLCDC_BaseAddrUpdateInterrupt)
    {
       
    }
    __DSB();
    __DSB();
}


