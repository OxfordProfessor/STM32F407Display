/**
				��Ŀ���ƣ�����FreeRTOS��LVGL������ʾϵͳ
 ******************************************************************************
 * @Entrance    main.c
 * @author  	�ſ���
 * @qq			3173244086
 * @version 	V1.2
 * @date    	2022-01-23
 ******************************************************************************
 * Change Logs:
 * Date           		Author        Notes				Changes						Bugs/unachieved improvement
 * 2022-01-23     		�ſ���     	  First version									1.�޷���ʾ��������û���ṩ�����жϣ�Ҳû�и���LVGL��������
 * 2022-01-24 14:08     �ſ���     	  V1.2				1.ΪLVGL�ṩ������			1.�޷�ˢ����Ļ
														2.ΪLVGL�ṩ����������	2.�޷�ʵ�ִ�����ʵ�ֿ�������
														3.�Ż���main��������Ŀ¼
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
#define START_TASK_PRIO		1			//�������ȼ�
#define START_STK_SIZE 		128  		//�����ջ��С	
TaskHandle_t StartTask_Handler;			//������
void start_task(void *pvParameters);	//������
/*
    led task
*/
#define LED0_TASK_PRIO		2			//�������ȼ�
#define LED0_STK_SIZE 		128  		//�����ջ��С
TaskHandle_t Led0Task_Handler;			//������
void led0_task(void *pvParameters);		//������
/*
    lvgl palpitate task(lvgl����������)
*/
#define LVGLPAL_TASK_PRIO		3			//�������ȼ�
#define LVGLPAL_STK_SIZE 		128  		//�����ջ��С
TaskHandle_t LAVGLPALTask_Handler;			//������
void lvglpal_task(void *pvParameters);		//������
/*
    display task(lvgl��ʾ����)
*/
#define DISPLAY_TASK_PRIO		4		//�������ȼ�
#define DISPLAY_STK_SIZE 		256  	//�����ջ��С	
TaskHandle_t DISPLAYTask_Handler;		//������
void display_task(void *pvParameters);	//������
/*********************
 *   Global Variables
 *********************/
u32 free_size = 0;
/**************************************************************************
 *      						@Main START MAIN
 *								@Main ��������ڴ�
 *************************************************************************/
int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	FSMC_SRAM_Init();					//SRAM��ʼ��
	LED_Init();		        			//��ʼ��LED�˿�
	KEY_Init();							//��ʼ������
	LCD_Init();							//��ʼ��LCD
	tp_dev.init();						//��ʼ��������
//	my_mem_init(SRAMIN);            	//��ʼ���ڲ��ڴ��
//  LCD_Clear(WHITE);					//����

	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ������������������������Ȼ��ɾ������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
    //����LED0����
    xTaskCreate((TaskFunction_t )led0_task,             
                (const char*    )"led0_task",           
                (uint16_t       )LED0_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LED0_TASK_PRIO,        
                (TaskHandle_t*  )&Led0Task_Handler);   
	//����lvgl palpitate����
	xTaskCreate((TaskFunction_t )lvglpal_task,             
                (const char*    )"lvglpal_task",           
                (uint16_t       )LVGLPAL_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LVGLPAL_TASK_PRIO,        
                (TaskHandle_t*  )&LAVGLPALTask_Handler);  
    //����display����
    xTaskCreate((TaskFunction_t )display_task,     
                (const char*    )"display_task",   
                (uint16_t       )DISPLAY_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )DISPLAY_TASK_PRIO,
                (TaskHandle_t*  )&DISPLAYTask_Handler); 
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}
//ΪLVGL�ӿ��������������
void vApplicationTickHook()
{
	lv_tick_inc(10);			//�ṩLVGL����
}

void lvglpal_task(void *pvParameters)
{
	while(1)
	{
//		vTaskDelay(100);
//		free_size = uxTaskGetStackHighWaterMark(LED0_TaskHandle);
		//��ȡLVGL��ʾ�����ջ�ռ��ʹ�����
		free_size = uxTaskGetStackHighWaterMark(DISPLAYTask_Handler);	
		//��ȡFreeRTOSӦ�ó���ʼ����֮���������ڵ���С��δ������Ĵ洢�ռ���ֽ�
		free_size = xPortGetMinimumEverFreeHeapSize();					
	}
}
//led��ָʾ����
void led0_task(void *pvParameters)
{
	while(1)
	{
		LED0=~LED0;
        vTaskDelay(500);			//��ʱ500ms��Ҳ����500��ʱ�ӽ���	
	}
}
//��ʾ����ʾ������ 
void display_task(void *pvParameters)
{
	TickType_t LastHandlerTime;

	lv_init();							//LVGL�������ʼ��
    lv_port_disp_init();				//LVGL����ʾ��ʼ��
    lv_port_indev_init();				//LVGL�ⴥ�����Ի�
	lv_chart_test_start();				//���г�ʼҳ��
	while(1)
	{
		lv_task_handler();						//LVGL��������
		vTaskDelayUntil(&LastHandlerTime,5);	//��ʱ��С��ʱʱ��
	}
} 


