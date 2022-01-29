/* Includes ------------------------------------------------------------------*/
/* Base registers address header file */
#include "stm32l1xx.h"
/* Library related header files */
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_utils.h"

/* RTOS RTX Kernel */
#include <RTL.h>

/* id1, id2 will contain task identifications at run-time. */
OS_TID id1, id2, id3;

/* Forward declaration of tasks. */
__task void task1 (void);
__task void task2 (void);
__task void task3 (void);

void SystemClock_Config(void);

int main(void)
{
    SystemClock_Config();
    
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
    
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    os_sys_init(task1);
    while (1)
    {
    }
}


/*---------------------------------------------------------------------------
  Tasks : Implement tasks
 *---------------------------------------------------------------------------*/
__task void task1 (void){
    /* Obtain own system task identification number. */
    id1 = os_tsk_self();

    /* Create task2,3 and obtain its task identification number. */
    id2 = os_tsk_create (task2, 0);
    id3 = os_tsk_create (task3, 0);
        
    for (;;) {
        
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_6 | LL_GPIO_PIN_7);
        
        /* Signal to task3 that task1 has completed. */
        os_evt_set(0x0004, id3);

        /* Wait for completion of task2 activity. */
        /*  0xFFFF makes it wait without timeout. */
        /*  0x0001 represents bit 2. */
        os_evt_wait_or(0x0001, 0xFFFF);

        /* Wait for 60+40 clock ticks before restarting task1 activity. */
        os_dly_wait(20);
    }
}

__task void task2 (void) {
    for (;;) {
        /* Wait for completion of task1 activity. */
        /*  0xFFFF makes it wait without timeout. */
        /*  0x0002 represents bit 2. */
        os_evt_wait_or(0x0002, 0xFFFF);

        /* Wait for 40+20 clock ticks before starting task2 activity. */
        os_dly_wait(60);
        
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6);

        /* Signal to task1 that task2 has completed. */
        os_evt_set(0x0001, id1);
    }
}

__task void task3 (void) {
    for (;;) {
        /* Wait for completion of task1 activity. */
        /*  0xFFFF makes it wait without timeout. */
        /*  0x0004 represents bit 2. */
        os_evt_wait_or(0x0004, 0xFFFF);

        /* Wait for 20+60 clock ticks before starting task3 activity. */
        os_dly_wait(40);
        
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_7);

        /* Signal to task2 that task3 has completed. */
        os_evt_set(0x0002, id2);
    }
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
