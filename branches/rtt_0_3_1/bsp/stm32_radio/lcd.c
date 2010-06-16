#include "stm32f10x.h"
#include "rtthread.h"
#include "board.h"

#include <rtgui/rtgui.h>
#include <rtgui/driver.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>

#if (LCD_VERSION == 1)
#include "fmt0371/FMT0371.h"
#endif

#if (LCD_VERSION == 2)
#include "ili_lcd_general.h"
#endif

void lcd_backlight_init(void);
void brightness_set(unsigned int value);

rt_err_t rt_hw_lcd_init(void);
void rt_hw_lcd_update(rtgui_rect_t *rect);
rt_uint8_t * rt_hw_lcd_get_framebuffer(void);
void rt_hw_lcd_set_pixel(rtgui_color_t *c, rt_base_t x, rt_base_t y);
void rt_hw_lcd_get_pixel(rtgui_color_t *c, rt_base_t x, rt_base_t y);
void rt_hw_lcd_draw_hline(rtgui_color_t *c, rt_base_t x1, rt_base_t x2, rt_base_t y);
void rt_hw_lcd_draw_vline(rtgui_color_t *c, rt_base_t x, rt_base_t y1, rt_base_t y2);
void rt_hw_lcd_draw_raw_hline(rt_uint8_t *pixels, rt_base_t x1, rt_base_t x2, rt_base_t y);

struct rtgui_graphic_driver _rtgui_lcd_driver =
{
    "lcd",
    2,
    240,
    320,
    rt_hw_lcd_update,
    rt_hw_lcd_get_framebuffer,
    rt_hw_lcd_set_pixel,
    rt_hw_lcd_get_pixel,
    rt_hw_lcd_draw_hline,
    rt_hw_lcd_draw_vline,
    rt_hw_lcd_draw_raw_hline
};

extern  void   info_init(void);
extern  void   player_init(void);
void radio_rtgui_init(void)
{
    rtgui_rect_t rect;

    rtgui_system_server_init();

    /* register dock panel */
    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = 240;
    rect.y2 = 25;
    rtgui_panel_register("info", &rect);

    /* register main panel */
    rect.x1 = 0;
    rect.y1 = 25;
    rect.x2 = 240;
    rect.y2 = 320;
    rtgui_panel_register("main", &rect);
    rtgui_panel_set_default_focused("main");

    rt_hw_lcd_init();

    info_init();
    player_init();
}

#if (LCD_VERSION == 1)
void rt_hw_lcd_update(rtgui_rect_t *rect)
{
    /* nothing for none-DMA mode driver */
}

rt_uint8_t * rt_hw_lcd_get_framebuffer(void)
{
    return RT_NULL; /* no framebuffer driver */
}

void rt_hw_lcd_set_pixel(rtgui_color_t *c, rt_base_t x, rt_base_t y)
{
    unsigned short p;

    /* get color pixel */
    p = rtgui_color_to_565p(*c);

    /* set X point */
    LCD_ADDR = 0x02;
    LCD_DATA = x;

    /* set Y point */
    LCD_ADDR = 0x03;
    LCD_DATA16(y);

    /* write pixel */
    LCD_ADDR = 0x0E;
    LCD_DATA16(p);
}

void rt_hw_lcd_get_pixel(rtgui_color_t *c, rt_base_t x, rt_base_t y)
{
    /* set X point */
    LCD_ADDR = 0x02;
    LCD_DATA = x;

    /* set Y point */
    LCD_ADDR = 0x03;
    LCD_DATA16( y );

    /* read pixel */
    LCD_ADDR = 0x0F;
    /* dummy read */
    x = LCD_DATA;

    *c = rtgui_color_from_565p( LCD_DATA16_READ() );
}

void rt_hw_lcd_draw_hline(rtgui_color_t *c, rt_base_t x1, rt_base_t x2, rt_base_t y)
{
    unsigned short p;

    /* get color pixel */
    p = rtgui_color_to_565p(*c);

    /* set X point */
    LCD_ADDR = 0x02;
    LCD_DATA = x1;

    /* set Y point */
    LCD_ADDR = 0x03;
    LCD_DATA16( y );

    /* write pixel */
    LCD_ADDR = 0x0E;
    while (x1 < x2)
    {
        LCD_DATA16(p);
        x1 ++;
    }
}

void rt_hw_lcd_draw_vline(rtgui_color_t *c, rt_base_t x, rt_base_t y1, rt_base_t y2)
{
    unsigned short p;

    /* get color pixel */
    p = rtgui_color_to_565p(*c);

    /* set X point */
    LCD_ADDR = 0x02;
    LCD_DATA = x;

    while (y1 < y2)
    {
        /* set Y point */
        LCD_ADDR = 0x03;
        LCD_DATA16( y1 );

        /* write pixel */
        LCD_ADDR = 0x0E;
        LCD_DATA16(p);

        y1 ++;
    }
}

