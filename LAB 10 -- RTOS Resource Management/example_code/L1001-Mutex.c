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
#include <RTL.h>

/* id1, id2 will contain task identifications at run-time. */
OS_TID PROC_TSK_ID, CONS_TSK_ID;
OS_MUT mut_res_amt;

/* Forward declaration of tasks. */
__task void tskSysinit (void);
__task void tskProducer (void);
__task void tskConsumer (void);

/* Shared resource */
int amount=0;

void add_amount(int amt);
void rem_amount(int amt);
void SystemClock_Config(void);

int main(void)
{
    SystemClock_Config();
    os_sys_init(tskSysinit);
    while (1);
}

__task void tskSysinit (void)
{
    /* Raise it own priority to highest among other defined tasks */
    os_tsk_prio_self(10);
    /* Create producer and consumer task then obtain its identification number */
    PROC_TSK_ID = os_tsk_create(tskProducer,2);
    CONS_TSK_ID = os_tsk_create(tskConsumer,2);
    
    /* Init mutex with 8 tokens (defined in RTX_Conf_CM.c) */
    os_mut_init(&mut_res_amt);
    
    /* Remove self (tskSysInit) from system */
    os_tsk_delete_self();
}

__task void tskProducer (void)
{
    for(;;)
    {
      add_amount(10);
    }
}
__task void tskConsumer (void)
{
    for(;;)
    {
      rem_amount(10);
    }
}
void add_amount (int amt)
{
    /* Protect mutual access by mutex */
    os_mut_wait(&mut_res_amt, 0xFFFF);
    amount += amt;
    os_dly_wait(10);
    /* Return mutex after done processing */
    os_mut_release(&mut_res_amt);
}
void rem_amount (int amt)
{
    /* Protect mutual access by mutex */
    os_mut_wait(&mut_res_amt, 0xFFFF);
    amount -= amt;
    os_dly_wait(10);
    /* Return mutex after done processing */
    os_mut_release(&mut_res_amt);
}

/*---------------------------------------------------------------------------
  Tasks : Implement tasks
 *---------------------------------------------------------------------------*/
/*__task void id (void){
    for (;;) 
    {
        os_mut_wait(&mut,0xffff);
        LCD_GLASS_ScrollSentence("59113076-0", 1, 250);
        os_dly_wait(10);
        os_mut_release(&mut);
        os_dly_wait(50);
    }
}

__task void subid (void) {
    for(;;)
    {
        os_mut_wait(&mut,0xffff);
        LCD_GLASS_Clear();
        LCD_GLASS_DisplayString("CPE-312");
        os_dly_wait(10);
        os_mut_release(&mut);
        os_dly_wait(100);
    }
}*/



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
    
    /* Set APB1 & APB2 prescaler */
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




