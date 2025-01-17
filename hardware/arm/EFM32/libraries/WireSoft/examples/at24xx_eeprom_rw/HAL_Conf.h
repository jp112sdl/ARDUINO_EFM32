/*   ARDUINO HAL special config file, overriden default define
     config arduino/hal/os/interupt priority/......
     ARDUINO HAL 项目配置文件， 改写 系统、库、缺省定义，配置改变系统的缺省参数/中断优先级等,节约内存；

     define priority(配置的优先次序):
      1  HAL_Conf.h in sketch project path
 	  2  HAL_Conf.h in variants/{variant}/configs.
 	  3  use default define  (in cores/arduino/efm32/efm32_build_defines.h)

     !!! Don't rename this file, 请不要改变文件名和扩展名!!!
     use this config file for save space
     2017.12 by huawei
*/
#ifndef __HALSPECELCONFIG_H__
#define __HALSPECELCONFIG_H__

#define  USE_ARDUINOSTREAMING   1
#define  USE_WARESOFT           1
#ifdef DEBUG_EFM_USER /*DEBUG*/
/* ------------------------------------------------------------------
   set USE_ERRORBLINK 1 enable blink a err code for debug
   blink err code:
     HardFault       31
     MemManage fault 32
     BusFault        33
     UsageFault      34
     MallocFailed    22 (freertos if set configUSE_MALLOC_FAILED_HOOK 1)
     StackOverflow   23 (freertos if configCHECK_FOR_STACK_OVERFLOW 1)
     others          __LINE__   (err line from assert_failed or _Error_Handler )
*/
#define USE_ERRORBLINK 1
#endif /* DEBUG_EFM_USER */


#endif
