// esp32cam-cmd.ino - Sketch to get framebuffers from ESP32-CAM and download them to the host (PC)
#include "esp_camera.h" // See https://github.com/espressif/esp32-camera/tree/master/driver
#define APP_NAME    "esp32cam-cmd"
#define APP_VERSION "V1"


// === FLED ===============================================================


// The GPIO pin connected to the built-in flash LED
#define FLED2_PIN  4 // high power led
// The GPIO pin connected to the hand made low power LED
#define FLED1_PIN 13 // low power led


void fled_on(int led) {
  if( led==1 ) digitalWrite(FLED1_PIN, HIGH);
  if( led==2 ) digitalWrite(FLED2_PIN, HIGH);
}


void fled_off(int led) {
  if( led==1 ) digitalWrite(FLED1_PIN, LOW);
  if( led==2 ) digitalWrite(FLED2_PIN, LOW);
}


void fled_setup() {
  fled_off(1);
  fled_off(2);
  pinMode(FLED1_PIN, OUTPUT);
  pinMode(FLED2_PIN, OUTPUT);
  Serial.printf("fled: init success\n"); 
}


// === CAM =========================================================================


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
#define CAM_INFRAMESIZE   FRAMESIZE_QVGA
#define CAM_PIXFORMAT     PIXFORMAT_GRAYSCALE
#define CAM_INFRAMEWIDTH  320 // =16*4*5
#define CAM_INFRAMEHEIGHT 240 // =16*3*5


// Configure the camera and return success
esp_err_t cam_setup() {
  // Overwrite some configuration entries (for rest see commodel.h)
  cammodel_config.frame_size = CAM_INFRAMESIZE;
  cammodel_config.pixel_format = CAM_PIXFORMAT;
  // Configure the camera on the board
  esp_err_t err = esp_camera_init(&cammodel_config);
  // Check for success
  if( err==ESP_OK ) 
    Serial.printf("cam : init success\n"); 
  else 
    Serial.printf("cam : init failed (%d)\n",err); 
  // Return results
  return err;
}


// Capture image, down-sampling, and print to Serial
// Uses led `led` for flash (0=none, 1=fled1, 2=fled2)
esp_err_t cam_capture(int led) {
  // Capture with LED
  fled_on(led);
    delay(50);
    camera_fb_t *fb = esp_camera_fb_get();
    delay(50);
  fled_off(led);

  if( !fb ) {
    Serial.printf("cam : fb get failed\n"); 
    return ESP_FAIL;
  }

  // These are "assert", you may leave them out
  if( fb->width!=CAM_INFRAMEWIDTH ) {
    Serial.printf("cam : mismatch in configured and actual frame width\n"); 
    return ESP_FAIL;
  }
  if( fb->height!=CAM_INFRAMEHEIGHT ) {
    Serial.printf("cam : mismatch in configured and actual frame width\n"); 
    return ESP_FAIL;
  }
  if( fb->format!=CAM_PIXFORMAT ) {
    Serial.printf("cam : mismatch in configured and actual frame width\n"); 
    return ESP_FAIL;
  }

  // Print to serial (with row counter and checksum)
  for( int y=0; y<CAM_INFRAMEHEIGHT; y++ ) {
    Serial.printf("%04x:",y);
    uint16_t csum = 0; 
    for( int x=0; x<CAM_INFRAMEWIDTH; x++ ) {
      uint8_t pixel = fb->buf[x+CAM_INFRAMEWIDTH*y];
      csum += pixel;
      Serial.printf("%02x"); 
    }
    Serial.printf(":%04x\n",csum); 
  }

  return ESP_OK;
}


// === TIME ===============================================================


void time_setup() {  
  Serial.printf("time: init success\n"); 
}


uint32_t time_seconds() {
  #define SECS 1000
  static uint32_t secs = 0;
  static uint32_t prev = 0;
  uint32_t cur = millis();
  while( cur-prev>=SECS ) { prev+=SECS; secs++; }
  return secs;
}


char time_buf[] = "#####d-##:##:##";
char * time_string() {
  uint32_t secs= time_seconds();
  uint32_t mins=secs/60; secs=secs%60;
  uint32_t hours=mins/60; mins=mins%60;
  uint32_t days=hours/24; hours=hours%60;
  sprintf(time_buf,"%ud-%02d:%02d:%02d",days,hours,mins,secs);
  return time_buf;
}

// === APP ===============================================================


uint32_t frame = 0;
void capture_print_frame(int led) {
  Serial.printf("app : frame %u flash %d time %s \n", frame++, led, time_string());
  Serial.printf("[[\n");
  cam_capture(led);
  Serial.printf("]]\n" );            
}


#define APP_PROMPT ">> "
void setup() {
  Serial.begin(115200);
  Serial.printf("\n\n\n\n");
  Serial.printf("app : welcome to " APP_NAME "\n");
  Serial.printf("app : version " APP_VERSION "\n");
  time_setup();
  fled_setup();
  cam_setup();
  for( int i=0; i<5; i++) {fled_on(2); delay(1); fled_off(2); delay(50); }
  Serial.printf("\napp : type 'h' for help\n" APP_PROMPT);
}


void loop() {
  if( Serial.available()>0 ) {
    char ch = Serial.read();
    if( ch==10 || ch==13 ) {
      Serial.printf("\n" APP_PROMPT);
    } else {
      Serial.printf("%c\n", ch);
      if( ch=='0' || ch=='1' || ch=='2' ) {
        capture_print_frame(ch-'0');
      } else if( ch=='t' ) {
        Serial.printf("app : time since power up %s\n", time_string());
      } else if( ch=='v' ) {
        Serial.printf("app : " APP_NAME " " APP_VERSION "\n");
      } else if( ch=='i' ) {
        Serial.printf("app : frame %dx%d pixels, monochrome 8 bit\n", CAM_INFRAMEWIDTH, CAM_INFRAMEHEIGHT);
        Serial.printf("app : capture returns '[[ <line>+ ]]' (takes 15sec) where\n");
        Serial.printf("app : <line>='<rrrr>:<pp><pp><pp>...:<ssss>' where\n");
        Serial.printf("app : <rrrr> is row number in 4 hex digits\n");
        Serial.printf("app : <pp> is pixel value in 2 hex digits\n");
        Serial.printf("app : <ssss> is sum of all <pp>s in 4 hex digits\n");
      } else if( ch=='h' ) {
        Serial.printf("app : to capture '0' (no LED), '1' (low LED), or '2' (high LED)\n");
        Serial.printf("app : 't' for time\n");
        Serial.printf("app : 'v' for version\n");
        Serial.printf("app : 'i' for frame info\n");
        Serial.printf("app : 'h' for this help\n");
      } else {
        Serial.printf("app : unknown command (char '%c'/0x%02x)\n",ch,ch);            
      }
      Serial.printf(APP_PROMPT);
      // Remove a CR or LF after a command
      while( ch=Serial.peek(), ch==10 || ch==13 ) ch=Serial.read();
    }
  }
}