void rt_hw_lcd_draw_raw_hline(rt_uint8_t *pixels, rt_base_t x1, rt_base_t x2, rt_base_t y)
{
    rt_uint16_t *ptr;

    /* get pixel */
    ptr = (rt_uint16_t*) pixels;

    /* set X point */
    LCD_ADDR = 0x02;
    LCD_DATA = x1;

    /* set Y point */
    LCD_ADDR = 0x03;
    LCD_DATA16( y );

    /* write pixel */
    LCD_ADDR = 0x0E;
    while (x1 < x2)
    {
        LCD_DATA16(*ptr);
        x1 ++;
        ptr ++;
    }
}

rt_err_t rt_hw_lcd_init(void)
{
    lcd_backlight_init();
    ftm0371_port_init();
    ftm0371_init();

    //LCD GRAM test
    {
        unsigned int test_x;
        unsigned int test_y;
        unsigned short temp;

        rt_kprintf("\r\nLCD GRAM test....");

        //write
        temp = 0;
        for( test_y=0; test_y<320; test_y++)
        {
            /* set X point */
            LCD_ADDR = 0x02;
            LCD_DATA = 0;

            /* set Y point */
            LCD_ADDR = 0x03;
            LCD_DATA16( test_y );

            /* write pixel */
            LCD_ADDR = 0x0E;
            for(test_x=0; test_x<240; test_x++)
            {
                LCD_DATA16(temp++);
            }
        }

        temp = 0;
        for( test_y=0; test_y<320; test_y++)
        {
            /* set X point */
            LCD_ADDR = 0x02;
            LCD_DATA = 0;

            /* set Y point */
            LCD_ADDR = 0x03;
            LCD_DATA16( test_y );

            /* write pixel */
            LCD_ADDR = 0x0f;
            /* dummy read */
            test_x = LCD_DATA;
            for(test_x=0; test_x<240; test_x++)
            {
                if ( LCD_DATA16_READ() != temp++)
                {
                    rt_kprintf("  LCD GRAM ERR!!");
                    while(1);
                }
            }
        }
        rt_kprintf("  TEST PASS!\r\n");
    }//LCD GRAM TEST

#ifndef DRIVER_TEST
    /* add lcd driver into graphic driver */
    rtgui_graphic_driver_add(&_rtgui_lcd_driver);
#endif

    return RT_EOK;
}

#include <finsh.h>

void hline(rt_base_t x1, rt_base_t x2, rt_base_t y, rt_uint32_t pixel)
{
    rt_hw_lcd_draw_hline(&pixel, x1, x2, y);
}
FINSH_FUNCTION_EXPORT(hline, draw a hline);

void vline(int x, int y1, int y2, rt_uint32_t pixel)
{
    rt_hw_lcd_draw_vline(&pixel, x, y1, y2);
}
FINSH_FUNCTION_EXPORT(vline, draw a vline);

void cls()
{
    rt_size_t index;
    rtgui_color_t white 	= RTGUI_RGB(0xff, 0xff, 0xff);

    for (index = 0; index < 320; index ++)
        rt_hw_lcd_draw_hline(&white, 0, 240, index);
}
FINSH_FUNCTION_EXPORT(cls, clear screen);
#endif

#if (LCD_VERSION == 2)
#include <finsh.h>

rt_err_t rt_hw_lcd_init(void)
{
    lcd_backlight_init();
    lcd_Initializtion();

#ifndef DRIVER_TEST
    /* add lcd driver into graphic driver */
    rtgui_graphic_driver_add(&_rtgui_lcd_driver);
#endif

    return RT_EOK;
}
#endif

#if LCD_USE_PWM
static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static TIM_OCInitTypeDef  TIM_OCInitStructure;
#define ARR  12000
#endif

void lcd_backlight_init(void)
{
    /* for old version */
    GPIO_InitTypeDef GPIO_InitStructure;
#if !LCD_USE_PWM
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOF,&GPIO_InitStructure);
    GPIO_SetBits(GPIOF,GPIO_Pin_9);
    /* for old version */
#else
    /* new version: use pwm in PB8 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    /* TIM4 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* GPIOB Configuration:TIM4 Channel4 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_9;//GPIO_Pin_8 |
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = ARR;//12000
    TIM_TimeBaseStructure.TIM_Prescaler = 2;//预分频2,频率36M
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    /* PWM1 Mode configuration: Channel4 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = (ARR/100)*50; /* 设置初始背光亮度 */
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);

    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);
#endif
}

void brightness_set(unsigned int value)
{
#if !LCD_USE_PWM
    if(value)
    {
        GPIO_SetBits(GPIOF,GPIO_Pin_9);
    }
    else
    {
        GPIO_ResetBits(GPIOF,GPIO_Pin_9);
    }
#else
    if(value>100)value=50;
    TIM_OCInitStructure.TIM_Pulse = (ARR/100)*value;
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);
#endif
}
FINSH_FUNCTION_EXPORT(brightness_set, set lcd_brightness 0~100);
