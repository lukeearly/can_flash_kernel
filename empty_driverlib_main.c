//#############################################################################
//
// FILE:   empty_driverlib_main.c
//
// TITLE:  Empty Project
//
// Empty Project Example
//
// This example is an empty project setup for Driverlib development.
//
//#############################################################################
//
//
// $Copyright:
// Copyright (C) 2013-2024 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

//
// Included Files
//
#include "F021_F2837xD_C28x.h"
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"

#define MY_STATION_ADDRESS 0xaaaa
const uint16_t DEVICE_ID = MY_STATION_ADDRESS;

#define CRO_CMD_CONNECT             0x01
#define CRO_CMD_GET_CCP_VERSION     0x1b
#define CRO_CMD_EXCHANGE_ID         0x17
#define CRO_CMD_SET_MTA             0x02
#define CRO_CMD_DNLOAD              0x03
#define CRO_CMD_DNLOAD_6            0x23
#define CRO_CMD_UPLOAD              0x04
#define CRO_CMD_SHORT_UP            0x0f
#define CRO_CMD_GET_DAQ_SIZE        0x14
#define CRO_CMD_SET_DAQ_PTR         0x15
#define CRO_CMD_WRITE_DAQ           0x16
#define CRO_CMD_START_STOP          0x06
#define CRO_CMD_DISCONNECT          0x07
#define CRO_CMD_TEST                0x05
#define CRO_CMD_CLEAR_MEMORY        0x10
#define CRO_CMD_PROGRAM             0x18
#define CRO_CMD_PROGRAM_6           0x22

#define DTO_PID_CRM                 0xff
#define DTO_PID_EV                  0xfe

#define CRM_CRC_ACK                 0x00
#define CRM_CRC_DAQPROC_OVERLOAD    0x01
#define CRM_CRC_CMDPROC_BUSY        0x10
#define CRM_CRC_DAQPROC_BUSY        0x11
#define CRM_CRC_INT_TIMEOUT         0x12
#define CRM_CRC_KEY_REQ             0x18
#define CRM_CRC_SESS_STAT_REQ       0x19
#define CRM_CRC_COLD_START_REQ      0x20
#define CRM_CRC_CAL_DATA_INIT_REQ   0x21
#define CRM_CRC_DAQ_LIST_INIT_REQ   0x22
#define CRM_CRC_CODE_UPDATE_REQ     0x23
#define CRM_CRC_UNKNOWN_CMD         0x30
#define CRM_CRC_CMD_SYNTAX          0x31
#define CRM_CRC_PARAM_OUT_OF_RANGE  0x32
#define CRM_CRC_ACCESS_DENIED       0x33
#define CRM_CRC_OVERLOAD            0x34
#define CRM_CRC_ACCESS_LOCKED       0x35
#define CRM_CRC_NOT_AVAILABLE       0x36

//
// Globals
//
uint16_t rx_data[8];
uint16_t tx_data[8];

#define ST_RUNNING      0x00;
#define ST_CAL_INIT     0x01;
#define ST_DAQL_INIT    0x02;
#define ST_COLD_START   0x03;
#define ST_FAULT        0x04;

uint8_t state;

uint8_t MTA0_ext;
uint32_t MTA0_addr;
uint8_t MTA1_ext;
uint32_t MTA1_addr;

#define EXT_DIRECT 0x0
#define EXT_USER   0x1

#define WRITABLE_LIMIT 0x03FFFF
#define READABLE_LIMIT 0x3FFFFF

#define USER_FLASH_BASE  0x90000 // spans Sectors 5-9
#define USER_FLASH_LIMIT 0xB7FFF // 160kw = 320kb

// leaves 0x80000 to 0x8FFFF (64kw = 128kb) for bootloader

#define BLOCK_SIZE 8
uint16_t progbuf[BLOCK_SIZE];
uint32_t blk_base = 0;
uint32_t blk_limit = 0;

// has to live in ram while modifying flash
#ifdef __cplusplus
#pragma CODE_SECTION(".TI.ramfunc");
#else
#pragma CODE_SECTION(program_flash, ".TI.ramfunc");
#endif
void program_flash() {

}

#define GIVE_UP 2

// sectors 5-9, all 32kw
const uint32_t sector_bases[] = {0x90000, 0x98000, 0xA0000, 0xA8000, 0xB0000};
const uint32_t sector_sizes[] = { 0x8000,  0x8000,  0x8000,  0x8000,  0x8000};

