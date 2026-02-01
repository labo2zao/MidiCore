/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Services/system/panic.h"
#include "Services/safe/safe_mode.h"
#include "Services/ui/ui.h"
#include "Services/watchdog/watchdog.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern TIM_HandleTypeDef htim6;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  HAL_RCC_NMI_IRQHandler();
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  
  /* === HARDFAULT DIAGNOSTIC - Check these in debugger! === */
  volatile uint32_t g_fault_pc = 0;      /* Faulting instruction address */
  volatile uint32_t g_fault_lr = 0;      /* Link register (caller) */
  volatile uint32_t g_fault_sp = 0;      /* Stack pointer at fault */
  volatile uint32_t g_fault_cfsr = 0;    /* Configurable Fault Status Reg */
  volatile uint32_t g_fault_hfsr = 0;    /* Hard Fault Status Reg */
  volatile uint32_t g_fault_mmfar = 0;   /* MemManage Fault Address */
  volatile uint32_t g_fault_bfar = 0;    /* Bus Fault Address */
  volatile uint32_t g_fault_exc_return = 0; /* Exception return value */
  
  /* Get the exception return value from LR to determine which stack was used */
  __asm volatile ("MOV %0, LR" : "=r" (g_fault_exc_return));
  
  /* 
   * EXC_RETURN bit 2 determines which stack pointer was used:
   *   Bit 2 = 0: MSP (Main Stack) - used before scheduler or in ISR
   *   Bit 2 = 1: PSP (Process Stack) - used by FreeRTOS tasks
   */
  if (g_fault_exc_return & 0x4) {
    /* PSP was used (FreeRTOS task context) */
    __asm volatile ("MRS %0, PSP" : "=r" (g_fault_sp));
  } else {
    /* MSP was used (before scheduler or ISR context) */
    __asm volatile ("MRS %0, MSP" : "=r" (g_fault_sp));
  }
  
  /* Stack frame: r0,r1,r2,r3,r12,lr,pc,xpsr (8 words at offsets 0-28) */
  if (g_fault_sp > 0x20000000 && g_fault_sp < 0x20020000) {
    /* Valid RAM address - safe to dereference */
    uint32_t *stack = (uint32_t*)g_fault_sp;
    g_fault_lr = stack[5];   /* Stacked LR (offset 20) */
    g_fault_pc = stack[6];   /* Stacked PC (offset 24) - WHERE THE CRASH HAPPENED! */
  }
  
  /* Read fault status registers */
  g_fault_cfsr = SCB->CFSR;   /* Bits: MMFSR[7:0], BFSR[15:8], UFSR[31:16] */
  g_fault_hfsr = SCB->HFSR;   /* Hard fault flags */
  g_fault_mmfar = SCB->MMFAR; /* Memory fault address */
  g_fault_bfar = SCB->BFAR;   /* Bus fault address */
  
  /* 
   * === IN CUBEIDE DEBUGGER ===
   * 1. Set breakpoint on the while(1) below
   * 2. When hit, check:
   *    - g_fault_pc: Right-click → Open Disassembly at address
   *    - g_fault_lr: Shows caller function
   *    - g_fault_cfsr: Decode fault type
   * 
   * CFSR bit meanings:
   *   Bit 0 (IACCVIOL):  Instruction access violation
   *   Bit 1 (DACCVIOL):  Data access violation  
   *   Bit 3 (MUNSTKERR): MemManage unstacking error
   *   Bit 4 (MSTKERR):   MemManage stacking error
   *   Bit 8 (IBUSERR):   Instruction bus error
   *   Bit 9 (PRECISERR): Precise data bus error
   *   Bit 10 (IMPRECISERR): Imprecise data bus error
   *   Bit 11 (UNSTKERR): Unstacking bus error
   *   Bit 12 (STKERR):   Stacking bus error
   *   Bit 16 (UNDEFINSTR): Undefined instruction
   *   Bit 17 (INVSTATE): Invalid state (Thumb)
   *   Bit 18 (INVPC):    Invalid PC
   *   Bit 19 (NOCP):     No coprocessor
   *   Bit 24 (UNALIGNED): Unaligned access
   *   Bit 25 (DIVBYZERO): Divide by zero
   */
  
  /* Keep variables alive for debugger */
  (void)g_fault_pc;
  (void)g_fault_lr;
  (void)g_fault_sp;
  (void)g_fault_cfsr;
  (void)g_fault_hfsr;
  (void)g_fault_mmfar;
  (void)g_fault_bfar;
  (void)g_fault_exc_return;
  
  panic_set(PANIC_HARDFAULT);
  safe_mode_set_forced(1u);
  ui_set_status_line("PANIC HF");
  watchdog_panic();
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* SET BREAKPOINT HERE - then check g_fault_* variables! */
    __NOP();
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 stream1 global interrupt.
  */
void DMA1_Stream1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream1_IRQn 0 */

  /* USER CODE END DMA1_Stream1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
  /* USER CODE BEGIN DMA1_Stream1_IRQn 1 */

  /* USER CODE END DMA1_Stream1_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */

  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream0 global interrupt.
  */
void DMA2_Stream0_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream0_IRQn 0 */

  /* USER CODE END DMA2_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi1_rx);
  /* USER CODE BEGIN DMA2_Stream0_IRQn 1 */

  /* USER CODE END DMA2_Stream0_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream3 global interrupt.
  */
void DMA2_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream3_IRQn 0 */

  /* USER CODE END DMA2_Stream3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi1_tx);
  /* USER CODE BEGIN DMA2_Stream3_IRQn 1 */

  /* USER CODE END DMA2_Stream3_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go FS global interrupt.
  */
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}

/* USER CODE BEGIN 1 */

// UART interrupt handlers for MIDI DIN reception
// These must be defined to enable interrupt-driven MIDI input

extern UART_HandleTypeDef huart1; // USART1
extern UART_HandleTypeDef huart2; // USART2
extern UART_HandleTypeDef huart3; // USART3
extern UART_HandleTypeDef huart5; // UART5

/**
  * @brief This function handles USART1 global interrupt (MIDI DIN3 / USB OTG shared)
  */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}

/**
  * @brief This function handles USART2 global interrupt (MIDI DIN1 - Primary)
  */
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}

/**
  * @brief This function handles USART3 global interrupt (MIDI DIN2)
  */
void USART3_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart3);
}

/**
  * @brief This function handles UART5 global interrupt (MIDI DIN4)
  */
void UART5_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart5);
}

/* USER CODE END 1 */
