#include "lv_port_disp.h"
#include "lcd.h"


//变量定义
#define COLOR_BUF_SIZE		(LV_HOR_RES_MAX*LV_VER_RES_MAX)	//全屏的大小
static lv_color_t color_buf[COLOR_BUF_SIZE]	__attribute__((at(0X68000000))); //分配到外部1MB sram的最起始处

//函数申明
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
#if LV_USE_GPU
static void gpu_blend(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
static void gpu_fill(lv_color_t * dest, uint32_t length, lv_color_t color);
#endif



//lvgl显示接口初始化
void lv_port_disp_init(void)
{
	static lv_disp_buf_t disp_buf;
	
	//显示缓冲区初始化
	lv_disp_buf_init(&disp_buf, color_buf, NULL, COLOR_BUF_SIZE); 

	//显示驱动默认值初始化   
	lv_disp_drv_t disp_drv;                         
	lv_disp_drv_init(&disp_drv);                 

	//设置屏幕的显示大小,我这里是为了支持正点原子的多个屏幕,采用动态获取的方式
	//如果你是用于实际项目的话,可以不用设置,那么其默认值就是lv_conf.h中LV_HOR_RES_MAX和LV_VER_RES_MAX宏定义的值
	disp_drv.hor_res = lcddev.width;
	disp_drv.ver_res = lcddev.height;

	//注册显示驱动回调
	disp_drv.flush_cb = disp_flush;

	//注册显示缓冲区
	disp_drv.buffer = &disp_buf;

#if LV_USE_GPU
	//可选的,只要当使用到GPU时,才需要实现gpu_blend和gpu_fill接口

	//使用透明度混合俩个颜色数组时需要用到gpu_blend接口
	disp_drv.gpu_blend = gpu_blend;

	//用一个颜色填充一个内存数组时需要用到gpu_fill接口
	disp_drv.gpu_fill = gpu_fill;
#endif

	//注册显示驱动到lvgl中
	lv_disp_drv_register(&disp_drv);
}


//把指定区域的显示缓冲区内容写入到屏幕上,你可以使用DMA或者其他的硬件加速器在后台去完成这个操作
//但是在完成之后,你必须得调用lv_disp_flush_ready()
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
	//把指定区域的显示缓冲区内容写入到屏幕
	LCD_Color_Fill(area->x1,area->y1,area->x2,area->y2,(u16*)color_p);
	//最后必须得调用,通知lvgl库你已经flushing拷贝完成了
	lv_disp_flush_ready(disp_drv);
}




//可选的
#if LV_USE_GPU

/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/
    uint32_t i;
    for(i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_fill_cb(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color);
{
    /*It's an example code which should be done by your GPU*/
    uint32_t x, y;
    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/

    for(y = fill_area->y1; y < fill_area->y2; y++) {
        for(x = fill_area->x1; x < fill_area->x2; x++) {
            dest_buf[x] = color;
        }
        dest_buf+=dest_width;    /*Go to the next line*/
    }
}

#endif 