// has to live in ram while modifying flash
#ifdef __cplusplus
#pragma CODE_SECTION(".TI.ramfunc");
#else
#pragma CODE_SECTION(clear_flash, ".TI.ramfunc");
#endif
uint8_t clear_flash(uint32_t *base, uint32_t size) {
    Fapi_StatusType           oReturnCheck;
    Fapi_FlashStatusType      oFlashStatus;
    Fapi_FlashStatusWordType  oFlashStatusWord;

    uint16_t i;
    for (i = 0; i < sizeof (sector_bases) / sizeof (uint32_t *); i++) {
        if ((uint32_t *) sector_bases[i] == base && sector_sizes[i] == size) {
            goto good;
        }
    }

    return CRM_CRC_ACCESS_DENIED;

good:
    //
    // Erase the sector that is programmed in the above example
    //
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector, (uint32 *)base);
    //
    // Wait until FSM is done with erase sector operation
    //
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady) {}

    if (oReturnCheck != Fapi_Status_Success) {
        //
        // Check Flash API documentation for possible errors
        //
        return CRM_CRC_NOT_AVAILABLE;
    }

    //
    // Read FMSTAT register contents to know the status of FSM after
    // erase command to see if there are any erase operation related errors
    //
    oFlashStatus = Fapi_getFsmStatus();
    if (oFlashStatus != 0) {
        //
        // Check Flash API documentation for FMSTAT and debug accordingly
        // Fapi_getFsmStatus() function gives the FMSTAT register contents.
        // Check to see if any of the EV bit, ESUSP bit, CSTAT bit or
        // VOLTSTAT bit is set (Refer to API documentation for more details).
        //
        return CRM_CRC_NOT_AVAILABLE;
    }

    //
    // Verify that sector is erased
    //
    oReturnCheck = Fapi_doBlankCheck(base, size, &oFlashStatusWord);

    if (oReturnCheck != Fapi_Status_Success) {
        return CRM_CRC_NOT_AVAILABLE;
    }

    return CRM_CRC_ACK;
}

uint16_t check_dnload_abs(uint8_t ext, uint32_t addr) {
    uint32_t a = addr >> 1;
    return a <= WRITABLE_LIMIT;
}

uint16_t check_upload_abs(uint8_t ext, uint32_t addr) {
    uint32_t a = addr >> 1;
    return a <= READABLE_LIMIT;
}

uint8_t dnload_abs(uint8_t ext, uint32_t addr, uint16_t value) {
    uint16_t *word;
    switch (ext | ~1) {
    case EXT_DIRECT:
        if (!check_dnload_abs(ext, addr)) return CRM_CRC_ACCESS_DENIED;
        word = (uint16_t *) (addr >> 1);
        break;
    case EXT_USER:
        return CRM_CRC_ACCESS_DENIED;
    default:
        return CRM_CRC_PARAM_OUT_OF_RANGE;
    }
    if (addr & 1) *word = *word & 0x00ff | value << 8;
    else *word = *word & 0xff00 | value & 0x00ff;
    return CRM_CRC_ACK;
}

uint8_t upload_abs(uint8_t ext, uint32_t addr, uint16_t *dest) {
    uint16_t *word;
    switch (ext) {
    case EXT_DIRECT:
        if (!check_upload_abs(ext, addr)) return CRM_CRC_ACCESS_DENIED;
        word = (uint16_t *) (addr >> 1);
        break;
    case EXT_USER:
        if (blk_base <= (addr >> 1) && (addr >> 1) < blk_limit && ((addr >> 1) - blk_base) < BLOCK_SIZE) {
            word = progbuf + (addr >> 1 - blk_base);
            return CRM_CRC_ACK;
        } else {
            word = (uint16_t *) (addr >> 1);
        }
        break;
    default:
        return CRM_CRC_PARAM_OUT_OF_RANGE;
    }
    if (addr & 1) *dest = *word >> 8;
    else *dest = *word & 0xff;
    return CRM_CRC_ACK;
}

uint8_t program_abs(uint8_t ext, uint32_t addr, uint16_t value) {
    uint16_t *word;
    switch (ext) {
    case EXT_DIRECT:
        return CRM_CRC_ACCESS_DENIED;
    case EXT_USER:
        if (blk_base <= (addr >> 1) && (addr >> 1) < blk_limit && ((addr >> 1) - blk_base) < BLOCK_SIZE) {
            word = progbuf + (addr >> 1 - blk_base);
            break;
        } else {
            return CRM_CRC_ACCESS_DENIED;
        }
    default:
        return CRM_CRC_PARAM_OUT_OF_RANGE;
    }
    if (addr & 1) *word = *word & 0x00ff | value << 8;
    else *word = *word & 0xff00 | value & 0x00ff;
    return CRM_CRC_ACK;
}

uint32_t p32(uint8_t *buf) {
    uint32_t ret = buf[3];
    ret <<= 8;
    ret |= buf[2];
    ret <<= 8;
    ret |= buf[1];
    ret <<= 8;
    ret |= buf[0];
    return ret;
}

void u32(uint8_t *buf, uint32_t val) {
    buf[0] = val & 0xff;
    val >>= 8;
    buf[1] = val & 0xff;
    val >>= 8;
    buf[2] = val & 0xff;
    val >>= 8;
    buf[3] = val;
}

