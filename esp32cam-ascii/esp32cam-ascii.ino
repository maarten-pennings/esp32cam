// esp32cam-ascii.ino - sample sketch to get framebuffers from ESP32-CAM and show them in ASCII
#include "esp_camera.h" // See https://github.com/espressif/esp32-camera/tree/master/driver


// Wiring https://randomnerdtutorials.com/program-upload-code-esp32-cam/ 
//   FTDI-VCC to ESP.5V <==== NOT to 3V3!
//   FTDI-GND to ESP.GND
//   FTDI-TX  to ESP.U0R
//   FTDI-RX  to ESP.U0T
// Arduino settings
//   Tools > Board > ESP32 Wrover Module
//   Tools > Partition Scheme > Huge APP (3MB No OTA/1MB SPIFFS)
//   Tools > Port > ...
// To flash
//   On breadboard wire IO0 to GND
//   Press reset button on ESP32-CAM
//   In IDE press upload
//   On breadboard remove wire IO0-GND
//   Press reset button on ESP32-CAM
// Board schematics https://github.com/SeeedDocument/forum_doc/raw/master/reg/ESP32_CAM_V1.6.pdf


// Make sure you define your camera model before the #include
#define CAMMODEL_AI_THINKER
#include "cammodel.h"


// Define input frame size and block size (subsampling rate) for the output frames
#define APP_INFRAMESIZE   FRAMESIZE_QVGA
#define APP_PIXFORMAT     PIXFORMAT_GRAYSCALE
#define APP_INFRAMEWIDTH  320 // =16*4*5
#define APP_INFRAMEHEIGHT 240 // =16*3*5
#define APP_BLOCKSIZE     5


// Setup output frame
#if APP_INFRAMEWIDTH % APP_BLOCKSIZE != 0
  #error Input frame width is not a multiple of blocksize
#endif
#if APP_INFRAMEHEIGHT % APP_BLOCKSIZE != 0
  #error Input frame height is not a multiple of blocksize
#endif
#define APP_OUTFRAMEWIDTH  (APP_INFRAMEWIDTH  / APP_BLOCKSIZE)
#define APP_OUTFRAMEHEIGHT (APP_INFRAMEHEIGHT / APP_BLOCKSIZE)
uint8_t app_outframe[APP_OUTFRAMEHEIGHT*APP_OUTFRAMEWIDTH];


#define APP_FLASHLED_PIN 4
void app_flashled_setup() {
  pinMode(APP_FLASHLED_PIN, OUTPUT);
  Serial.printf("fled: init success)\n"); 
}

void app_flashled_on() {
  digitalWrite(APP_FLASHLED_PIN, HIGH);
}

void app_flashled_off() {
  digitalWrite(APP_FLASHLED_PIN,LOW);
}


// Configure the camera and return success
esp_err_t app_cam_setup() {
  // Overwrite some configuration entries (for rest see commodel.h)
  cammodel_config.frame_size = APP_INFRAMESIZE;
  cammodel_config.pixel_format = APP_PIXFORMAT;
  // Configure the camera on the board
  esp_err_t err = esp_camera_init(&cammodel_config);
  // Check for success
  if( err==ESP_OK ) 
    Serial.printf("cam : init success)\n"); 
  else 
    Serial.printf("cam : init failed (%d)\n",err); 
  // Return results
  return err;
}


// Capture image, down-sampling, and save in app_outframe
esp_err_t app_cam_capture(bool flashled) {
  if( flashled ) app_flashled_on();
  camera_fb_t *fb = esp_camera_fb_get();
  if( flashled ) app_flashled_off();
  if( !fb ) {
    Serial.printf("cam : fb get failed\n"); 
    return ESP_FAIL;
  }

  // These are "assert", you may leave them out
  if( fb->width!=APP_INFRAMEWIDTH ) {
    Serial.printf("cam : mismatch in configured and actual frame width\n"); 
    return ESP_FAIL;
  }
  if( fb->height!=APP_INFRAMEHEIGHT ) {
    Serial.printf("cam : mismatch in configured and actual frame width\n"); 
    return ESP_FAIL;
  }
  if( fb->format!=APP_PIXFORMAT ) {
    Serial.printf("cam : mismatch in configured and actual frame width\n"); 
    return ESP_FAIL;
  }

  // Subsample
  for( int y=0; y<APP_OUTFRAMEHEIGHT; y++ ) {
    for( int x=0; x<APP_OUTFRAMEWIDTH; x++ ) {
      int sum=0;
      for( int yy=y*APP_BLOCKSIZE; yy<(y+1)*APP_BLOCKSIZE; yy++ ) {
        for( int xx=x*APP_BLOCKSIZE; xx<(x+1)*APP_BLOCKSIZE; xx++ ) {
           sum+= fb->buf[xx+APP_INFRAMEWIDTH*yy]; 
        }
      }
      app_outframe[x+APP_OUTFRAMEWIDTH*y]= sum/(APP_BLOCKSIZE*APP_BLOCKSIZE);
    }
  }

  return ESP_OK;
}


// Print outframe to Serial in ASCII
void app_printframe(uint8_t * frame, int width, int height) {
  static const char *level="W@8Oo=- ";
  for( int y=0; y<height; y++ ) {
    Serial.printf("%3d: ",y);
    for( int x=0; x<width; x++ ) {
      // Mirror?
      int yy= y; 
      int xx= (width-1)-x;
      // Print
      Serial.printf("%c",level[frame[xx+width*yy]/32]);
    }
    Serial.printf("\n");
  }
  Serial.printf("\n");
}


uint32_t prev;
void setup() {
  Serial.begin(115200);
  Serial.printf("\nesp32cam-ascii\n");
  app_flashled_setup();
  app_cam_setup();
  prev= millis();
}


void loop() {
  app_cam_capture(false); // true for flash enable
  uint32_t span=millis()-prev; prev=span+prev;
  Serial.printf("delay=%u FPS=%f.2\n",span,(1000.1+span/2)/span);
  app_printframe(app_outframe,APP_OUTFRAMEWIDTH,APP_OUTFRAMEHEIGHT);
}
