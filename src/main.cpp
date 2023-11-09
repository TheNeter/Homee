#include <lvgl.h>
#include <TFT_eSPI.h>
#include "FT62XXTouchScreen.h"
#include <Arduino.h>
#include <Wire.h>

#include "esp_freertos_hooks.h"

TFT_eSPI tft = TFT_eSPI(); 

static const uint32_t screenWidth  = 480;
static const uint32_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

FT62XXTouchScreen touchScreen = FT62XXTouchScreen(screenWidth, PIN_SDA, PIN_SCL);


/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
   uint32_t w = ( area->x2 - area->x1 + 1 );
   uint32_t h = ( area->y2 - area->y1 + 1 );

   tft.startWrite();
   tft.setAddrWindow( area->x1, area->y1, w, h );
   tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
   tft.endWrite();

   lv_disp_flush_ready( disp );
}

uint16_t lastx = 0;
uint16_t lasty = 0;
/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{

   uint8_t offsetX = 161;

  TouchPoint touchPos = touchScreen.read();
  if (touchPos.touched) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchPos.xPos;
    data->point.y = touchPos.yPos - offsetX;
    lastx = touchPos.xPos;
    lasty = touchPos.yPos-offsetX;
  } else {
    data->state = LV_INDEV_STATE_REL;
     data->point.x = lastx;
     data->point.y = lasty;
     
  }
}
lv_obj_t * label1;
static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
      lv_label_set_text(label1, "Worked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
    }
}

static void lv_tick_task(void)
{
 lv_tick_inc(portTICK_RATE_MS);
}
void setup()
{
   Serial.begin( 115200 ); /* We all love debugging with console */

   lv_init();

   esp_err_t err = esp_register_freertos_tick_hook((esp_freertos_tick_cb_t)lv_tick_task); 
   tft.begin();
   tft.setRotation(1);

   // Enable Backlight
   pinMode(TFT_BL, OUTPUT);
   digitalWrite(TFT_BL,1);
   touchScreen.begin();



   lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

   /*Initialize the display*/
   static lv_disp_drv_t disp_drv;
   lv_disp_drv_init( &disp_drv );
   disp_drv.hor_res = screenWidth;
   disp_drv.ver_res = screenHeight;
   disp_drv.flush_cb = my_disp_flush;
   disp_drv.draw_buf = &draw_buf;
   lv_disp_drv_register( &disp_drv );


   //Init Touchpad
   static lv_indev_drv_t indev_drv;
   lv_indev_drv_init(&indev_drv);
   indev_drv.type = LV_INDEV_TYPE_POINTER;
   indev_drv.read_cb = my_touchpad_read;
   lv_indev_drv_register(&indev_drv);

  /*Do you GUI Magic*/
   lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
   lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
  lv_obj_set_width(btn1, 70);
  lv_obj_set_height(btn1, 32);
  lv_obj_set_pos(btn1, 32, 100);
  label1 = lv_label_create(btn1);
  lv_label_set_text(label1, "Hello");
}

void loop()
{
   lv_task_handler();
   delay( 1 );
}