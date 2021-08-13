// esp32cam-train.ino - sketch used to generate ML training material
#include "esp_camera.h" // See https://github.com/espressif/esp32-camera/tree/master/driver
#define CAMMODEL_AI_THINKER
#include "cammodel.h" // Make sure you define your camera model before the #include
#define APP_NAME    "esp32cam-train"
#define APP_VERSION "V1"


// What to capture =============================================================

// Region of Interest (se below, inframe is 320x240)
#define APP_CROP_X0       110
#define APP_CROP_Y0       40
#define APP_CROP_X1       230
#define APP_CROP_Y1       220

// Subsampling ratio (an input block of APP_BLOCKSIZE*APP_BLOCKSIZE becomes one output block)
#define APP_BLOCKSIZE     4



// Flash LED driver =============================================================


// Flash LED settings
#define FLED_PIN         4    // The GPIO pin for the high-power LED.
#define FLED_CHANNEL     15   // I just picked first PWM channel (of the 16).
#define FLED_FREQUENCY   4096 // Some arbirary PWM frequemcy, high enough to not see it.
#define FLED_RESOLUTION  8    // 8 bit resolution for the duty-cycle.

void fled_setup() {
  ledcSetup(FLED_CHANNEL, FLED_FREQUENCY, FLED_RESOLUTION); // Setup a PWM channel
  ledcAttachPin(FLED_PIN, FLED_CHANNEL); // Attach the PWM channel to the LED pin
  ledcWrite(FLED_CHANNEL, 0); // Set duty cycle of the PWM channel to 0 (off)
  Serial.printf("fled: setup success\n"); 
}

void fled_set(int duty) {
  if( duty<0 ) duty= 0;
  if( duty>100 ) duty= 100;
  duty= duty * ((1<<FLED_RESOLUTION)-1) / 100;
  ledcWrite(FLED_CHANNEL, duty); 
}


// Camera driver =============================================================

// Input frame size 
#define CAM_PIXFORMAT     PIXFORMAT_GRAYSCALE
#define CAM_INFRAMESIZE   FRAMESIZE_QVGA // = 320 * 240
#define CAM_INFRAMEWIDTH  320
#define CAM_INFRAMEHEIGHT 240

// These macros compute the output frame size
#define CAM_CROP_WIDTH  (APP_CROP_X1 - APP_CROP_X0)
#define CAM_CROP_HEIGHT (APP_CROP_Y1 - APP_CROP_Y0)
#if CAM_CROP_WIDTH % APP_BLOCKSIZE != 0
  #error Crop width is not a multiple of blocksize
#endif
#if CAM_CROP_HEIGHT % APP_BLOCKSIZE != 0
  #error Crop height is not a multiple of blocksize
#endif
#define CAM_OUTFRAMEWIDTH  (CAM_CROP_WIDTH  / APP_BLOCKSIZE)
#define CAM_OUTFRAMEHEIGHT (CAM_CROP_HEIGHT / APP_BLOCKSIZE)
// The output frame buffer
uint8_t cam_outframe[CAM_OUTFRAMEHEIGHT*CAM_OUTFRAMEWIDTH];

// Configure the camera and return if that was successful
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

// Capture image, down-sampling, and save in cam_outframe[CAM_OUTFRAMEHEIGHT*CAM_OUTFRAMEWIDTH]
esp_err_t cam_capture() {
  camera_fb_t *fb = esp_camera_fb_get();
  
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

  // Subsample
  for( int y=0; y<CAM_OUTFRAMEHEIGHT; y++ ) {
    for( int x=0; x<CAM_OUTFRAMEWIDTH; x++ ) {
      int sum=0;
      for( int yy=APP_CROP_Y0+y*APP_BLOCKSIZE; yy<APP_CROP_Y0+(y+1)*APP_BLOCKSIZE; yy++ ) {
        for( int xx=APP_CROP_X0+x*APP_BLOCKSIZE; xx<APP_CROP_X0+(x+1)*APP_BLOCKSIZE; xx++ ) {
          sum+= fb->buf[xx+CAM_INFRAMEWIDTH*yy]; 
        }
      }
      cam_outframe[x+CAM_OUTFRAMEWIDTH*y]= sum/(APP_BLOCKSIZE*APP_BLOCKSIZE);
    }
  }

  return ESP_OK;
}


