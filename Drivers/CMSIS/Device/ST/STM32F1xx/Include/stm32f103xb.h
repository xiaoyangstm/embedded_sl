/**
 * @file stm32f103xb.h
 * @brief STM32F103xB Register Definitions
 */

#ifndef __STM32F103XB_H
#define __STM32F103XB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Memory Map */
#define FLASH_BASE          0x08000000UL
#define SRAM_BASE           0x20000000UL
#define PERIPH_BASE         0x40000000UL
#define APB1PERIPH_BASE     PERIPH_BASE
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x00010000UL)
#define AHBPERIPH_BASE      (PERIPH_BASE + 0x00020000UL)

/* GPIO Ports */
#define GPIOA_BASE          (APB2PERIPH_BASE + 0x0800UL)
#define GPIOB_BASE          (APB2PERIPH_BASE + 0x0C00UL)
#define GPIOC_BASE          (APB2PERIPH_BASE + 0x1000UL)
#define GPIOD_BASE          (APB2PERIPH_BASE + 0x1400UL)

/* I2C */
#define I2C1_BASE           (APB1PERIPH_BASE + 0x5400UL)
#define I2C2_BASE           (APB1PERIPH_BASE + 0x5800UL)

/* RCC */
#define RCC_BASE            (AHBPERIPH_BASE + 0x1000UL)

/* TIM */
#define TIM1_BASE           (APB2PERIPH_BASE + 0x2C00UL)
#define TIM2_BASE           (APB1PERIPH_BASE + 0x0000UL)
#define TIM3_BASE           (APB1PERIPH_BASE + 0x0400UL)
#define TIM4_BASE           (APB1PERIPH_BASE + 0x0800UL)

/* USART */
#define USART1_BASE         (APB2PERIPH_BASE + 0x3800UL)
#define USART2_BASE         (APB1PERIPH_BASE + 0x4400UL)
#define USART3_BASE         (APB1PERIPH_BASE + 0x4800UL)

/* DMA */
#define DMA1_BASE           (AHBPERIPH_BASE + 0x0000UL)
#define DMA2_BASE           (AHBPERIPH_BASE + 0x0400UL)

/* FLASH */
#define FLASH_R_BASE        (AHBPERIPH_BASE + 0x2000UL)

/* PWR */
#define PWR_BASE            (APB1PERIPH_BASE + 0x7000UL)

/* EXTI */
#define EXTI_BASE           (APB2PERIPH_BASE + 0x0400UL)

/* AFIO */
#define AFIO_BASE           (APB2PERIPH_BASE + 0x0000UL)

/* NVIC */
#define NVIC_BASE           (0xE000E100UL)
#define SCB_BASE            (0xE000ED00UL)
#define SysTick_BASE        (0xE000E010UL)

/* GPIO Register Structure */
typedef struct {
  volatile uint32_t CRL;
  volatile uint32_t CRH;
  volatile uint32_t IDR;
  volatile uint32_t ODR;
  volatile uint32_t BSRR;
  volatile uint32_t BRR;
  volatile uint32_t LCKR;
} GPIO_TypeDef;

/* I2C Register Structure */
typedef struct {
  volatile uint32_t CR1;
  volatile uint32_t CR2;
  volatile uint32_t OAR1;
  volatile uint32_t OAR2;
  volatile uint32_t DR;
  volatile uint32_t SR1;
  volatile uint32_t SR2;
  volatile uint32_t CCR;
  volatile uint32_t TRISE;
} I2C_TypeDef;

/* RCC Register Structure */
typedef struct {
  volatile uint32_t CR;
  volatile uint32_t CFGR;
  volatile uint32_t CIR;
  volatile uint32_t APB2RSTR;
  volatile uint32_t APB1RSTR;
  volatile uint32_t AHBENR;
  volatile uint32_t APB2ENR;
  volatile uint32_t APB1ENR;
  volatile uint32_t BDCR;
  volatile uint32_t CSR;
} RCC_TypeDef;

/* TIM Register Structure */
typedef struct {
  volatile uint32_t CR1;
  volatile uint32_t CR2;
  volatile uint32_t SMCR;
  volatile uint32_t DIER;
  volatile uint32_t SR;
  volatile uint32_t EGR;
  volatile uint32_t CCMR1;
  volatile uint32_t CCMR2;
  volatile uint32_t CCER;
  volatile uint32_t CNT;
  volatile uint32_t PSC;
  volatile uint32_t ARR;
  volatile uint32_t RCR;
  volatile uint32_t CCR1;
  volatile uint32_t CCR2;
  volatile uint32_t CCR3;
  volatile uint32_t CCR4;
  volatile uint32_t BDTR;
  volatile uint32_t DCR;
  volatile uint32_t DMAR;
} TIM_TypeDef;

/* USART Register Structure */
typedef struct {
  volatile uint32_t SR;
  volatile uint32_t DR;
  volatile uint32_t BRR;
  volatile uint32_t CR1;
  volatile uint32_t CR2;
  volatile uint32_t CR3;
  volatile uint32_t GTPR;
} USART_TypeDef;

/* EXTI Register Structure */
typedef struct {
  volatile uint32_t IMR;
  volatile uint32_t EMR;
  volatile uint32_t RTSR;
  volatile uint32_t FTSR;
  volatile uint32_t SWIER;
  volatile uint32_t PR;
} EXTI_TypeDef;

/* DMA Channel Structure */
typedef struct {
  volatile uint32_t CCR;
  volatile uint32_t CNDTR;
  volatile uint32_t CPAR;
  volatile uint32_t CMAR;
} DMA_Channel_TypeDef;

