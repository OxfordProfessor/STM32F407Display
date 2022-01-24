/**
				项目名称：基于FreeRTOS及LVGL界面显示系统
 ******************************************************************************
 * @Entrance    main.c
 * @author  	张凯洋
 * @qq			3173244086
 * @version 	V1.2
 * @date    	2022-01-23
 ******************************************************************************
 * Change Logs:
 * Date           		Author        Notes				Changes						Bugs/unachieved improvement
 * 2022-01-23     		张凯洋     	  First version									1.无法显示，可能是没有提供心跳中断，也没有给出LVGL事务处理函数
 * 2022-01-24 14:08     张凯洋     	  V1.2				1.为LVGL提供了心跳			1.无法刷新屏幕
														2.为LVGL提供了事务处理函数	2.无法实现触摸来实现控制外设
														3.优化了main函数程序目录
 ******************************************************************************
 * @attention
 * All rights reserved
 ******************************************************************************
**/
/*********************
 *      INCLUDES
 *********************/
/*
	system include
*/
#include "sys.h"
#include "delay.h"
/*
	Function and Peripheral device include
*/
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "key.h"
#include "beep.h"
#include "string.h"
#include "stdio.h"
#include "display.h"
#include "sram.h"
/*
	FreeRTOS include
*/
#include "malloc.h"
#include "FreeRTOS.h"
#include "task.h"
/*
    LCD include
*/
#include "lcd.h"
#include "touch.h"
/*
    lvgl include
*/
#include "../lvgl.h"
#include "../lv_port_disp.h"
#include "../lv_port_indev.h"
/*********************
 *      TASK CREATE
 *********************/
/*
    start task
*/
#define START_TASK_PRIO		1			//任务优先级
#define START_STK_SIZE 		128  		//任务堆栈大小	
TaskHandle_t StartTask_Handler;			//任务句柄
void start_task(void *pvParameters);	//任务函数
/*
    led task
*/
#define LED0_TASK_PRIO		2			//任务优先级
#define LED0_STK_SIZE 		128  		//任务堆栈大小
TaskHandle_t Led0Task_Handler;			//任务句柄
void led0_task(void *pvParameters);		//任务函数
/*
    lvgl palpitate task(lvgl心跳服务函数)
*/
#define LVGLPAL_TASK_PRIO		3			//任务优先级
#define LVGLPAL_STK_SIZE 		128  		//任务堆栈大小
TaskHandle_t LAVGLPALTask_Handler;			//任务句柄
void lvglpal_task(void *pvParameters);		//任务函数
/*
    display task(lvgl显示函数)
*/
#define DISPLAY_TASK_PRIO		4		//任务优先级
#define DISPLAY_STK_SIZE 		256  	//任务堆栈大小	
TaskHandle_t DISPLAYTask_Handler;		//任务句柄
void display_task(void *pvParameters);	//任务函数
/*********************
 *   Global Variables
 *********************/
u32 free_size = 0;
/**************************************************************************
 *      						@Main START MAIN
 *								@Main 主函数入口处
 *************************************************************************/
int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组4
	delay_init(168);					//初始化延时函数
	uart_init(115200);     				//初始化串口
	FSMC_SRAM_Init();					//SRAM初始化
	LED_Init();		        			//初始化LED端口
	KEY_Init();							//初始化按键
	LCD_Init();							//初始化LCD
	tp_dev.init();						//初始化触摸屏
//	my_mem_init(SRAMIN);            	//初始化内部内存池
//  LCD_Clear(WHITE);					//清屏

	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数，创建两个任务，然后删除自身
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建LED0任务
    xTaskCreate((TaskFunction_t )led0_task,             
                (const char*    )"led0_task",           
                (uint16_t       )LED0_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LED0_TASK_PRIO,        
                (TaskHandle_t*  )&Led0Task_Handler);   
	//创建lvgl palpitate任务
	xTaskCreate((TaskFunction_t )lvglpal_task,             
                (const char*    )"lvglpal_task",           
                (uint16_t       )LVGLPAL_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LVGLPAL_TASK_PRIO,        
                (TaskHandle_t*  )&LAVGLPALTask_Handler);  
    //创建display任务
    xTaskCreate((TaskFunction_t )display_task,     
                (const char*    )"display_task",   
                (uint16_t       )DISPLAY_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )DISPLAY_TASK_PRIO,
                (TaskHandle_t*  )&DISPLAYTask_Handler); 
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}
//为LVGL接口提高心跳任务函数
void vApplicationTickHook()
{
	lv_tick_inc(10);			//提供LVGL心跳
}

void lvglpal_task(void *pvParameters)
{
	while(1)
	{
//		vTaskDelay(100);
//		free_size = uxTaskGetStackHighWaterMark(LED0_TaskHandle);
		//获取LVGL显示任务堆栈空间的使用情况
		free_size = uxTaskGetStackHighWaterMark(DISPLAYTask_Handler);	
		//获取FreeRTOS应用程序开始运行之后曾经存在的最小的未被分配的存储空间的字节
		free_size = xPortGetMinimumEverFreeHeapSize();					
	}
}
//led灯指示任务
void led0_task(void *pvParameters)
{
	while(1)
	{
		LED0=~LED0;
        vTaskDelay(500);			//延时500ms，也就是500个时钟节拍	
	}
}
//显示屏显示任务函数 
void display_task(void *pvParameters)
{
	TickType_t LastHandlerTime;

	lv_init();							//LVGL库总体初始化
    lv_port_disp_init();				//LVGL库显示初始化
    lv_port_indev_init();				//LVGL库触摸初试化
	lv_chart_test_start();				//运行初始页面
	while(1)
	{
		lv_task_handler();						//LVGL事务处理函数
		vTaskDelayUntil(&LastHandlerTime,5);	//延时最小延时时间
	}
} 