uint16_t p16(uint8_t *buf) {
    uint16_t ret = buf[1];
    ret <<= 8;
    ret |= buf[0];
    return ret;
}

void u16(uint8_t *buf, uint16_t val) {
    buf[0] = val & 0xff;
    val >>= 8;
    buf[1] = val;
}

uint16_t flash_initialized = 0;
Fapi_StatusType init_flash() {
    if (flash_initialized) return Fapi_Status_Success;
//
//    #ifdef CPU1
//        Flash_claimPumpSemaphore(FLASH_CPU1_WRAPPER);
//    #elif defined(CPU2)
//        Flash_claimPumpSemaphore(FLASH_CPU2_WRAPPER);
//    #endif

    Fapi_StatusType oReturnCheck;

    oReturnCheck = Fapi_initializeAPI(F021_CPU0_BASE_ADDRESS,
                                      DEVICE_SYSCLK_FREQ/1000000U);

    if (oReturnCheck != Fapi_Status_Success) {
        return oReturnCheck;
    }

    oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank0);

    if (oReturnCheck != Fapi_Status_Success) {
        return oReturnCheck;
    }

    flash_initialized = 1;
    return Fapi_Status_Success;
}

void close_flash() {
    if (flash_initialized) {
//        Flash_releasePumpSemaphore();
        flash_initialized = 0;
    }
}

