 /*
© [2024] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/ 
/* Scheduler include files */
#include "FreeRTOS.h"
#include "task.h"
#include "lpTimer.h"


/* Define the function that is called by configPRE_PWR_DOWN_PROCESSING() */
__attribute__((weak)) void vPortConfigureForEnterPowerDown(void)
{
   /* configPRE_PWR_DOWN_PROCESSING() is executed immediately before go to power-down 
   sleep instruction. It can be used to turn peripheral clocks off, and activate any 
   microcontroller specific low power functionality. */


}

/* Define the function that is called by configPOST_PWR_DOWN_PROCESSING() */
__attribute__((weak)) void vPortConfigureForExitPowerDown(void)
{
    /* Allow the application to define some post standby sleep processing. 
    It can be used to reverse the actions of configPRE_PWR_DOWN_PROCESSING(),  
    and in so doing, return the microcontroller back to its fully operational state */

}

/* Define the function that is called by configPRE_SLEEP_PROCESSING(x) */
__attribute__((weak)) void vPortConfigureForEnterStandbyMode(TickType_t xExpectedIdleTime)
{
   /* configPRE_SLEEP_PROCESSING() is executed immediately before go to standby sleep instruction. 
   It can be used to turn peripheral clocks off, and activate any microcontroller specific 
   low power functionality. */


}

/* Define the function that is called by configPOST_SLEEP_PROCESSING(x) */
__attribute__((weak)) void vPortConfigureForExitStandbyMode(TickType_t xExpectedIdleTime)
{
    /* Allow the application to define some post standby sleep processing. 
    It can be used to reverse the actions of configPRE_SLEEP_PROCESSING(),  
    and in so doing, return the microcontroller back to its fully operational state */
    

}



