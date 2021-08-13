// cammodel.h - constants for connecting various camera models
// Pick one from
//   CAMMODEL_AI_THINKER
//   CAMMODEL_WROVER_KIT
//   CAMMODEL_ESP_EYE
//   CAMMODEL_M5STACK_PSRAM
//   CAMMODEL_M5STACK_WIDE


// Define the pins with wich the camera is attached.
#if defined(CAMMODEL_AI_THINKER)
// I believe this has an Omnivision OV2640, see https://www.arducam.com/ov2640/
//   1600x1200@15fps (UXGA)
//   800 x 600@30fps (SVGA)
//   352x288@60fps (CIF)

  #define CAMMODEL_PWDN     32
  #define CAMMODEL_RESET    -1
  #define CAMMODEL_XCLK      0
  #define CAMMODEL_SIOD     26
  #define CAMMODEL_SIOC     27
  
  #define CAMMODEL_Y9       35
  #define CAMMODEL_Y8       34
  #define CAMMODEL_Y7       39
  #define CAMMODEL_Y6       36
  #define CAMMODEL_Y5       21
  #define CAMMODEL_Y4       19
  #define CAMMODEL_Y3       18
  #define CAMMODEL_Y2        5
  
  #define CAMMODEL_VSYNC    25
  #define CAMMODEL_HREF     23
  #define CAMMODEL_PCLK     22

#elif defined(CAMMODEL_WROVER_KIT)

  #define CAMMODEL_PWDN    -1
  #define CAMMODEL_RESET   -1
  #define CAMMODEL_XCLK    21
  #define CAMMODEL_SIOD    26
  #define CAMMODEL_SIOC    27
  
  #define CAMMODEL_Y9      35
  #define CAMMODEL_Y8      34
  #define CAMMODEL_Y7      39
  #define CAMMODEL_Y6      36
  #define CAMMODEL_Y5      19
  #define CAMMODEL_Y4      18
  #define CAMMODEL_Y3       5
  #define CAMMODEL_Y2       4
  
  #define CAMMODEL_VSYNC   25
  #define CAMMODEL_HREF    23
  #define CAMMODEL_PCLK    22

#elif defined(CAMMODEL_ESP_EYE)

  #define CAMMODEL_PWDN    -1
  #define CAMMODEL_RESET   -1
  #define CAMMODEL_XCLK    4
  #define CAMMODEL_SIOD    18
  #define CAMMODEL_SIOC    23
  
  #define CAMMODEL_Y9      36
  #define CAMMODEL_Y8      37
  #define CAMMODEL_Y7      38
  #define CAMMODEL_Y6      39
  #define CAMMODEL_Y5      35
  #define CAMMODEL_Y4      14
  #define CAMMODEL_Y3      13
  #define CAMMODEL_Y2      34
  
  #define CAMMODEL_VSYNC   5
  #define CAMMODEL_HREF    27
  #define CAMMODEL_PCLK    25

#elif defined(CAMMODEL_M5STACK_PSRAM)

  #define CAMMODEL_PWDN     -1
  #define CAMMODEL_RESET    15
  #define CAMMODEL_XCLK     27
  #define CAMMODEL_SIOD     25
  #define CAMMODEL_SIOC     23
  
  #define CAMMODEL_Y9       19
  #define CAMMODEL_Y8       36
  #define CAMMODEL_Y7       18
  #define CAMMODEL_Y6       39
  #define CAMMODEL_Y5        5
  #define CAMMODEL_Y4       34
  #define CAMMODEL_Y3       35
  #define CAMMODEL_Y2       32
  
  #define CAMMODEL_VSYNC    22
  #define CAMMODEL_HREF     26
  #define CAMMODEL_PCLK     21

#elif defined(CAMMODEL_M5STACK_WIDE)

  #define CAMMODEL_PWDN     -1
  #define CAMMODEL_RESET    15
  #define CAMMODEL_XCLK     27
  #define CAMMODEL_SIOD     22
  #define CAMMODEL_SIOC     23
  
  #define CAMMODEL_Y9       19
  #define CAMMODEL_Y8       36
  #define CAMMODEL_Y7       18
  #define CAMMODEL_Y6       39
  #define CAMMODEL_Y5        5
  #define CAMMODEL_Y4       34
  #define CAMMODEL_Y3       35
  #define CAMMODEL_Y2       32
  
  #define CAMMODEL_VSYNC    25
  #define CAMMODEL_HREF     26
  #define CAMMODEL_PCLK     21

#else

  #error "Camera model not selected"
  
#endif


// Setup the configuration
// See https://github.com/espressif/esp32-camera/blob/master/driver/include/sensor.h for enums
#include "esp_camera.h"
static camera_config_t cammodel_config = {
  // Control pins
  .pin_pwdn       = CAMMODEL_PWDN,
  .pin_reset      = CAMMODEL_RESET,
  .pin_xclk       = CAMMODEL_XCLK,
  .pin_sscb_sda   = CAMMODEL_SIOD,
  .pin_sscb_scl   = CAMMODEL_SIOC,
  // Data pins
  .pin_d7         = CAMMODEL_Y9,
  .pin_d6         = CAMMODEL_Y8,
  .pin_d5         = CAMMODEL_Y7,
  .pin_d4         = CAMMODEL_Y6,
  .pin_d3         = CAMMODEL_Y5,
  .pin_d2         = CAMMODEL_Y4,
  .pin_d1         = CAMMODEL_Y3,
  .pin_d0         = CAMMODEL_Y2,
  // Sync pins
  .pin_vsync      = CAMMODEL_VSYNC,
  .pin_href       = CAMMODEL_HREF,
  .pin_pclk       = CAMMODEL_PCLK,
  // 20MHz or 10MHz for OV2640 double FPS (Experimental)
  .xclk_freq_hz   = 20000000,
  .ledc_timer     = LEDC_TIMER_0,
  .ledc_channel   = LEDC_CHANNEL_0,
  // Format of the pixel data: PIXFORMAT_ + RGB565|YUV422|GRAYSCALE|JPEG | RGB888|RAW|RGB444|RGB555
  // Do not use sizes above QVGA when not JPEG
  .pixel_format   = PIXFORMAT_GRAYSCALE,
  // Size of the output image: FRAMESIZE_ + QVGA   |CIF     |VGA    |SVGA   |XGA     |SXGA     |UXGA      || 96X96|QQVGA  |QCIF   |HQVGA  |240X240|HVGA   |HD
  //                                        320x240|352x288?|640x480|800x600|1024x768|1280x1024|1600x1200 || 96x96|160x120|176x144|240x176|240x240|480x320|1280x720
  .frame_size     = FRAMESIZE_QVGA,
  // Quality of JPEG output. 0-63 lower means higher quality
  .jpeg_quality   = 10,
  // Number of frame buffers to be allocated. If more than one, then each frame will be acquired (double speed).
  .fb_count       = 1
};
