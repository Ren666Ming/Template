/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_sdmmc_host.h"
#include "fsl_sdmmc_event.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief SDMMCHOST detect card insert status by host controller.
 * @param base host base address.
 * @param userData user can register a application card insert callback through userData.
 */
static void SDMMCHOST_DetectCardInsertByHost(SDMMCHOST_TYPE *base, void *userData);

/*!
 * @brief SDMMCHOST detect card remove status by host controller.
 * @param base host base address.
 * @param userData user can register a application card insert callback through userData.
 */
static void SDMMCHOST_DetectCardRemoveByHost(SDMMCHOST_TYPE *base, void *userData);

/*!
 * @brief SDMMCHOST transfer function.
 * @param base host base address.
 * @param content transfer configurations.
 */
static status_t SDMMCHOST_TransferFunction(SDMMCHOST_TYPE *base, SDMMCHOST_TRANSFER *content);

/*!
 * @brief SDMMCHOST transfer complete callback.
 * @param base host base address.
 * @param handle host handle.
 * @param status interrupt status.
 * @param userData user data.
 */
static void SDMMCHOST_TransferCompleteCallback(SDMMCHOST_TYPE *base, void *handle, status_t status, void *userData);

/*!
 * @brief card detect deinit function.
 */
static void SDMMCHOST_CardDetectDeinit(void);

/*!
 * @brief card detect deinit function.
 * @param host base address.
 * @param host detect card configuration.
 */
static status_t SDMMCHOST_CardDetectInit(SDMMCHOST_TYPE *base, const sdmmchost_detect_card_t *cd);
/*******************************************************************************
 * Variables
 ******************************************************************************/

static sdif_handle_t s_sdifHandle;
static uint32_t s_sdifDmaTable[SDIF_DMA_TABLE_WORDS];
static volatile bool s_sdifTransferSuccessFlag = true;
/*! @brief Card detect flag. */
static volatile bool s_sdInsertedFlag = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
static void SDMMCHOST_DetectCardInsertByHost(SDIF_Type *base, void *userData)
{
    s_sdInsertedFlag = true;
    SDMMCEVENT_Notify(kSDMMCEVENT_CardDetect);
    /* application callback */
    if (userData && ((sdmmchost_detect_card_t *)userData)->cardInserted)
    {
        ((sdmmchost_detect_card_t *)userData)->cardInserted(true, ((sdmmchost_detect_card_t *)userData)->userData);
    }
}

static void SDMMCHOST_DetectCardRemoveByHost(SDIF_Type *base, void *userData)
{
    s_sdInsertedFlag = false;
    /* application callback */
    if (userData && ((sdmmchost_detect_card_t *)userData)->cardRemoved)
    {
        ((sdmmchost_detect_card_t *)userData)->cardRemoved(false, ((sdmmchost_detect_card_t *)userData)->userData);
    }
}

/* Transfer complete callback function. */
static void SDMMCHOST_TransferCompleteCallback(SDMMCHOST_TYPE *base, void *handle, status_t status, void *userData)
{
    /* receive the right status, notify the event */
    if (status == kStatus_Success)
    {
        s_sdifTransferSuccessFlag = true;
    }
    else
    {
        s_sdifTransferSuccessFlag = false;
    }

    SDMMCEVENT_Notify(kSDMMCEVENT_TransferComplete);
}

/* User defined transfer function. */
static status_t SDMMCHOST_TransferFunction(SDMMCHOST_TYPE *base, SDMMCHOST_TRANSFER *content)
{
    status_t error = kStatus_Success;

    sdif_dma_config_t dmaConfig;

    memset(s_sdifDmaTable, 0, sizeof(s_sdifDmaTable));
    memset(&dmaConfig, 0, sizeof(dmaConfig));

    /* user DMA mode transfer data */
    if (content->data != NULL)
    {
        dmaConfig.enableFixBurstLen = false;
        dmaConfig.mode = kSDIF_DualDMAMode;
        dmaConfig.dmaDesBufferStartAddr = s_sdifDmaTable;
        dmaConfig.dmaDesBufferLen = SDIF_DMA_TABLE_WORDS;
        dmaConfig.dmaDesSkipLen = 0U;
    }

    do
    {
        error = SDIF_TransferNonBlocking(base, &s_sdifHandle, &dmaConfig, content);
    } while (error == kStatus_SDIF_SyncCmdTimeout);

    if ((error != kStatus_Success) ||
        (false == SDMMCEVENT_Wait(kSDMMCEVENT_TransferComplete, SDMMCHOST_TRANSFER_COMPLETE_TIMEOUT)) ||
        (!s_sdifTransferSuccessFlag))
    {
        error = kStatus_Fail;
    }

    SDMMCEVENT_Delete(kSDMMCEVENT_TransferComplete);

    return error;
}

