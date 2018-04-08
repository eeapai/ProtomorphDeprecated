////////////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License                                                           //
//                                                                                //
// Copyright (c) 2018, pa.eeapai@gmail.com                                        //
// All rights reserved.                                                           //
//                                                                                //
// Redistribution and use in source and binary forms, with or without             //
// modification, are permitted provided that the following conditions are met:    //
//                                                                                //
// * Redistributions of source code must retain the above copyright notice, this  //
//   list of conditions and the following disclaimer.                             //
//                                                                                //
// * Redistributions in binary form must reproduce the above copyright notice,    //
//   this list of conditions and the following disclaimer in the documentation    //
//   and/or other materials provided with the distribution.                       //
//                                                                                //
// * Neither the name of the copyright holder nor the names of its                //
//   contributors may be used to endorse or promote products derived from         //
//   this software without specific prior written permission.                     //
//                                                                                //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"    //
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE      //
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE   //
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL     //
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR     //
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER     //
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,  //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  //
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.           //
////////////////////////////////////////////////////////////////////////////////////
#include "string.h"
#include "stdio.h"

#include "stm32f1xx.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_io32.h"

#include "WinUSBSTM32Device.h"

#include "HAL_IO32STM32Impl.h"
#include "HAL_IO32ICommHost.h"

#define SWBKPT() { asm(" BKPT #0"); }

void SystemClock_Config();
void Configure_USART(void);

volatile bool g_bContinue = false;
#define WAIT() { SWBKPT(); while(!g_bContinue); }

