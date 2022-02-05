#include "stm32l1xx.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_tim.h"
#include "stm32l1xx_ll_lcd.h"
#include "stm32l1xx_ll_exti.h"
#include "stm32l1xx_ll_usart.h"
#include "stm32l152_glass_lcd.h"
#include "stdio.h"
/* RTOS RTX Kernel */
#include "RTL.h"

void SystemClock_Config(void);

/* Forward declaration of tasks. */
__task void tskSysInit (void);
__task void tskEnqueue (void);
__task void tskDequeue (void);

OS_SEM emptyQsemaphore, filledQsemaphore;
OS_TID enq_task, serv_task;

//Varaible
uint8_t customer_queue[3] = {0};
uint8_t enq_index = 0, deq_index = 0;

void enQ(void);
void deQ(void);

int main(void)
{
    SystemClock_Config();
    os_sys_init(tskSysInit);
    while (1);
}

/*---------------------------------------------------------------------------
  Tasks : Implement tasks
 *---------------------------------------------------------------------------*/
__task void tskSysInit (void)
{
    /* Raise it own priority to highest among other defined tasks */
    os_tsk_prio_self(10);

    /* Create tasks and obtain their task identifications. */
    enq_task = os_tsk_create (tskEnqueue, 2);
    serv_task = os_tsk_create (tskDequeue, 2);
        
    os_sem_init(&emptyQsemaphore,3); /* initially, queue is empty, no customer */
    os_sem_init(&filledQsemaphore,0); /* no cunsumer task to execute */
    
    os_tsk_delete_self();
}

__task void tskEnqueue (void) 
{
    for (;;) {
            /* Generate customer every 100 ticks */
            enQ();
            os_dly_wait(100);
    }
}

__task void tskDequeue (void) 
{
    for (;;) {
            /* Generate customer every 100 ticks */
            deQ();
    }
}

void enQ(void){
    os_sem_wait(emptyQsemaphore,0xFFFF);
    customer_queue[enq_index] = 1;
    enq_index = (enq_index + 1) % 3;
    os_sem_send(filledQsemaphore);
}

void deQ(void){
    os_sem_wait(filledQsemaphore,0xFFFF);
    customer_queue[deq_index] = 0;
    deq_index = (deq_index + 1) % 3;
    os_dly_wait(300); // spend 300 ticks before done serving
    os_sem_send(emptyQsemaphore);
}

/* ==============   BOARD SPECIFIC CONFIGURATION CODE BEGIN    ============== */
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 32000000
  *            HCLK(Hz)                       = 32000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLLMUL                         = 6
  *            PLLDIV                         = 3
  *            Flash Latency(WS)              = 1
  * @retval None
  */

void SystemClock_Config(void)
{
    /* Enable ACC64 access and set FLASH latency */ 
    LL_FLASH_Enable64bitAccess();; 
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    /* Set Voltage scale1 as MCU will run at 32MHz */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    
    /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
    while (LL_PWR_IsActiveFlag_VOSF() != 0)
    {
    };
    
    /* Enable HSI if not already activated*/
    if (LL_RCC_HSI_IsReady() == 0)
    {
        /* HSI configuration and activation */
        LL_RCC_HSI_Enable();
        while(LL_RCC_HSI_IsReady() != 1)
        {
        };
    }
    
        
    /* Main PLL configuration and activation */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);

    LL_RCC_PLL_Enable();
    while(LL_RCC_PLL_IsReady() != 1)
    {
    };
    
    /* Sysclk activation on the main PLL */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
    {
    };
    
    /* Set APB1 & APB2 prescaler*/
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    /* Set systick to 1ms in using frequency set to 32MHz                             */
    /* This frequency can be calculated through LL RCC macro                          */
    /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
    LL_Init1msTick(32000000);
    
    /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
    LL_SetSystemCoreClock(32000000);
}

/* ==============   BOARD SPECIFIC CONFIGURATION CODE END      ============== */

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
        ex: printf("Wrong parameters value: file %s on line %d", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/