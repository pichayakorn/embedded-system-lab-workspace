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

/* Key note */
#define SP                  (uint16_t)1

#define B_3                 (uint16_t)245
#define Db_4                (uint16_t)277
#define D_4                 (uint16_t)293
#define E_4                 (uint16_t)329
#define Gb_4                (uint16_t)370
#define G_4                 (uint16_t)392
#define Ab_4                (uint16_t)415
#define A_4                 (uint16_t)440
#define B_4                 (uint16_t)494
#define Db_5                (uint16_t)554
#define D_5                 (uint16_t)587
#define E_5                 (uint16_t)659
#define F_5                 (uint16_t)698
#define Gb_5                (uint16_t)740
#define G_5                 (uint16_t)784
#define Ab_5                (uint16_t)830
#define A_5                 (uint16_t)880
#define Bb_5                (uint16_t)932
#define B_5                 (uint16_t)988
#define C_6                 (uint16_t)1046
#define Db_6                (uint16_t)1109
#define D_6                 (uint16_t)1174
#define Eb_6                (uint16_t)1244
#define E_6                 (uint16_t)1318

/* For 3s update event */
#define TIM2_PSC            32000
#define TIM2_ARR            150

/* Fix PSC for key note */
#define TIM4_PSC            2

/* Macro function for ARR calculation */
#define ARR_CALCULATE(N)    ( (32E6) / ((TIM4_PSC) * (N)) )

uint16_t sheetNote[] = {D_6,Db_6,B_5,A_5,B_5,Db_6,D_6,Db_6,B_5,A_5,B_5,Db_6,D_6,Db_6,B_5,A_5,B_5,Db_6,D_6,Db_6,B_5,A_5,A_5,SP,
                        E_6,D_6,Db_6,B_5,Db_6,D_6,E_6,D_6,Db_6,B_5,Db_6,D_6,E_6,D_6,Db_6,B_5,Db_6,D_6,Db_6,D_6,Eb_6,E_6,E_6,SP,
                        B_5,Db_6,D_6,G_5,A_5,B_5,A_5,B_5,Db_6,Gb_5,G_5,A_5,G_5,A_5,B_5,E_5,Gb_5,G_5,Gb_5,G_5,A_5,D_5,E_5,Gb_5,
                        E_6,Eb_6,D_6,Db_6,C_6,B_5,Bb_5,A_5,Ab_5,G_5,Gb_5,F_5,E_5,E_5,SP,
                        A_4,A_4,SP,A_4,SP,A_4,B_4,SP,Db_5,
                        D_5,SP,D_4,D_4,SP,Gb_4,B_4,SP,Gb_4,A_4,A_4,SP,
                        A_4,SP,B_4,A_4,SP,Gb_4,G_4,SP,Gb_4,E_4,E_4,SP,
                        B_3,SP,E_4,E_4,SP,G_4,Db_5,SP,Db_5,B_4,SP,A_4,
                        G_4,G_4,SP,G_4,SP,Gb_4,B_3,B_3,Db_4,Db_4,SP,D_4,
                        E_4,E_4,E_4,E_4,E_4,E_4,E_4,E_4,E_4,SP,SP,SP,
                        A_4,SP,D_4,D_4,SP,Gb_4,B_4,SP,Gb_4,A_4,A_4,SP,
                        A_4,SP,B_4,A_4,SP,Gb_4,G_4,SP,Gb_4,E_4,E_4,SP,
                        B_3,SP,E_4,E_4,SP,G_4,Db_5,SP,Db_5,B_4,SP,A_4,
                        G_4,SP,G_4,Gb_4,SP,E_4,Db_4,Db_4,Db_4,E_4,E_4,E_4,
                        D_4,D_4,D_4,D_4,D_4,D_4,D_4,D_4,D_4,SP,SP,SP,
                        B_4,B_4,SP,B_4,SP,A_4,G_4,A_4,B_4,A_4,A_4,SP,
                        E_4,SP,Gb_4,Ab_4,SP,E_4,A_4,A_4,A_4,SP,SP,SP,
                        E_5,SP,Gb_5,Ab_5,SP,E_5,A_5,A_5,A_5,SP,SP,SP,
                        B_4,B_4,B_4,A_4,A_4,A_4,G_4,G_4,G_4,G_4,G_4,SP,
                        E_4,E_4,SP,Db_5,SP,B_4,A_4,SP,B_4,A_4,SP,G_4,
                        SP,SP,SP,A_4,SP,B_4,Gb_4,Gb_4,Gb_4,Gb_4,SP,E_4,
                        D_4,D_4,D_4,D_4,D_4,D_4,SP,SP,D_5,E_5,SP,Gb_5,
                        G_5,Gb_5,G_5,Gb_5,SP,G_5,Gb_5,SP,E_5,E_5,SP,F_5,
                        Gb_5,F_5,Gb_5,F_5,SP,Gb_5,E_5,SP,D_5,D_5,SP,Db_5,
                        B_4,SP,SP,B_4,SP,SP,Db_5,SP,D_5,E_5,E_5,SP,
                        B_4,SP,SP,B_4,SP,SP,Db_5,SP,D_5,E_5,SP,D_5,
                        SP,SP,SP,SP,SP,SP,D_5,D_5,D_5,D_5,D_5,D_5};