unsigned char g_abyCommBuf[8*1024 + 1];
static const CHAL_IO32_STM32Impl::SInitConf s_IO32STMConf =
{
  {
    { GPIOB, GPIO_PIN_12 },
    { GPIOB, GPIO_PIN_13 },
    {nullptr},//{ GPIOB, GPIO_PIN_14 }, //USB pull-up
    { GPIOB, GPIO_PIN_15 },
    { GPIOA, GPIO_PIN_8 },
    {nullptr},//{ GPIOA, GPIO_PIN_9 },  // UART
    {nullptr},//{ GPIOA, GPIO_PIN_10 }, // UART
    {nullptr},//    { GPIOA, GPIO_PIN_11 }, // USB
    {nullptr},//    { GPIOA, GPIO_PIN_12 }, // USB
    { GPIOA, GPIO_PIN_15 },
    { GPIOB, GPIO_PIN_3 },
    { GPIOB, GPIO_PIN_4 },
    { GPIOB, GPIO_PIN_5 },
    { GPIOB, GPIO_PIN_6 },
    { GPIOB, GPIO_PIN_7 }, // mode high
    { GPIOB, GPIO_PIN_8 }, // mode sense
    { GPIOB, GPIO_PIN_9 }, // mode low
    { GPIOB, GPIO_PIN_11 },
    { GPIOB, GPIO_PIN_10 },
    { GPIOB, GPIO_PIN_1 },
    { GPIOB, GPIO_PIN_0 },
    { GPIOA, GPIO_PIN_7 },
    { GPIOA, GPIO_PIN_6 },
    { GPIOA, GPIO_PIN_5 },
    { GPIOA, GPIO_PIN_4 },
    { GPIOA, GPIO_PIN_3 }, // BME280.Data
    { GPIOA, GPIO_PIN_2 }, // BME280.Clk
    { GPIOA, GPIO_PIN_1 }, // BME280.GND
    { GPIOA, GPIO_PIN_0 }, // BMO280.Vcc
    { GPIOC, GPIO_PIN_15 },
    { GPIOC, GPIO_PIN_14 },
    { GPIOC, GPIO_PIN_13 }, // LED
  }
};
//COMPILETIME_ASSERT(_countof(s_aIO32GPIOs) == IHAL_IO32::numPins);
int main()
{
  HAL_Init();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  /* Configure the system clock to 72 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();
  Configure_USART();
  DBGLOG("\33[2J\n\r");    // clear screen command
  DBGLOG("\33[3J\n\r");    // clear scroll back
  printf("Here I come, here I go\r\n");

  CHAL_IO32_STM32Impl io32(&s_IO32STMConf);
  unsigned char byLEDOn = IHAL_IO32::pin31 | IHAL_IO32::functionLow;
  unsigned char byLEDOff = IHAL_IO32::pin31 | IHAL_IO32::functionIn;
  io32.DoIO(1, &byLEDOn, 0);
  io32.DoIO(1, &byLEDOff, 0);

  const unsigned char abyModeOut[] =
  {
      IHAL_IO32::pin14 | IHAL_IO32::functionHigh,
      IHAL_IO32::pin15 | IHAL_IO32::functionIn,
      IHAL_IO32::pin16 | IHAL_IO32::functionLow,
      IHAL_IO32::pin15 | IHAL_IO32::functionRead
  };
  unsigned char byModeIn = 0;

  io32.DoIO(sizeof(abyModeOut), abyModeOut, &byModeIn, 1);
  bool bModeIO32 = IHAL_IO32::IsReadLevelHigh(byModeIn);
  if ( bModeIO32 )
  {
    io32.DoIO(1, &byLEDOn, 0);
  }

  CWinUSBSTM32Device USBDev( g_abyCommBuf, sizeof(g_abyCommBuf));
  ICommDevice *pCommDev = &USBDev;

  SIO32STM32USB IO32STM32USBContext = { 0 };
  IO32STM32USBContext.m_pWinUSBContext = &USBDev;
  USBD_HandleTypeDef USBD_Device;
  USBD_Device.pUserData = &IO32STM32USBContext;

  /* Init IO32 Application */
  USBD_Init(&USBD_Device, &IO32_Desc, 0);

  /* Add Supported Class */
  USBD_RegisterClass(&USBD_Device, &USBD_IO32_ClassDriver);

  /* Start Device Process */
  USBD_Start(&USBD_Device);

  // Optimization trick:
  // Give comm buff to IO32 for pin buffer so that memmove finishes faster when reading
  // data arriving on communication channel.
  CHAL_IO32_ICommHost io32Host(&io32, g_abyCommBuf, sizeof(g_abyCommBuf), pCommDev, "");

  while ( bModeIO32 )
  {
    io32Host.HostProcess();
  }

  unsigned long dwNumBytes = 0;
  unsigned char *pbyReceived = NULL;
  while ( true )
  {
    if ( ICommDevice::connectionConnected != pCommDev->GetStatus() )
    {
      pCommDev->Disconnect();
      pCommDev->Connect(nullptr);
      continue;
    }
    pCommDev->Receive(g_abyCommBuf, sizeof(g_abyCommBuf), &dwNumBytes);
    if ( ICommDevice::connectionConnected != pCommDev->GetStatus() )
    {
      continue;
    }
    if ( !dwNumBytes )
    {
      continue;
    }
    pCommDev->Send(g_abyCommBuf, dwNumBytes, nullptr);
  }
  return 0;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  while (1)
  {
    SWBKPT();
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            HSE PREDIV1                    = 1
  *            PLLMUL                         = 9
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config()
{
  RCC_ClkInitTypeDef clkinitstruct = { 0 };
  RCC_OscInitTypeDef oscinitstruct = { 0 };
  RCC_PeriphCLKInitTypeDef rccperiphclkinit = { 0 };

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  oscinitstruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState = RCC_HSE_ON;
  oscinitstruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLMUL = RCC_PLL_MUL9;

  oscinitstruct.PLL.PLLState = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  if (HAL_RCC_OscConfig(&oscinitstruct) != HAL_OK)
  {
    /* Start Conversation Error */
    Error_Handler();
  }

  /* USB clock selection */
  rccperiphclkinit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  rccperiphclkinit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  HAL_RCCEx_PeriphCLKConfig(&rccperiphclkinit);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
   * clocks dividers */
  clkinitstruct.ClockType =
    (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
     RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2) != HAL_OK)
  {
    /* Start Conversation Error */
    Error_Handler();
  }
}


#define USARTx_INSTANCE               USART1
#define USARTx_CLK_ENABLE()           LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1)

#define USARTx_GPIO_CLK_ENABLE()      LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA)   /* Enable the peripheral clock of GPIOA */
#define USARTx_TX_PIN                 LL_GPIO_PIN_9
#define USARTx_TX_GPIO_PORT           GPIOA
#define USARTx_RX_PIN                 LL_GPIO_PIN_10
#define USARTx_RX_GPIO_PORT           GPIOA
#define APB_Div 1

/**
  * @brief  This function configures USARTx Instance.
  * @note   This function is used to :
  *         -1- Enable GPIO clock and configures the USART pins.
  *         -2- Configure USART functional parameters.
  *         -3- Enable USART.
  * @note   Peripheral configuration is minimal configuration from reset values.
  *         Thus, some useless LL unitary functions calls below are provided as
  *         commented examples - setting is default configuration from reset.
  * @param  None
  * @retval None
  */
void Configure_USART(void)
{

  /* (1) Enable GPIO clock and configures the USART pins *********************/

  /* Enable the peripheral clock of GPIO Port */
  USARTx_GPIO_CLK_ENABLE();

  /* Enable USART peripheral clock *******************************************/
  USARTx_CLK_ENABLE();

  /* Configure Tx Pin as : Alternate function, High Speed, Push pull, Pull up */
  LL_GPIO_SetPinMode(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinSpeed(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_PULL_UP);

  /* Configure Rx Pin as : Input Floating function, High Speed, Pull up */
  LL_GPIO_SetPinMode(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_MODE_FLOATING);
  LL_GPIO_SetPinSpeed(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinPull(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_PULL_UP);


  /* (2) Configure USART functional parameters ********************************/

  /* Disable USART prior modifying configuration registers */
  /* Note: Commented as corresponding to Reset value */
  // LL_USART_Disable(USARTx_INSTANCE);

  /* TX/RX direction */
  LL_USART_SetTransferDirection(USARTx_INSTANCE, LL_USART_DIRECTION_TX_RX);

  /* 8 data bit, 1 start bit, 1 stop bit, no parity */
  LL_USART_ConfigCharacter(USARTx_INSTANCE, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);

  /* No Hardware Flow control */
  /* Reset value is LL_USART_HWCONTROL_NONE */
  // LL_USART_SetHWFlowCtrl(USARTx_INSTANCE, LL_USART_HWCONTROL_NONE);

  /* Set Baudrate to 115200 using APB frequency set to 72000000/APB_Div Hz */
  /* Frequency available for USART peripheral can also be calculated through LL RCC macro */
  /* Ex :
      Periphclk = LL_RCC_GetUSARTClockFreq(Instance); or LL_RCC_GetUARTClockFreq(Instance); depending on USART/UART instance

      In this example, Peripheral Clock is expected to be equal to 72000000/APB_Div Hz => equal to SystemCoreClock/APB_Div
  */
  LL_USART_SetBaudRate(USARTx_INSTANCE, SystemCoreClock/APB_Div, 115200);

  /* (3) Enable USART *********************************************************/
  LL_USART_Enable(USARTx_INSTANCE);
}
#ifdef __cplusplus
extern "C" {
#endif


/// Minimal sys impl
int PutChar(int ch)
{
  // Output to trace, UART, shared memory etc.
  while (!LL_USART_IsActiveFlag_TXE(USARTx_INSTANCE));
  LL_USART_TransmitData8(USARTx_INSTANCE, ch);
  return ch;
}

#include <sys/stat.h>

int _close(int file)
{
  return 0;
}

int _fstat(int file, struct stat *st)
{
  st->st_mode =  S_IFSOCK;//S_IFCHR;// S_IFIFO;
  return 0;
}

int _isatty(int file)
{
  return 1;
}

int _lseek(int file, int ptr, int dir)
{
  return 0;
}

int _read(int file, char *ptr, int len)
{
  return 0;
}

int _init()
{
  return 0;
}

caddr_t _sbrk(int incr)
{
  extern char _end;
  static char *heap_end;
  char *prev_heap_end;
  if (heap_end == 0)
  {
    heap_end = &_end;
  }
  prev_heap_end = heap_end;
  heap_end += incr;
  return (caddr_t) prev_heap_end;
}

int _write(int file, char *ptr, int len)
{
  int i;
  for ( i=0; i < len; i++ )
  {
    PutChar(*ptr++);
  }
  return len;
}
//////////////////////////////////////


typedef void (*pfunc)(void);

/* start address for the initialization values of the .data section defined in linker script */
extern unsigned long _sidata;
/* start address for the .data section. defined in linker script */
extern unsigned long _sdata;
/* end address for the .data section. defined in linker script */
extern unsigned long _edata;

/* start address for the .bss section. defined in linker script */
extern unsigned long _sbss;
/* end address for the .bss section. defined in linker script */
extern unsigned long _ebss;

extern pfunc _sinit_array[];
extern pfunc _einit_array[];

extern pfunc _sfinit_array[];
extern pfunc _efinit_array[];

int __cxa_atexit(void (*destroyer)(void*), void* object, void* dso_handle);
int __aeabi_atexit(void* object, void (*destroyer)(void*), void* dso_handle)
{
  return __cxa_atexit(destroyer, object, dso_handle);
}

void __cxa_pure_virtual(void)
{
  while (1);
}

void *__dso_handle;

int __cxa_atexit(void (*destructor) (void *), void *arg, void *dso)
{
  return 0;
}
void __cxa_finalize(void *f)
{

}

void operator delete(void *what)
{
  asm(" BKPT");
  while( 1 );
}
__attribute__( ( naked ) )
void NMI_Handler()
{
  asm(" BKPT");
  while( 1 );
}

typedef struct
{
  unsigned long m_dwExR0;
  unsigned long m_dwExR1;
  unsigned long m_dwExR2;
  unsigned long m_dwExR3;
  unsigned long m_dwExR12;
  unsigned long m_dwExLR;
  unsigned long m_dwExPC;
  unsigned long m_dwExPSR;
}SARMv7MExceptionContext;

__attribute__( ( naked ) )
void HardFault_Handler()
{
  register SARMv7MExceptionContext * pARMv7MExceptionContext __asm("r0");
  register unsigned long dwLR __asm("lr");

  if ( dwLR & 4 )
    asm(" mrs r0, psp");
  else
    asm(" mrs r0, msp");

  SWBKPT();
  while ( 1 );
}
__attribute__( ( naked ) )
void MemManage_Handler()
{
  SWBKPT();
  while( 1 );
}

__attribute__( ( naked ) )
void BusFault_Handler()
{
  SWBKPT();
  while( 1 );
}

__attribute__( ( naked ) )
void UsageFault_Handler()
{
  SWBKPT();
  while( 1 );
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

__attribute__( ( naked ) )
void defaultHandler()
{
  SWBKPT();
  while( 1 );
}

__attribute__( ( naked ) )
void WWDG_IRQHandler()
{
  SWBKPT();
  while( 1 );
}

__attribute__( ( naked ) )
void PVD_IRQHandler()
{
  SWBKPT();
  while( 1 );
}

extern PCD_HandleTypeDef hpcd;
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd);
}

#if 1
/* Copy the data segment initializers from flash to SRAM */
static void copyData(void)
{
  volatile unsigned long *pdwDataSrc = &_sidata;
  volatile unsigned long *pdwDataDest = &_sdata;

  if ( &_sdata == &_edata )
  {
   return;
  }

  if ( &_sidata == &_sdata )
  {
   return;
  }

  while( pdwDataDest < &_edata )
  {
    *pdwDataDest++ = *pdwDataSrc++;
  }
}

/* Zero fill the bss segment. */
static void zeroBss(void)
{
  volatile unsigned long *pdwDataDest = &_sbss;

  if ( &_sbss == &_ebss )
  {
   return;
  }

  while ( pdwDataDest < &_ebss )
  {
    *pdwDataDest++ = 0;
  }
}

/* Call static constructors */
static void callConstructors(void)
{
  pfunc *pfTable;
  if ( _sinit_array == _einit_array )
  {
   return;
  }

  for ( pfTable = _einit_array - 1; pfTable - _sinit_array >= 0; pfTable-- )
  {
    pfTable[0]();
  }
}

/* Call static destructors */

//static void callDestructors(void)
//{
//  pfunc *pfTable;
//  if ( _sfinit_array == _efinit_array )
//  {
//   return;
//  }
//
//  for ( pfTable = _efinit_array - 1; pfTable - _sfinit_array >= 0; pfTable-- )
//  {
//    pfTable[0]();
//  }
//}
void _exit (int a)
{
  while(1) {};
}

extern const unsigned long g_adwVectors[];
extern unsigned long _estack;
__attribute__( ( naked ) )
void Reset_Handler(void)
{
  register unsigned long *pulSP __asm("sp") = &_estack;
  copyData();
  zeroBss();
  callConstructors();
//  atexit();
//  _init();
  SystemInit();
  main();
  *pulSP = g_adwVectors[0];
//  exit();
  while( 1 );
}

#define BootRAM 0xF108F85F
__attribute__ ((section(".isr_vector")))
const unsigned long g_adwVectors[] =
{
  (unsigned long)&_estack                     ,
  (unsigned long)Reset_Handler                ,
  (unsigned long)NMI_Handler                  ,
  (unsigned long)HardFault_Handler            ,
  (unsigned long)MemManage_Handler            ,
  (unsigned long)BusFault_Handler             ,
  (unsigned long)UsageFault_Handler           ,
  (unsigned long) 0                           ,
  (unsigned long) 0                           ,
  (unsigned long) 0                           ,
  (unsigned long) 0                           ,
  (unsigned long) SVC_Handler                 ,
  (unsigned long) DebugMon_Handler            ,
  (unsigned long) 0                           ,
  (unsigned long) PendSV_Handler              ,
  (unsigned long) SysTick_Handler             ,
  (unsigned long) WWDG_IRQHandler             , //  WWDG_IRQHandler             ,
  (unsigned long) PVD_IRQHandler             , //  PVD_IRQHandler              ,
  (unsigned long) defaultHandler             , //  TAMPER_IRQHandler           ,
  (unsigned long) defaultHandler             , //  RTC_IRQHandler              ,
  (unsigned long) defaultHandler             , //  FLASH_IRQHandler            ,
  (unsigned long) defaultHandler             , //  RCC_IRQHandler              ,
  (unsigned long) defaultHandler             , //  EXTI0_IRQHandler            ,
  (unsigned long) defaultHandler             , //  EXTI1_IRQHandler            ,
  (unsigned long) defaultHandler             , //  EXTI2_IRQHandler            ,
  (unsigned long) defaultHandler             , //  EXTI3_IRQHandler            ,
  (unsigned long) defaultHandler             , //  EXTI4_IRQHandler            ,
  (unsigned long) defaultHandler             , //  DMA1_Channel1_IRQHandler    ,
  (unsigned long) defaultHandler             , //  DMA1_Channel2_IRQHandler    ,
  (unsigned long) defaultHandler             , //  DMA1_Channel3_IRQHandler    ,
  (unsigned long) defaultHandler             , //  DMA1_Channel4_IRQHandler    ,
  (unsigned long) defaultHandler             , //  DMA1_Channel5_IRQHandler    ,
  (unsigned long) defaultHandler             , //  DMA1_Channel6_IRQHandler    ,
  (unsigned long) defaultHandler             , //  DMA1_Channel7_IRQHandler    ,
  (unsigned long) defaultHandler             , //  ADC1_2_IRQHandler           ,
  (unsigned long) defaultHandler             , //  USB_HP_CAN1_TX_IRQHandler   ,
  (unsigned long) USB_LP_CAN1_RX0_IRQHandler  ,
  (unsigned long) defaultHandler             , //  CAN1_RX1_IRQHandler         ,
  (unsigned long) defaultHandler             , //  CAN1_SCE_IRQHandler         ,
  (unsigned long) defaultHandler             , //  EXTI9_5_IRQHandler          ,
  (unsigned long) defaultHandler             , //  TIM1_BRK_IRQHandler         ,
  (unsigned long) defaultHandler             , //  TIM1_UP_IRQHandler          ,
  (unsigned long) defaultHandler             , //  TIM1_TRG_COM_IRQHandler     ,
  (unsigned long) defaultHandler             , //  TIM1_CC_IRQHandler          ,
  (unsigned long) defaultHandler             , //  TIM2_IRQHandler             ,
  (unsigned long) defaultHandler             , //  TIM3_IRQHandler             ,
  (unsigned long) defaultHandler             , //  TIM4_IRQHandler             ,
  (unsigned long) defaultHandler             , //  I2C1_EV_IRQHandler          ,
  (unsigned long) defaultHandler             , //  I2C1_ER_IRQHandler          ,
  (unsigned long) defaultHandler             , //  I2C2_EV_IRQHandler          ,
  (unsigned long) defaultHandler             , //  I2C2_ER_IRQHandler          ,
  (unsigned long) defaultHandler             , //  SPI1_IRQHandler             ,
  (unsigned long) defaultHandler             , //  SPI2_IRQHandler             ,
  (unsigned long) defaultHandler             , //  USART1_IRQHandler           ,
  (unsigned long) defaultHandler             , //  USART2_IRQHandler           ,
  (unsigned long) defaultHandler              , //  USART3_IRQHandler           ,
  (unsigned long) defaultHandler             , //  EXTI15_10_IRQHandler        ,
  (unsigned long) defaultHandler            , //  RTC_Alarm_IRQHandler        ,
  (unsigned long) defaultHandler            , //  USBWakeUp_IRQHandler        ,
  (unsigned long) 0                           ,
  (unsigned long) 0                           ,
  (unsigned long) 0                           ,
  (unsigned long) 0                           ,
  (unsigned long) 0                           ,
  (unsigned long) 0                           ,
  (unsigned long) 0                           ,
  (unsigned long) BootRAM           // @0x108. This is for boot in RAM mode for
                                    //STM32F10x Medium Density devices.
};
#endif

#ifdef __cplusplus
}
#endif
