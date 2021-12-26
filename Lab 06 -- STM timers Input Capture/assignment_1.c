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

void SystemClock_Config(void);

void GPIO_Config(void)
{
    LL_GPIO_InitTypeDef TIMIC_GPIO_InitStruct;
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    // GPIO Configuration
    TIMIC_GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    TIMIC_GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
    TIMIC_GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    TIMIC_GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
    TIMIC_GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    TIMIC_GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    LL_GPIO_Init(GPIOA, &TIMIC_GPIO_InitStruct);
}

void TIMx_IC_Config(void)
{
    LL_TIM_IC_InitTypeDef TIMIC_InitStruct;

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    // TIM_IC Configgure CH1
    TIMIC_InitStruct.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
    TIMIC_InitStruct.ICFilter = LL_TIM_IC_FILTER_FDIV1_N2;
    TIMIC_InitStruct.ICPolarity = LL_TIM_IC_POLARITY_BOTHEDGE;
    TIMIC_InitStruct.ICPrescaler = LL_TIM_ICPSC_DIV1;
    LL_TIM_IC_Init(TIM2, LL_TIM_CHANNEL_CH1, &TIMIC_InitStruct);

    NVIC_SetPriority(TIM2_IRQn, 0);
    
    NVIC_EnableIRQ(TIM2_IRQn);
    LL_TIM_EnableIT_CC1(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);

    LL_TIM_EnableCounter(TIM2);
}

uint16_t uwIC1 = 0;
uint16_t uwIC2 = 0;
uint16_t uwDiff = 0;
uint16_t uhICIndex = 0;

uint32_t TIM2CLK;
uint32_t PSC;
uint32_t IC1PSC;
float period;

int main()
{
    SystemClock_Config();
    TIMx_IC_Config();
    GPIO_Config();

    while(1) {
        if (uhICIndex == 2) {
            // Period calculation
            PSC = LL_TIM_GetPrescaler(TIM2) + 1;
            TIM2CLK = SystemCoreClock / PSC;
            IC1PSC = __LL_TIM_GET_ICPSC_RATIO(LL_TIM_IC_GetPrescaler(TIM2, LL_TIM_CHANNEL_CH1));

            period = (uwDiff*(PSC) * 1.0) / (TIM2CLK * IC1PSC * 1.0) * 1E3;  // Calculate uptime period (ms)
            uhICIndex = 0; 
        }
    }
}

void TIM2_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_CC1(TIM2) == SET) {
        LL_TIM_ClearFlag_CC1(TIM2);

        // Detect 1st rising edge
        if (uhICIndex == 0) {
            uwIC1 = LL_TIM_IC_GetCaptureCH1(TIM2);
            uhICIndex = 1;
        } else if (uhICIndex == 1) {
            uwIC2 = LL_TIM_IC_GetCaptureCH1(TIM2);
            if (uwIC2 > uwIC1) {
                uwDiff = uwIC2 - uwIC1;
            } else {
                uwDiff = (LL_TIM_GetAutoReload(TIM2) - uwIC1) + uwIC2 + 1;
            }
            uhICIndex = 2;
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