static status_t SDMMCHOST_CardDetectInit(SDMMCHOST_TYPE *base, const sdmmchost_detect_card_t *cd)
{
    sdmmchost_detect_card_type_t cdType = kSDMMCHOST_DetectCardByHostCD;

    if (cd != NULL)
    {
        cdType = cd->cdType;
    }

    /* for interrupt case, only kSDMMCHOST_DetectCardByHostCD can generate interrupt, so implement it only */
    if (cdType != kSDMMCHOST_DetectCardByHostCD)
    {
        return kStatus_Fail;
    }

    if (!SDMMCEVENT_Create(kSDMMCEVENT_CardDetect))
    {
        return kStatus_Fail;
    }
    /* enable the card detect interrupt */
    SDMMCHOST_CARD_DETECT_INSERT_ENABLE(base);
    /* check if card is inserted */
    if (SDMMCHOST_CARD_DETECT_INSERT_STATUS(base, false))
    {
        s_sdInsertedFlag = true;
        /* application callback */
        if (cd && cd->cardInserted)
        {
            cd->cardInserted(true, cd->userData);
        }
    }

    return kStatus_Success;
}

static void SDMMCHOST_CardDetectDeinit(void)
{
    SDMMCEVENT_Delete(kSDMMCEVENT_CardDetect);
    s_sdInsertedFlag = false;
}

void SDMMCHOST_Delay(uint32_t milliseconds)
{
    SDMMCEVENT_Delay(milliseconds);
}

status_t SDMMCHOST_WaitCardDetectStatus(SDMMCHOST_TYPE *base, const sdmmchost_detect_card_t *cd, bool waitCardStatus)
{
    uint32_t timeout = SDMMCHOST_CARD_DETECT_TIMEOUT;

    if (cd != NULL)
    {
        timeout = cd->cdTimeOut_ms;
    }

    if (waitCardStatus != s_sdInsertedFlag)
    {
        /* Wait card inserted. */
        do
        {
            if (!SDMMCEVENT_Wait(kSDMMCEVENT_CardDetect, timeout))
            {
                return kStatus_Fail;
            }
        } while (waitCardStatus != s_sdInsertedFlag);
    }

    return kStatus_Success;
}

bool SDMMCHOST_IsCardPresent(void)
{
    return s_sdInsertedFlag;
}

void SDMMCHOST_PowerOffCard(SDMMCHOST_TYPE *base, const sdmmchost_pwr_card_t *pwr)
{
    if (pwr != NULL)
    {
        pwr->powerOff();
        SDMMCHOST_Delay(pwr->powerOffDelay_ms);
    }
    else
    {
        /* disable the card power */
        SDIF_EnableCardPower(base, false);
        /* Delay several milliseconds to make card stable. */
        SDMMCHOST_Delay(500U);
    }
}

void SDMMCHOST_PowerOnCard(SDMMCHOST_TYPE *base, const sdmmchost_pwr_card_t *pwr)
{
    /* use user define the power on function  */
    if (pwr != NULL)
    {
        pwr->powerOn();
        SDMMCHOST_Delay(pwr->powerOnDelay_ms);
    }
    else
    {
        /* Enable the card power */
        SDIF_EnableCardPower(base, true);
        /* Delay several milliseconds to make card stable. */
        SDMMCHOST_Delay(500U);
    }
}

status_t SDMMCHOST_Init(SDMMCHOST_CONFIG *host, void *userData)
{
    sdif_transfer_callback_t sdifCallback = {0};
    sdif_host_t *sdifHost = (sdif_host_t *)host;
		
		/* init event timer. */
    SDMMCEVENT_InitTimer();  

    /* Initialize SDIF. */
    sdifHost->config.endianMode = kSDMMCHOST_EndianModeLittle;
    sdifHost->config.responseTimeout = 0xFFU;
    sdifHost->config.cardDetDebounce_Clock = 0xFFFFFFU;
    sdifHost->config.dataTimeout = 0xFFFFFFU;
    SDIF_Init(sdifHost->base, &(sdifHost->config));

    /* Set callback for SDIF driver. */
    sdifCallback.TransferComplete = SDMMCHOST_TransferCompleteCallback;
    sdifCallback.cardInserted = SDMMCHOST_DetectCardInsertByHost;
    sdifCallback.cardRemoved = SDMMCHOST_DetectCardRemoveByHost;

    /* Create handle for SDIF driver */
    SDIF_TransferCreateHandle(sdifHost->base, &s_sdifHandle, &sdifCallback, userData);

    /* Create transfer complete event. */
    if (false == SDMMCEVENT_Create(kSDMMCEVENT_TransferComplete))
    {
        return kStatus_Fail;
    }

    /* Define transfer function. */
    sdifHost->transfer = SDMMCHOST_TransferFunction;

    /* Enable the card power here for mmc card case, because mmc card don't need card detect*/
    SDIF_EnableCardPower(sdifHost->base, true);
    /* card detect init */
    SDMMCHOST_CardDetectInit(sdifHost->base, (sdmmchost_detect_card_t *)userData);

    return kStatus_Success;
}

void SDMMCHOST_Reset(SDMMCHOST_TYPE *base)
{
    /* reserved for future */
}

void SDMMCHOST_Deinit(void *host)
{
    sdif_host_t *sdifHost = (sdif_host_t *)host;
    SDIF_Deinit(sdifHost->base);
    SDMMCHOST_CardDetectDeinit();
}
