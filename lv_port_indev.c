#include "lv_port_indev.h"
#include "touch.h"



//��������
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);



//lvgl�������豸��ʼ��
void lv_port_indev_init(void)
{
    lv_indev_drv_t indev_drv;

    //lvgl֧�ֺܶ��������豸,��������һ�㳣�õľ��Ǵ�����,Ҳ����Touchpad
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);
}



//���ᱻlvgl�����Ե���,����ֵ��ͨ��lv_conf.h�е�LV_INDEV_DEF_READ_PERIOD���������
//��ֵ��Ҫ���õ�̫��,�����о�����������,Ĭ��ֵΪ30ms
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
		static uint16_t last_x = 0;
		static uint16_t last_y = 0;
	
		if(tp_dev.sta&TP_PRES_DOWN)//����������
		{
			last_x = tp_dev.x[0];
			last_y = tp_dev.y[0];
			data->point.x = last_x;
			data->point.y = last_y;
			data->state = LV_INDEV_STATE_PR;
		}else{//�����ɿ���
			data->point.x = last_x;
			data->point.y = last_y;
			data->state = LV_INDEV_STATE_REL;
		}
		
		//����false����û�л��������
    return false;
}


