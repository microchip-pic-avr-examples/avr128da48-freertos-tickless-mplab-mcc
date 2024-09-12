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

#define LpTIMER_CLOCK     (32768UL)

#define LpTIMER_TICK_PERIOD_MS  (( TickType_t ) 1000 / 1000 )

#define LpTIMER_TICKS_TO_COUNTS(tick_cnt) (uint32_t)(((float)((tick_cnt) * LpTIMER_CLOCK)/1000) - 0.5)

#define LpTIMER_COUNTS_TO_TICKS(counts)   (uint32_t)((float)((counts) * 1000)/LpTIMER_CLOCK  )

static uint32_t lpTimerValue;
volatile uint8_t isWakeUpInterrupt;

void WakeUpISR(void)
{
  isWakeUpInterrupt = 0x55;
}

void LowPowerTimerUpdate(void)
{
  lpTimerValue += 0x00010000;
}

void lpTimer_Initialize(void)
{
	lpTimerValue = 0;
	isWakeUpInterrupt = 0;
    while (RTC.STATUS & RTC_CTRLABUSY_bm);
    RTC_Stop();
 	RTC_SetOVFIsrCallback(LowPowerTimerUpdate);
	RTC_SetCMPIsrCallback(WakeUpISR);
	
    while (RTC.STATUS & RTC_CTRLABUSY_bm);
    RTC.CTRLA |= 0x80;      //run in standby mode enabled

    while (RTC.STATUS & RTC_CTRLABUSY_bm);
	RTC_Start();
}

static uint32_t ulGetExternalTime(void)
{
    uint32_t time_u32;
	
    time_u32 = RTC_ReadCounter() + lpTimerValue;

    return time_u32;
}

static void vSetWakeTimeInterrupt(uint16_t xExpectedIdleTime)
{
    uint32_t rtc_cnt_time;

    /* Compute the required  */
    rtc_cnt_time = LpTIMER_TICKS_TO_COUNTS(xExpectedIdleTime);
    rtc_cnt_time += ulGetExternalTime();

    while (RTC.STATUS & RTC_CMPBUSY_bm);
    RTC.CMP = (rtc_cnt_time & 0xFFFF);

    //Clear, then enable compare interrupt
    RTC.INTFLAGS = RTC_CMP_bm;
	RTC_EnableCMPInterrupt();
 }

/* Define the function that is called by portSUPPRESS_TICKS_AND_SLEEP() */
__attribute__((weak)) void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
     eSleepModeStatus eSleepStatus;
     uint32_t ulLowPowerTimeBeforeSleep, ulLowPowerTimeAfterSleep;
    
    /* Read the current time from a time source that will remain operational while the microcontroller is in a low power state */
    ulLowPowerTimeBeforeSleep = ulGetExternalTime();

    /* Stop the timer that is generating the tick interrupt */
    TCB0_Stop();

    /* Enter a critical section that will not effect interrupts bringing the MCU out of sleep mode */
    portDISABLE_INTERRUPTS();

    /* Ensure it is still ok to enter the sleep mode */
    eSleepStatus = eTaskConfirmSleepModeStatus();

    if (eSleepStatus == eAbortSleep)
    {
        /* A task has been moved out of the Blocked state since this macro was executed, or a context switch is being held pending.  Do not enter a sleep state.  Restart the tick and exit the critical section. */
        TCB0_Start();
        portENABLE_INTERRUPTS();
    }
    else
    {
        if (eSleepStatus == eNoTasksWaitingTimeout)
        {
            /* A user definable macro that allows application code to be inserted here.  
            Such application code can be used to minimize power consumption further by 
            turning off IO, peripheral clocks, the Flash, etc. */
            configPRE_PWR_DOWN_PROCESSING();

            /* There are no running state tasks and no tasks that are blocked with a time out.  
            Assuming the application does not care if the tick time slips with respect to calendar 
            time then enter a deep sleep that can only be woken by the user button being pushed on 
            the Curiosity Nano board.  
            If the application does require the tick time to keep better track of the calender time 
            then the PIT peripheral can be used to make rough adjustments. */

            portSET_MODE_AND_SLEEP(SLEEP_MODE_PWR_DOWN);

            /* A user definable macro that allows application code to be inserted here.  
            Such application code can be used to reverse any actions taken by the 
            configPRE_STOP_PROCESSING(). */

            configPOST_PWR_DOWN_PROCESSING();

            /* Exit the critical section - it might be possible to do this immediately after the 
            SET_MODE_AND_SLEEP calls */

            /* Restart the timer that is generating the tick interrupt*/
            TCB0_Start();       
            portENABLE_INTERRUPTS();
        }
        else
        {
            /* Configure an interrupt to bring the microcontroller out of its low power state at
            the time the kernel next needs to execute.  
            The interrupt must be generated from a source that remains operational when the
            microcontroller is in a low power state. */
            vSetWakeTimeInterrupt(xExpectedIdleTime);

            /* Allow the application to define some pre-sleep processing */
            configPRE_SLEEP_PROCESSING(xExpectedIdleTime);

            /* Enter the low power state */
            portSET_MODE_AND_SLEEP(SLEEP_MODE_STANDBY);

            /* Disable low power timer interrupt */
			RTC_DisableCMPInterrupt();
			RTC.INTFLAGS = RTC_CMP_bm;
			
            /* Determine how long the microcontroller was actually in a low power state for, 
            which will be less than xExpectedIdleTime if the microcontroller was brought out 
            of low power mode by an interrupt other than that configured by the 
            vSetWakeTimeInterrupt() call.
             * Note that the scheduler is suspended before portSUPPRESS_TICKS_AND_SLEEP() is 
             called, and resumed when portSUPPRESS_TICKS_AND_SLEEP() returns.  
             Therefore no other tasks will execute until this function completes. */
			
            ulLowPowerTimeAfterSleep = ulGetExternalTime();

            /* Allow the application to define some post sleep processing. 
            It can be used to reverse the actions of configPRE_SLEEP_PROCESSING(),  and in so doing, return the microcontroller back to its fully operational state */
            configPOST_SLEEP_PROCESSING(xExpectedIdleTime);
            
            /* Correct the kernels tick count to account for the time the microcontroller spent in its low power state */
            if (isWakeUpInterrupt == 0x55) 
			{
			    isWakeUpInterrupt = 0x00;
				vTaskStepTick(xExpectedIdleTime);
			}
			else
			{
			    vTaskStepTick(LpTIMER_COUNTS_TO_TICKS(ulLowPowerTimeAfterSleep - ulLowPowerTimeBeforeSleep));
			}

            /* Exit the critical section - it might be possible to do this immediately after the SET_MODE_AND_SLEEP calls */
            portENABLE_INTERRUPTS();

            /* Restart the timer that is generating the tick interrupt*/
            TCB0_Start();
        }
    }
}