uint16_t cur = 0;

void SystemClock_Config(void);
void TIM_BASE_Config(uint16_t);
void TIM_OC_GPIO_Config(void);
void TIM_OC_Config(uint16_t);
void UPDATE_Keynote(uint16_t);

int main()
{
    SystemClock_Config();
    TIM_OC_Config(ARR_CALCULATE(sheetNote[cur]));

    while(1);
}

void TIM_BASE_Config(uint16_t ARR) {
    LL_TIM_InitTypeDef TIM_InitStruct;
    
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
    
    /* Time-base 4 configure */
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = ARR - 1;
    TIM_InitStruct.Prescaler = TIM4_PSC - 1;
    LL_TIM_Init(TIM4, &TIM_InitStruct);
    
    /* Time-base 2 configure */
    TIM_InitStruct.Autoreload = TIM2_ARR - 1;
    TIM_InitStruct.Prescaler = TIM2_PSC - 1;
    LL_TIM_Init(TIM2, &TIM_InitStruct);
}

void TIM_OC_GPIO_Config(void) {
    /* Declare struct for GPIO config */
    LL_GPIO_InitTypeDef GPIO_InitStruct;
    
    /* Enable GPIOB clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    
    /* Config PB6 as alternate function (TIM4_CH1) */
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void TIM_OC_Config(uint16_t note) {
    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct;
    
    TIM_BASE_Config(note);
    TIM_OC_GPIO_Config();
    
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    TIM_OC_InitStruct.CompareValue = LL_TIM_GetAutoReload(TIM4) / 2;    //  50% duty
    LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
      
    LL_TIM_ClearFlag_UPDATE(TIM2);
    LL_TIM_EnableIT_UPDATE(TIM2);
    
    /* Interrupt Configure */
    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);
    
    NVIC_SetPriority(TIM4_IRQn, 1);
    NVIC_EnableIRQ(TIM4_IRQn);
    LL_TIM_EnableIT_CC1(TIM4);
    
    /* Start Output Compare in PWM Mode */
    LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM4);
    
    LL_TIM_EnableCounter(TIM2);
}

void UPDATE_Keynote(uint16_t note) {
    if (note != SP) {
        uint16_t ARR = ARR_CALCULATE(note);
        LL_TIM_SetAutoReload(TIM4, ARR - 1);
        LL_TIM_OC_SetCompareCH1(TIM4, LL_TIM_GetAutoReload(TIM4) / 2);    //  50% duty
    } else {
        LL_TIM_SetAutoReload(TIM4, 1);
        LL_TIM_OC_SetCompareCH1(TIM4, 0);    //  0% duty
    }
}

void TIM2_IRQHandler(void) {
    if (LL_TIM_IsActiveFlag_CC1(TIM2) == SET) {
        LL_TIM_ClearFlag_CC1(TIM2);
        if (cur != sizeof(sheetNote)/sizeof(sheetNote[0]) - 1) {
            UPDATE_Keynote(sheetNote[++cur]);           
        } else {
            LL_TIM_OC_SetCompareCH1(TIM4, 0);    //  0% duty
        }
    }
}

void TIM4_IRQHandler(void) {
    if (LL_TIM_IsActiveFlag_CC1(TIM4) == SET) {
        LL_TIM_ClearFlag_CC1(TIM4);
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
