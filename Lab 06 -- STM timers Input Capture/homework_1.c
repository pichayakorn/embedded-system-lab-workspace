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

/* For LCT4727 Configuration */
#define A           LL_GPIO_PIN_2
#define B           LL_GPIO_PIN_10
#define C           LL_GPIO_PIN_11
#define D           LL_GPIO_PIN_12
#define E           LL_GPIO_PIN_13
#define F           LL_GPIO_PIN_14
#define G           LL_GPIO_PIN_15
#define ALL_SEG     A | B | C | D | E | F | G

#define DIGIT_1     LL_GPIO_PIN_0
#define DIGIT_2     LL_GPIO_PIN_1
#define DIGIT_3     LL_GPIO_PIN_2
#define DIGIT_4     LL_GPIO_PIN_3
#define ALL_DIGIT   DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4

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
void LCT4727_config(void);
void display_map(float);
uint32_t segment_num_map(uint8_t);

void TIMx_IC_Config(void)
{
    LL_GPIO_InitTypeDef timic_gpio;
    LL_TIM_InitTypeDef timbase;
    LL_TIM_IC_InitTypeDef timic;
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    
    timic_gpio.Mode = LL_GPIO_MODE_ALTERNATE;
    timic_gpio.Pull = LL_GPIO_PULL_DOWN;
    timic_gpio.Pin = LL_GPIO_PIN_1 | LL_GPIO_PIN_3;
    timic_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    timic_gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    timic_gpio.Alternate = LL_GPIO_AF_1;
    LL_GPIO_Init(GPIOA, &timic_gpio);
    
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    
    // TIM_IC Configure CH1
    timic.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
    timic.ICFilter = LL_TIM_IC_FILTER_FDIV1;
    timic.ICPolarity = LL_TIM_IC_POLARITY_RISING;
    timic.ICPrescaler = LL_TIM_ICPSC_DIV1;
    LL_TIM_IC_Init(TIM2, LL_TIM_CHANNEL_CH2, &timic);
    
    // TIM_IC Configure CH4
    timic.ICPolarity = LL_TIM_IC_POLARITY_FALLING;
    LL_TIM_IC_Init(TIM2, LL_TIM_CHANNEL_CH4, &timic);
    
    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);
    
    LL_TIM_EnableIT_CC2(TIM2);
    LL_TIM_EnableIT_CC4(TIM2);
    
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH4);
    
    LL_TIM_EnableCounter(TIM2);
}

void GPIO_Config(void)
{
    LL_GPIO_InitTypeDef timic_gpio;
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    
    timic_gpio.Mode = LL_GPIO_MODE_OUTPUT;
    timic_gpio.Pull = LL_GPIO_PULL_NO;
    timic_gpio.Pin = LL_GPIO_PIN_2;
    timic_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    timic_gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    LL_GPIO_Init(GPIOA, &timic_gpio);

    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);
}

uint16_t uwIC1 = 0;
uint16_t uwIC2 = 0;
uint16_t uwDiff = 0;
uint16_t uhICIndex = 0;
uint8_t state = 0;
float period = 0;
float distance = 0;

uint32_t display[3] = { RESET, RESET, RESET };
uint32_t digit[3] = { DIGIT_2, DIGIT_3, DIGIT_4 };

int main()
{
    uint32_t TIM2CLK;
    uint32_t PSC;
    uint32_t IC1PSC;
    uint8_t i;

    SystemClock_Config();
    GPIO_Config();
    TIMx_IC_Config();
    LCT4727_config();
    
    while(1) {
        switch(state)
        {
            case 0:
                // Trigger measurement
                LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_2);
                LL_mDelay(1);
                LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);
                state = 1;
            break;
            
            case 1:
                // Measure uptime using TIM_IC, if uhIndex == 2, rising and falling duration is measured
                if(uhICIndex == 1)
                {
                    // Period calculation
                    PSC = LL_TIM_GetPrescaler(TIM2) + 1;
                    TIM2CLK = SystemCoreClock / PSC;
                    IC1PSC = __LL_TIM_GET_ICPSC_RATIO(LL_TIM_IC_GetPrescaler(TIM2, LL_TIM_CHANNEL_CH1));
                    
                    period = (uwDiff*(PSC) * 1.0) / (TIM2CLK *IC1PSC * 1.0); // calculate uptime period
                    distance = (period * 340) / 2 * 1E2; // centimeter unit
                    uhICIndex = 0;
                    state = 0;
                }
            break;
        }
        display_map(distance);
        for (i = 0; i < 3; ++i) {
            LL_GPIO_ResetOutputPin(GPIOC, ALL_DIGIT);       // Write 0 to GPIO port
            LL_GPIO_ResetOutputPin(GPIOB, ALL_SEG);         // Reset all segment (PB2, PB10 - PB15)
            LL_GPIO_SetOutputPin(GPIOB, display[i]);
            LL_GPIO_SetOutputPin(GPIOC, digit[i]);
        }
    }
}

void LCT4727_config(void)
{
    LL_GPIO_InitTypeDef lct4727_InitStruct;

    // Configure ltc4727js
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    
    lct4727_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    lct4727_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    lct4727_InitStruct.Pull = LL_GPIO_PULL_NO;
    lct4727_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    lct4727_InitStruct.Pin = ALL_SEG;
    LL_GPIO_Init(GPIOB, &lct4727_InitStruct);
    
    lct4727_InitStruct.Pin = DIGIT_2 | DIGIT_3 | DIGIT_4;
    LL_GPIO_Init(GPIOC, &lct4727_InitStruct);
}

void display_map(float distance) {
    for (uint8_t i = 0; i < 3; ++i) {
        switch(i) {
            case 0: display[i] = segment_num_map(display / 100);          break;
            case 1: display[i] = segment_num_map(display / 10 % 10);      break;
            case 2: display[i] = segment_num_map(display % 10);           break;
        }
    }
}

uint32_t segment_num_map(uint8_t num)
{
    switch(num) {
        case 0: return ZERO;
        case 1: return ONE;
        case 2: return TWO;
        case 3: return THREE;
        case 4: return FOUR;
        case 5: return FIVE;
        case 6: return SIX;
        case 7: return SEVEN;
        case 8: return EIGHT;
        case 9: return NINE;
    }
    return RESET;
}

void TIM2_IRQHandler(void)
{	  
    if(LL_TIM_IsActiveFlag_CC2(TIM2) == SET)
    {
        LL_TIM_ClearFlag_CC2(TIM2);
        
        // Detect rising edge
        uwIC1 = LL_TIM_IC_GetCaptureCH2(TIM2);
    }
    if(LL_TIM_IsActiveFlag_CC4(TIM2) == SET)
    {
        LL_TIM_ClearFlag_CC4(TIM2);

        // Detect rising edge
        uwIC2 = LL_TIM_IC_GetCaptureCH4(TIM2);
        
        if(uwIC2 > uwIC1)
        {
            uwDiff = uwIC2 - uwIC1;
        }
        else if(uwIC2 < uwIC1)
        {
            uwDiff = ((LL_TIM_GetAutoReload(TIM2) - uwIC1) + uwIC2) + 1;
        }
        uhICIndex = 1;
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