typedef struct {
  volatile uint32_t ISR;
  volatile uint32_t IFCR;
  DMA_Channel_TypeDef C1;
  DMA_Channel_TypeDef C2;
  DMA_Channel_TypeDef C3;
  DMA_Channel_TypeDef C4;
  DMA_Channel_TypeDef C5;
  DMA_Channel_TypeDef C6;
  DMA_Channel_TypeDef C7;
} DMA_TypeDef;

/* NVIC Structure */
typedef struct {
  volatile uint32_t ISER[8];
  uint32_t RESERVED0[24];
  volatile uint32_t ICER[8];
  uint32_t RESERVED1[24];
  volatile uint32_t ISPR[8];
  uint32_t RESERVED2[24];
  volatile uint32_t ICPR[8];
  uint32_t RESERVED3[24];
  volatile uint32_t IABR[8];
  uint32_t RESERVED4[56];
  volatile uint8_t IP[240];
} NVIC_Type;

typedef struct {
  volatile uint32_t CPUID;
  volatile uint32_t ICSR;
  volatile uint32_t VTOR;
  volatile uint32_t AIRCR;
  volatile uint32_t SCR;
  volatile uint32_t CCR;
  volatile uint8_t SHP[12];
  volatile uint32_t SHCSR;
  volatile uint32_t CFSR;
  volatile uint32_t HFSR;
  volatile uint32_t DFSR;
  volatile uint32_t MMFAR;
  volatile uint32_t BFAR;
  volatile uint32_t AFSR;
} SCB_Type;

typedef struct {
  volatile uint32_t CTRL;
  volatile uint32_t LOAD;
  volatile uint32_t VAL;
  volatile uint32_t CALIB;
} SysTick_Type;

/* PWR Register Structure */
typedef struct {
  volatile uint32_t CR;
  volatile uint32_t CSR;
} PWR_TypeDef;

/* FLASH Register Structure */
typedef struct {
  volatile uint32_t ACR;
  volatile uint32_t KEYR;
  volatile uint32_t OPTKEYR;
  volatile uint32_t SR;
  volatile uint32_t CR;
  volatile uint32_t AR;
  volatile uint32_t RESERVED;
  volatile uint32_t OBR;
  volatile uint32_t OPTWR;
} FLASH_TypeDef;

/* Peripheral Declarations */
#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD               ((GPIO_TypeDef *) GPIOD_BASE)
#define I2C1                ((I2C_TypeDef *) I2C1_BASE)
#define I2C2                ((I2C_TypeDef *) I2C2_BASE)
#define RCC                 ((RCC_TypeDef *) RCC_BASE)
#define TIM1                ((TIM_TypeDef *) TIM1_BASE)
#define TIM2                ((TIM_TypeDef *) TIM2_BASE)
#define TIM3                ((TIM_TypeDef *) TIM3_BASE)
#define TIM4                ((TIM_TypeDef *) TIM4_BASE)
#define USART1              ((USART_TypeDef *) USART1_BASE)
#define USART2              ((USART_TypeDef *) USART2_BASE)
#define USART3              ((USART_TypeDef *) USART3_BASE)
#define DMA1                ((DMA_TypeDef *) DMA1_BASE)
#define DMA2                ((DMA_TypeDef *) DMA2_BASE)
#define EXTI                ((EXTI_TypeDef *) EXTI_BASE)
#define NVIC                ((NVIC_Type *) NVIC_BASE)
#define SCB                 ((SCB_Type *) SCB_BASE)
#define SysTick             ((SysTick_Type *) SysTick_BASE)
#define PWR                 ((PWR_TypeDef *) PWR_BASE)
#define FLASH               ((FLASH_TypeDef *) FLASH_R_BASE)

/* SysTick Bits */
#define SysTick_CTRL_CLKSOURCE_Msk   (1 << 2)
#define SysTick_CTRL_TICKINT_Msk     (1 << 1)
#define SysTick_CTRL_ENABLE_Msk      (1 << 0)

/* RCC Bits */
#define RCC_APB2ENR_IOPAEN  (1 << 2)
#define RCC_APB2ENR_IOPBEN  (1 << 3)
#define RCC_APB2ENR_IOPCEN  (1 << 4)
#define RCC_APB2ENR_IOPDEN  (1 << 5)
#define RCC_APB2ENR_AFIOEN  (1 << 0)
#define RCC_APB1ENR_I2C1EN  (1 << 21)
#define RCC_APB1ENR_I2C2EN  (1 << 22)
#define RCC_APB2ENR_TIM1EN  (1 << 11)
#define RCC_APB1ENR_TIM2EN  (1 << 0)
#define RCC_APB1ENR_TIM3EN  (1 << 1)
#define RCC_APB1ENR_TIM4EN  (1 << 2)
#define RCC_APB1ENR_USART2EN (1 << 17)
#define RCC_APB2ENR_USART1EN (1 << 14)
#define RCC_AHBENR_DMA1EN   (1 << 0)

/* I2C Bits */
#define I2C_CR1_PE          (1 << 0)
#define I2C_CR1_START       (1 << 8)
#define I2C_CR1_STOP        (1 << 9)
#define I2C_CR1_ACK         (1 << 10)
#define I2C_CR1_SWRST       (1 << 15)
#define I2C_SR1_SB          (1 << 0)
#define I2C_SR1_ADDR        (1 << 1)
#define I2C_SR1_TXE         (1 << 7)
#define I2C_SR1_RXNE        (1 << 6)
#define I2C_SR1_BTF         (1 << 2)
#define I2C_SR2_BUSY        (1 << 1)
#define I2C_SR2_MSL         (1 << 0)

/* GPIO Bits */
#define GPIO_CRL_MODE0      (0x3 << 0)
#define GPIO_CRL_CNF0       (0x3 << 2)

/* System Clock Configuration */
void SystemInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F103XB_H */