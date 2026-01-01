/*
 * ************************************************
 * 
 *              STM32 blink gcc demo
 * 
 *  CPU: STM32F103C8
 *  PIN: PA1
 * 
 * ************************************************
*/

#include "stm32f10x.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "sgl.h"
#include "tft.h"

#define LED_PERIPH RCC_APB2Periph_GPIOC
#define LED_PORT GPIOC
#define LED_PIN GPIO_Pin_13


#define  PANEL_WIDTH  240
#define  PANEL_HEIGHT 240


sgl_color_t panel_buffer[PANEL_WIDTH * 10];


void USART1_GPIO_Config(void) {  
    GPIO_InitTypeDef GPIO_InitStructure;  
  
    // 使能GPIOA和USART1时钟  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);  
  
    // USART1_TX   GPIOA.9  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
    // USART1_RX   GPIOA.10  
    // 注意：对于发送，我们不需要配置USART1_RX的GPIO  
    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入  
    // GPIO_Init(GPIOA, &GPIO_InitStructure);  
}

// USART1 配置  
void USART1_Config(void) {  
    USART_InitTypeDef USART_InitStructure;  
    // USART1参数初始化  
    USART_InitStructure.USART_BaudRate = 115200; // 波特率115200
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 数据位8  
    USART_InitStructure.USART_StopBits = USART_StopBits_1; // 停止位1  
    USART_InitStructure.USART_Parity = USART_Parity_No; // 无校验位  
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控制  
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 收发模式  
  
    // 初始化USART1  
    USART_Init(USART1, &USART_InitStructure);  
  
    // 使能USART1接收和发送中断  
    // 注意：如果你不使用中断，可以注释掉这两行  
    // USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  
    // USART_ITConfig(USART1, USART_IT_TXE, ENABLE);  
  
    // 使能USART1  
    USART_Cmd(USART1, ENABLE);  
}  
 
void UART1_SendString(const char *str) {
    while (*str != '\0') {
        // 等待发送寄存器为空
        while ((USART1->SR & USART_SR_TXE) == 0);
        // 写入字符到发送寄存器
        USART1->DR = (uint8_t)(*str++);
    }
    // 等待最后一个字符发送完成
    while ((USART1->SR & USART_SR_TC) == 0);
}


void delay(int x)
{
    for (int i = 0; i < x; i++)
    {
        for (int j = 0; j < 1000; j++)
            __NOP();
    }
}


#include <stdio.h>


#include "stm32f10x.h"                  // 标准外设库头文件
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

/*#################################相关宏定义，需用户修改######################################*/
#define    TIM2_TIME_PSC           720     //定义预分频值为7200，即时钟为72000000/7200=10000即10kHz
#define    TIM2_TIME_ARR           200     //定义计数值为1000，即定时时间为1000/10000=0.1s

void TIM2_Init(void)
{

   RCC->APB1ENR|=(1<<0);       //¿ªÆôTIM2Ê±ÖÓ
	
   NVIC_SetPriorityGrouping(2);      //ÉèÖÃÖÐ¶ÏÓÅÏÈ¼¶·Ö×éÎª2
   NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(2,2,3));    //ÉèÖÃÖÐ¶ÏÎª£ºÇÀÕ¼ÓÅÏÈ¼¶Îª2£¬×ÓÓÅÏÈ¼¶Îª1
   NVIC_EnableIRQ(TIM2_IRQn);       //Ê¹ÄÜTIM2ÖÐ¶Ï
  
   TIM2->PSC|=TIM2_TIME_PSC-1;          //Ô¤·ÖÆµ¼Ä´æÆ÷Ð´Èë7200
   TIM2->DIER|=(1<<0);                  //¿ªÆô¸üÐÂÖÐ¶Ï×´Ì¬
   TIM2->ARR&=0X00;                     //Çå³ý×Ô¶¯ÖØ×°¼Ä´æÆ÷µÄÖµ
   TIM2->ARR|=TIM2_TIME_ARR-1;          //×Ô¶¯ÖØ×°¼Ä´æÆ÷1000-1
   TIM2->CR1|=(1<<7);                   //½«ARR¼Ä´æÆ÷Öµ×°Èë»º³åÆ÷ÖÐ
   TIM2->CR1|=1<<0;                     //Ê¹ÄÜ¼ÆÊ±Æ÷£¬¿ªÊ¼¼ÆÊý
}


extern const unsigned char gImage_test[7080];
const sgl_pixmap_t test_pixmap = {
    .width = 60,
    .height = 59,
    .bitmap = gImage_test,
};


void TIM2_IRQHandler(void)
{
   TIM2->SR&=~(1<<0);
   sgl_tick_inc(1);
}


static void demo_anim_path(struct sgl_anim *anim, int32_t value)
{
    sgl_obj_set_pos_y(anim->data, value);
}


void demo_anim_finish(struct sgl_anim *anim)
{
    sgl_obj_delete(anim->data);
}

int main(void)
{
    GPIO_InitTypeDef gpioDef;
    RCC_APB2PeriphClockCmd(LED_PERIPH, ENABLE);
    gpioDef.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioDef.GPIO_Pin = LED_PIN;
    gpioDef.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(LED_PORT, &gpioDef);

    sgl_device_fb_t fb_dev = {
        .xres = PANEL_WIDTH,
        .yres = PANEL_HEIGHT,
        .xres_virtual = PANEL_WIDTH,
        .yres_virtual = PANEL_HEIGHT,
        .flush_area = demo_panel_flush_area,
        .buffer[0] = panel_buffer,
        .buffer_size = SGL_ARRAY_SIZE(panel_buffer),
    };

    sgl_device_fb_register(&fb_dev);
    sgl_device_log_register(UART1_SendString);

    USART1_GPIO_Config();
    USART1_Config();
    UART1_SendString("STM32F103C8T6 SGL DEMO\r\n");

    sgl_init();

    tft_init();

    TIM2_Init();

    sgl_boot_animation();

    sgl_obj_t *rect = NULL;
    uint32_t count = 0;
    sgl_anim_t *anim = 0;
    sgl_obj_t *label = NULL;


    while (1) {
        sgl_task_handle();
        count ++;
        if(count > 2000) {
            count = 0;
            int x = sgl_rand() % 240;
            rect = sgl_rect_create(NULL);
            sgl_obj_set_pos(rect, x, 0);
            sgl_obj_set_size(rect, 59, 59);
            sgl_rect_set_radius(rect, 10);
            sgl_rect_set_pixmap(rect, &test_pixmap);

            anim = sgl_anim_create();
            sgl_anim_set_data(anim, rect);
            sgl_anim_set_act_duration(anim, 1000);
            sgl_anim_set_start_value(anim, 0);
            sgl_anim_set_end_value(anim, 240);
            sgl_anim_set_path(anim, demo_anim_path, SGL_ANIM_PATH_LINEAR);
            sgl_anim_set_repeat_cnt(anim, 1);
            sgl_anim_set_finish_cb(anim, demo_anim_finish);
            sgl_anim_set_auto_free(anim);
            sgl_anim_start(anim);
        }
    }
}