//
// Main
//
void main(void)
{

    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Disable pin locks and enable internal pull-ups.
    //
    Device_initGPIO();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    //
    // PinMux and Peripheral Initialization
    //
    Board_init();

    //
    // C2000Ware Library initialization
    //
    C2000Ware_libraries_init();

    //
    // Enable Global Interrupt (INTM) and real time interrupt (DBGM)
    //
    EINT;
    ERTM;

    while(1)
    {
        //
        // Poll RxOk bit in CAN_ES register to check completion of Reception
        //
        if(((HWREGH(DiagCAN_BASE + CAN_O_ES) & CAN_ES_RXOK)) == CAN_ES_RXOK)
        {
            //
            // Get the received message
            //
            if (!CAN_readMessage(DiagCAN_BASE, 31, rx_data)) continue;
            // skip if it is the wrong ID

            // default ack response
            tx_data[0] = DTO_PID_CRM;
            tx_data[1] = CRM_CRC_ACK;
            tx_data[2] = rx_data[1]; // counter

            uint16_t i;
            for (i = 3; i < 8; i++) {
                tx_data[i] = 0xaf;
            }

            uint16_t size;
            uint8_t  ext;
            uint32_t addr;
            uint8_t  *arr;
            uint8_t err;

            switch (rx_data[0]) {
            case CRO_CMD_TEST:
                if (p16(rx_data + 2) != MY_STATION_ADDRESS) goto tmp_disc;
                break;
            case CRO_CMD_CONNECT:
                if (p16(rx_data + 2) != MY_STATION_ADDRESS) goto tmp_disc;
                // TODO connect
                break;
            case CRO_CMD_DISCONNECT:
                if (p16(rx_data + 4) != MY_STATION_ADDRESS) continue;
                if (rx_data[2] == 0) {
                tmp_disc: {}
                    // TODO temporary disconnect
                } else if (rx_data[2] == 1) {
                    // TODO permanent disconnect
                } else {
                    // invalid command syntax
                    tx_data[1] = CRM_CRC_CMD_SYNTAX;
                    state = ST_FAULT;
                }
                break;
            case CRO_CMD_EXCHANGE_ID:
                tx_data[3] = 2; // length of device ID
                tx_data[4] = 0;
                tx_data[5] = 0x40; // availability
                tx_data[6] = 0x00; // protections
                MTA0_ext = EXT_DIRECT;
                MTA0_addr = ((uint32_t) &DEVICE_ID) << 1;
                break;
            case CRO_CMD_SET_MTA:
                ext = rx_data[3];
                addr = p32(rx_data + 4);
                if (rx_data[2] > 1 || !(ext == EXT_DIRECT || ext == EXT_USER)) {
                    tx_data[1] = CRM_CRC_PARAM_OUT_OF_RANGE;
                    state = ST_FAULT;
                } else if (ext == EXT_DIRECT && addr > READABLE_LIMIT
                        || ext == EXT_USER && ( addr < USER_FLASH_BASE || addr > USER_FLASH_LIMIT ) ) {
                    tx_data[1] = CRM_CRC_ACCESS_DENIED;
                    state = ST_FAULT;
                } else if (rx_data[2] == 0) {
                    MTA0_ext = rx_data[3];
                    MTA0_addr = p32(rx_data + 4);
                } else {
                    MTA1_ext = rx_data[3];
                    MTA1_addr = p32(rx_data + 4);
                }
                break;
            case CRO_CMD_DNLOAD:
                size = rx_data[2];
                if (size > 5) {
                    // out of range
                    tx_data[1] = CRM_CRC_PARAM_OUT_OF_RANGE;
                    state = ST_FAULT;
                    break;
                }
                arr = rx_data + 3;
                goto dnload;
            case CRO_CMD_DNLOAD_6:
                size = 6;
                arr = rx_data + 2;
            dnload:
                addr = MTA0_addr;
                ext = MTA0_ext;
                if (ext != EXT_DIRECT || (addr + size) >> 1 > WRITABLE_LIMIT) {
                    tx_data[1] = CRM_CRC_ACCESS_DENIED;
                    state = ST_FAULT;
                    break;
                }
                for (i = 0; i < size; i++) {
                    err = dnload_abs(ext, addr, arr[i]);
                    if (err) {
                        tx_data[1] = err;
                        break;
                    }
                    addr++;
                }
                MTA0_addr = addr;
                tx_data[3] = MTA0_ext;
                u32(tx_data + 4, MTA0_addr);
                break;
            case CRO_CMD_PROGRAM:
                size = rx_data[2];
                if (size > 5) {
                    // out of range
                    tx_data[1] = CRM_CRC_PARAM_OUT_OF_RANGE;
                    state = ST_FAULT;
                    break;
                }
                arr = rx_data + 3;
                goto program;
            case CRO_CMD_PROGRAM_6:
                size = 6;
                arr = rx_data + 2;
            program:
                addr = MTA0_addr;
                ext = MTA0_ext;
                if (ext != EXT_USER || addr >> 1 < USER_FLASH_BASE || (addr + size) >> 1 > USER_FLASH_LIMIT) {
                    tx_data[1] = CRM_CRC_ACCESS_DENIED;
                    state = ST_FAULT;
                    break;
                }
                err = init_flash();
                if (err) {
                    tx_data[1] = CRM_CRC_NOT_AVAILABLE;
                    break;
                }
                for (i = 0; i < size; i++) {
                    err = program_abs(ext, addr, arr[i]);
                    if (err) {
                        tx_data[1] = err;
                        break;
                    }
                    addr++;
                }
                MTA0_addr = addr;
                tx_data[3] = MTA0_ext;
                u32(tx_data + 4, MTA0_addr);
                break;
            case CRO_CMD_UPLOAD:
                size = rx_data[2];
                if (size > 5) {
                    // out of range
                    tx_data[1] = CRM_CRC_PARAM_OUT_OF_RANGE;
                    state = ST_FAULT;
                    break;
                }
                arr = tx_data + 3;
                addr = MTA0_addr;
                ext = MTA0_ext;
                for (i = 0; i < size; i++) {
                    err = upload_abs(ext, addr, arr + i);
                    if (err) {
                        tx_data[1] = err;
                        break;
                    }
                    addr++;
                }
                MTA0_addr = addr;
                break;
            case CRO_CMD_SHORT_UP:
                size = rx_data[2];
                if (size > 5) {
                    // out of range
                    tx_data[1] = CRM_CRC_PARAM_OUT_OF_RANGE;
                    state = ST_FAULT;
                    break;
                }
                arr = tx_data + 3;
                addr = p32(rx_data + 4);
                ext = rx_data[3];
                // TODO: check illegal memory addresses here
                for (i = 0; i < size; i++) {
                    err = upload_abs(ext, addr, arr + i);
                    if (err) {
                        tx_data[1] = err;
                        break;
                    }
                    addr++;
                }
                break;
            case CRO_CMD_CLEAR_MEMORY:
                addr = MTA0_addr;
                ext = MTA0_ext;
                if (ext != EXT_USER || addr >> 1 < USER_FLASH_BASE || (addr + p32(rx_data + 2)) >> 1 > USER_FLASH_LIMIT || addr % 4) {
                    tx_data[1] = CRM_CRC_ACCESS_DENIED;
                    state = ST_FAULT;
                    break;
                }

                err = init_flash();
                if (err) {
                    tx_data[1] = CRM_CRC_NOT_AVAILABLE;
                    break;
                }

                err = clear_flash((uint32_t *) (addr >> 1), p32(rx_data + 2));
                tx_data[1] = err;
                break;
            case CRO_CMD_GET_DAQ_SIZE:
            case CRO_CMD_SET_DAQ_PTR:
            case CRO_CMD_WRITE_DAQ:
            case CRO_CMD_START_STOP:
                tx_data[1] = CRM_CRC_ACCESS_DENIED;
                break;
            default:
                // unknown command
                tx_data[1] = CRM_CRC_UNKNOWN_CMD;
                state = ST_FAULT;
            }

            CAN_sendMessage(DiagCAN_BASE, 32, 8, tx_data);
        }
    }
}

//
// End of File
//