// Print cam_outframe[CAM_OUTFRAMEHEIGHT*CAM_OUTFRAMEWIDTH] to Serial
void cam_printframe() {
  static const char *level="W@8Oo=- ";
  // The order of the for-x and for-y loop determines rotation.
  // A for loop can count up or down; this determines mirroring.
  for( int x=0; x<CAM_OUTFRAMEWIDTH; x++ ) {
    Serial.printf("%3d: ",x);
    // First print hex
    for( int y=CAM_OUTFRAMEHEIGHT-1; y>=0; y-- ) {
      Serial.printf("%02x",cam_outframe[x+CAM_OUTFRAMEWIDTH*y]);
    }
    Serial.printf(" ");
    // next print ascii impression
    for( int y=CAM_OUTFRAMEHEIGHT-1; y>=0; y-- ) {
      Serial.printf("%c",level[cam_outframe[x+CAM_OUTFRAMEWIDTH*y]/32]);
    }
    Serial.printf("\n");
  }
}


// Application =============================================================


int app_count = 0;

void app_prompt() {
  Serial.printf("%d>> ",app_count);
}

void setup() {
  Serial.begin(115200);
  while( !Serial ) delay(100);
  Serial.printf("\n\n\n\n");
  Serial.printf("app : welcome to " APP_NAME "\n");
  Serial.printf("app : version " APP_VERSION "\n");
  fled_setup();
  cam_setup();
  Serial.printf("\napp : type 'h' for help\n");
  app_prompt();
}

int app_fled = 5;

void loop() {
  if( Serial.available()>0 ) {
    char ch = Serial.read();
    Serial.printf("%c\n", ch);
    if( ch>='0' && ch<='5'  ) {
      int old= app_fled;
      app_fled= (ch-'0')*10;
      Serial.printf("app : flash level %d -> %d\n",old,app_fled);
    } else if( ch=='v' ) {
      Serial.printf("app : name " APP_NAME "\n");
      Serial.printf("app : version " APP_VERSION "\n");
    } else if( ch=='i' ) {
      Serial.printf("app : input frame %dx%d pixels, monochrome 8 bit\n", CAM_INFRAMEWIDTH, CAM_INFRAMEHEIGHT);
      Serial.printf("app : region of interest %dx%d - %dx%d\n", APP_CROP_X0,APP_CROP_Y0,APP_CROP_X1,APP_CROP_Y1);
      Serial.printf("app : subsampling %dx\n", APP_BLOCKSIZE);
      Serial.printf("app : output frame %dx%d pixels, monochrome 8 bit\n", CAM_OUTFRAMEWIDTH, CAM_OUTFRAMEHEIGHT);
    } else if( ch=='h' ) {
      Serial.printf("app : '0'..'5' to set the flash level\n");
      Serial.printf("app : 'v' for version\n");
      Serial.printf("app : 'i' for frame info\n");
      Serial.printf("app : ' ' (or an empty command) to capture an image\n");
      Serial.printf("app : 'h' for this help\n");
    } else if( ch==' ' || ch==10 || ch==13) {
      if( app_fled>0 ) fled_set(app_fled*20);
        cam_capture(); // saved in cam_outframe[CAM_OUTFRAMEHEIGHT*CAM_OUTFRAMEWIDTH]
      if( app_fled>0 ) fled_set(0);
      cam_printframe();
      app_count++;
    } else {
      Serial.printf("app : unknown command (char '%c'/0x%02x)\n",ch,ch);            
    }
    app_prompt();
    // Remove a CR or LF after a command
    while( ch=Serial.peek(), ch==10 || ch==13 ) ch=Serial.read();
  }
}
