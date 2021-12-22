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
/* EXTI driver included */
#include "stm32l1xx_ll_exti.h"

/* For 1000ms update event */
#define TIMx_PSC            32000 
#define TIMx_ARR            1000

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
void TIM_BASE_Config(void);
void TIM_OC_GPIO_Config(void);
void TIM_OC_Config(void);
void L293D_Config(void);
void USER_GPIO_Config(void);
void EXTI_Config(void);
void LCT4727_config(void);
void display_map(void);
uint32_t segment_num_map(uint8_t);

uint32_t display[3] = { RESET, RESET, RESET };
uint32_t digit[3] = { DIGIT_2, DIGIT_3, DIGIT_4 };

int main()
{
    uint8_t i;

    SystemClock_Config();
    L293D_Config();
    USER_GPIO_Config();
    EXTI_Config();
    LCT4727_config();
    
    // Reset state
    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4);
    LL_TIM_SetCounter(TIM3, RESET);
    // EN
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);
    display_map();

    while(1) {
        for (i = 0; i < 3; ++i) {
            LL_GPIO_ResetOutputPin(GPIOC, ALL_DIGIT);       // Write 0 to GPIO port
            LL_GPIO_ResetOutputPin(GPIOB, ALL_SEG);         // Reset all segment (PB2, PB10 - PB15)
            LL_GPIO_SetOutputPin(GPIOB, display[i]);
            LL_GPIO_SetOutputPin(GPIOC, digit[i]);
        }
    }
}

void TIM_BASE_Config(void)
{
    LL_TIM_InitTypeDef TIMBASE_InitStruct;
    
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
    
    TIMBASE_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV2;
    TIMBASE_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIMBASE_InitStruct.Autoreload = TIMx_ARR - 1;
    TIMBASE_InitStruct.Prescaler =  TIMx_PSC - 1;
    
    LL_TIM_Init(TIM3, &TIMBASE_InitStruct);
}

void TIM_OC_GPIO_Config(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct;
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void TIM_OC_Config(void)
{
    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct;
    
    TIM_BASE_Config();
    TIM_OC_GPIO_Config();
    
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    TIM_OC_InitStruct.CompareValue = LL_TIM_GetAutoReload(TIM3);    // 100% duty
    LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
    /*Interrupt Configure*/
    NVIC_SetPriority(TIM3_IRQn, 1);
    NVIC_EnableIRQ(TIM3_IRQn);
    LL_TIM_EnableIT_CC2(TIM3);
    
    /*Start Output Compare in PWM Mode*/
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM3);
}

void L293D_Config(void)
{
    LL_GPIO_InitTypeDef l293d_InitStruct;

    TIM_OC_Config();
    
    // Configure l293d
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    
    l293d_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    l293d_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    l293d_InitStruct.Pull = LL_GPIO_PULL_NO;
    l293d_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    l293d_InitStruct.Pin = LL_GPIO_PIN_4 | LL_GPIO_PIN_7;
    LL_GPIO_Init(GPIOB, &l293d_InitStruct);

    l293d_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    l293d_InitStruct.Alternate = LL_GPIO_AF_2;
    l293d_InitStruct.Pin = LL_GPIO_PIN_5;
    LL_GPIO_Init(GPIOB, &l293d_InitStruct);
}

void USER_GPIO_Config(void) {
    /* Declare struct for GPIO config */
    LL_GPIO_InitTypeDef GPIO_InitStruct;
    
    /* Enable GPIOB clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    
    /* Config User-Botton GPIOA PA0 as input */
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);                  // Write configure value to registers
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

void EXTI_Config(void) {
    /* Enable System configuration controller clock */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    
    /* EXTI Line 0 (EXTI0) for PA0 */
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_0);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_0);
    
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_SetPriority(EXTI0_IRQn, 0);
}

void display_map(void) {
    uint8_t level = LL_TIM_OC_GetCompareCH2(TIM3) * 100 / TIMx_ARR;
    for (uint8_t i = 0; i < 3; ++i) {
        switch(i) {
            case 0: display[i] = segment_num_map(level / 100);          break;
            case 1: display[i] = segment_num_map(level / 10 % 10);      break;
            case 2: display[i] = segment_num_map(level % 10);           break;
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

void EXTI0_IRQHandler(void) {
    // Check if EXTI0 bit is setted by interrupt
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) == SET) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);             // Clear pending bit by writing 1
        
        uint32_t CCRx = LL_TIM_OC_GetCompareCH2(TIM3);
        if (CCRx == TIMx_ARR * 0.01) {
            CCRx = TIMx_ARR;
        } else {
            CCRx -= TIMx_ARR * 0.01;
        }
        LL_TIM_OC_SetCompareCH2(TIM3, CCRx);
        display_map();
    }
}

void TIM3_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_CC2(TIM3) == SET) {
        LL_TIM_ClearFlag_CC2(TIM3);
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
