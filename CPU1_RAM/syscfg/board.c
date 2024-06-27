/*
 * Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "board.h"

//*****************************************************************************
//
// Board Configurations
// Initializes the rest of the modules. 
// Call this function in your application if you wish to do all module 
// initialization.
// If you wish to not use some of the initializations, instead of the 
// Board_init use the individual Module_inits
//
//*****************************************************************************
void Board_init()
{
	EALLOW;

	PinMux_init();
	CAN_init();
	// FLASH Initialization:
	// The "FLASH_init()" should be called after or during initialization functions like 
	// Device_init() or Device_enableAllPeripherals().

	EDIS;
}

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************
void PinMux_init()
{
	//
	// PinMux for modules assigned to CPU1
	//
	
	//
	// CANB -> DiagCAN Pinmux
	//
	GPIO_setPinConfig(DiagCAN_CANRX_PIN_CONFIG);
	GPIO_setPadConfig(DiagCAN_CANRX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(DiagCAN_CANRX_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(DiagCAN_CANTX_PIN_CONFIG);
	GPIO_setPadConfig(DiagCAN_CANTX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(DiagCAN_CANTX_GPIO, GPIO_QUAL_ASYNC);


}

//*****************************************************************************
//
// CAN Configurations
//
//*****************************************************************************
void CAN_init(){
	DiagCAN_init();
}

void DiagCAN_init(){
	CAN_initModule(DiagCAN_BASE);
	//
	// Refer to the Driver Library User Guide for information on how to set
	// tighter timing control. Additionally, consult the device data sheet
	// for more information about the CAN module clocking.
	//
	CAN_setBitTiming(DiagCAN_BASE, 15, 0, 15, 7, 3);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 31
	//      Message Identifier: 1793
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_RX
	//      Message ID Mask: 0
	//      Message Object Flags: 
	//      Message Data Length: 8 Bytes
	//
	CAN_setupMessageObject(DiagCAN_BASE, 31, DiagCAN_MessageObj31_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_RX, 0, 0,8);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 32
	//      Message Identifier: 1794
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_TX
	//      Message ID Mask: 0
	//      Message Object Flags: 
	//      Message Data Length: 8 Bytes
	//
	CAN_setupMessageObject(DiagCAN_BASE, 32, DiagCAN_MessageObj32_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_TX, 0, 0,8);
	//
	// Start CAN module operations
	//
	CAN_startModule(DiagCAN_BASE);
}

//*****************************************************************************
//
// FLASH Configurations
//
//*****************************************************************************
#ifdef __cplusplus
#pragma CODE_SECTION(".TI.ramfunc");
#else
#pragma CODE_SECTION(FLASH_init, ".TI.ramfunc");
#endif
void FLASH_init(){
    //
    // UserFMC0 Initialization
    // 
    //
    // Set the bank power up delay so that the bank will power up properly.
    //
    Flash_setBankPowerUpDelay(UserFMC0_BASE, 0x14);
    //
    // This function sets the fallback power mode of the flash bank specified by
    // the bank parameter. The power mode is specified by the powerMode
    // parameter
    //
    
    //
    // Set available banks to active power mode
    // 
    Flash_setBankPowerMode(UserFMC0_BASE, FLASH_BANK, FLASH_BANK_PWR_ACTIVE);
    //
    // Sets the fallback power mode of the charge pump.
    //
    // Set pump power mode to active
    //
    Flash_setPumpPowerMode(UserFMC0_BASE, FLASH_PUMP_PWR_ACTIVE);
    //
    // Disable cache and prefetch mechanism before changing wait states
    //
    Flash_disableCache(UserFMC0_BASE);
    Flash_disablePrefetch(UserFMC0_BASE);
    //
    // This function sets the number of wait states for a flash read access. The
    // waitstates parameter is a number between 0 and 15. It is important
    // to look at your device's datasheet for information about what the required
    // minimum flash wait-state is for your selected SYSCLK frequency.
    //
    // By default the wait state amount is configured to the maximum 15. 
    //
    // Set flash wait states
    //
    Flash_setWaitstates(UserFMC0_BASE, UserFMC0_WAITSTATES); 
    //
    // Enable prefetch
    //
    Flash_enablePrefetch(UserFMC0_BASE);
    //
    // Enable cache
    //
    Flash_enableCache(UserFMC0_BASE);
    //
    // Enables flash error correction code (ECC) protection.
    //
    Flash_enableECC(UserFMC0_ECCBASE);
    //
    // Sets the single bit error threshold. Valid ranges are from 0-65535.
    //
    Flash_setErrorThreshold(UserFMC0_ECCBASE, UserFMC0_ERRORTHRESHOLD);
    //
    // Force a pipeline flush to ensure that the write to the last register
    // configured occurs before returning.
    //
    FLASH_DELAY_CONFIG;
}
