#include "lv_port_indev.h"
#include "touch.h"



//函数申明
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);



//lvgl的输入设备初始化
void lv_port_indev_init(void)
{
    lv_indev_drv_t indev_drv;

    //lvgl支持很多种输入设备,但是我们一般常用的就是触摸屏,也就是Touchpad
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);
}



//将会被lvgl周期性调用,周期值是通过lv_conf.h中的LV_INDEV_DEF_READ_PERIOD宏来定义的
//此值不要设置的太大,否则会感觉触摸不灵敏,默认值为30ms
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
		static uint16_t last_x = 0;
		static uint16_t last_y = 0;
	
		if(tp_dev.sta&TP_PRES_DOWN)//触摸按下了
		{
			last_x = tp_dev.x[0];
			last_y = tp_dev.y[0];
			data->point.x = last_x;
			data->point.y = last_y;
			data->state = LV_INDEV_STATE_PR;
		}else{//触摸松开了
			data->point.x = last_x;
			data->point.y = last_y;
			data->state = LV_INDEV_STATE_REL;
		}
		
		//返回false代表没有缓冲的数据
    return false;
}


