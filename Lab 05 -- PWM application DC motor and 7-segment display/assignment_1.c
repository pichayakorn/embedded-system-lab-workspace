/* Base registers address header file */
#include "stm32l1xx.h"
/* Library related header files */
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_utils.h"
/* Timer peripheral driver included */
#include "stm32l1xx_ll_tim.h"

#define A           LL_GPIO_PIN_2
#define B           LL_GPIO_PIN_10
#define C           LL_GPIO_PIN_11
#define D           LL_GPIO_PIN_12
#define E           LL_GPIO_PIN_13
#define F           LL_GPIO_PIN_14
#define G           LL_GPIO_PIN_15
#define ALL_SEG     A | B | C | D | E | F | G

#define DEGIT1      LL_GPIO_PIN_0
#define DEGIT2      LL_GPIO_PIN_1
#define DEGIT3      LL_GPIO_PIN_2
#define DEGIT4      LL_GPIO_PIN_3
#define ALL_DEGIT   DEGIT1 | DEGIT2 | DEGIT3 | DEGIT4

#define ZERO        A | B | C | D | E | F
#define ONE         B | C
#define TWO         A | B | D | G | E
#define THREE       A | B | C | D | G
#define FOUR        B | C | F | G
#define FIVE        A | C | D | F | G
#define SIX         A | C | D | E | F | G
#define SEVEN       A | B | C
#define EIGHT       A | B | C | D | E | F | G
#define NINE        A | B | C | F | G

void SystemClock_Config(void);

int main()
{
    LL_GPIO_InitTypeDef lct4727_InitStruct;
    uint8_t i;
    uint32_t myId[4] = { ZERO, SEVEN, ONE, ZERO};
    uint32_t digit[4] = { DEGIT1, DEGIT2, DEGIT3, DEGIT4 };
    
    SystemClock_Config();
    
    // Configure ltc4727js
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    
    lct4727_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    lct4727_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    lct4727_InitStruct.Pull = LL_GPIO_PULL_NO;
    lct4727_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    lct4727_InitStruct.Pin = ALL_SEG;
    LL_GPIO_Init(GPIOB, &lct4727_InitStruct);
    
    lct4727_InitStruct.Pin = ALL_DEGIT;
    LL_GPIO_Init(GPIOC, &lct4727_InitStruct);

    while(1) {
        for (i = 0; i < 4; ++i) {
            LL_GPIO_ResetOutputPin(GPIOC, ALL_DEGIT);       // Write 0 to GPIO port
            LL_GPIO_ResetOutputPin(GPIOB, ALL_SEG);         // Reset all segment (PB2, PB10 - PB15)
            LL_GPIO_SetOutputPin(GPIOB, myId[i]);
            LL_GPIO_SetOutputPin(GPIOC, digit[i]);
            
            //LL_mDelay(1000);    // USE for DEBuG increase delay to see what's happenning when 7-seg is lit
        }
    }
}

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
