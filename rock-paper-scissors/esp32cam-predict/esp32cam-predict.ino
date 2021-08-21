// esp32cam-predict.ino - sketch used to predict rock/paper/scissors using ML on camera input 
#include <float.h>           // For FLT_MAX
#include <EloquentTinyML.h>  // TensorFlow lite for ESP32, from https://github.com/eloquentarduino/EloquentTinyML
#include "rps32model.h"      // The Rock/paper/scissors model from https://github.com/maarten-pennings/MachineLearning/tree/main/rock-paper-scissors
#include "esp_camera.h"      // The camera driver,see https://github.com/espressif/esp32-camera/tree/master/driver
#define  CAMMODEL_AI_THINKER // Define the correct board so that "cammodel.h" sets correct macros (pin defs etc)
#include "cammodel.h"        // Make sure you define your camera model before the #include

#define APP_NAME    "esp32cam-predict"
#define APP_VERSION "V1"


// What region to capture =======================================================

// Region of Interest (see below, inframe is 320x240, but ascii-out is rotated 90 CW)
#define APP_CROP_X0       122 // top of ascii-out
#define APP_CROP_Y0       36  // left of ascii-out
#define APP_CROP_X1       234 // bottom of ascii-out
#define APP_CROP_Y1       220 // right of ascii-out

// Subsampling ratio (an input block of APP_BLOCKSIZE*APP_BLOCKSIZE becomes one output block)
#define APP_BLOCKSIZE     4


// Flash LED driver =============================================================

// Flash LED settings
#define FLED_PIN         4    // The GPIO pin for the high-power LED.
#define FLED_CHANNEL     15   // I just picked first PWM channel (of the 16).
#define FLED_FREQUENCY   4096 // Some arbirary PWM frequemcy, high enough to not see it.
#define FLED_RESOLUTION  8    // 8 bit resolution for the duty-cycle.

// Configure pins for the flash LED
int fled_setup() {
  ledcSetup(FLED_CHANNEL, FLED_FREQUENCY, FLED_RESOLUTION); // Setup a PWM channel
  ledcAttachPin(FLED_PIN, FLED_CHANNEL); // Attach the PWM channel to the LED pin
  ledcWrite(FLED_CHANNEL, 0); // Set duty cycle of the PWM channel to 0 (off)
  Serial.printf("fled: setup success\n"); 
  return 1;
}

// Set flash LED brightness to `duty` (0..100).
void fled_set(int duty) {
  if( duty<0 ) duty= 0;
  if( duty>100 ) duty= 100;
  duty= duty * ((1<<FLED_RESOLUTION)-1) / 100;
  ledcWrite(FLED_CHANNEL, duty); 
}


// Image processing ================================================================

// Histogram equalization (https://en.wikipedia.org/wiki/Histogram_equalization)
void img_histeq(uint8_t * img, int imgsize) {
  #define COLS 256       // Number of colors
  static int bins[COLS]; // Histogram bins 
  
  // Histogram bins cleared to 0
  for( int i = 0; i<COLS; i++ ) bins[i]=0;
  // Histogram bins count pixel data from image
  for( int i = 0; i<imgsize; i++ ) bins[ img[i] ]+=1;
  // Cumulate histogram bins
  for( int i = 1; i<COLS; i++ ) bins[i]+=bins[i-1];
  // Find smallest non-zero bin
  int binmin=0;
  for( int i = 0; i<COLS; i++ ) if( bins[i]>0 ) { binmin=bins[i]; break; }
  // Equalize (+0.5 is for rounding)
  for( int i = 0; i<imgsize; i++ ) img[i] = (bins[img[i]]-binmin) * 255.0 / (bins[COLS-1]-binmin) + 0.5;
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

// Configure the camera and return 1 if that was successful, 0 otherwise
int cam_setup() {
  // Overwrite some configuration entries (for rest see commodel.h)
  cammodel_config.frame_size = CAM_INFRAMESIZE;
  cammodel_config.pixel_format = CAM_PIXFORMAT;
  // Configure the camera on the board
  esp_err_t err = esp_camera_init(&cammodel_config);
  // Check for success
  if( err==ESP_OK ) 
    Serial.printf("cam : setup success\n"); 
  else 
    Serial.printf("cam : setup FAIL (%d)\n",err); 
  // Return results
  return err==ESP_OK;
}

// Capture image, subsampling, equalize, and save in cam_outframe[CAM_OUTFRAMEHEIGHT*CAM_OUTFRAMEWIDTH]
esp_err_t cam_capture(int histeq) {
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

  // Histogram equalization
  if( histeq ) {
    img_histeq(cam_outframe, CAM_OUTFRAMEHEIGHT*CAM_OUTFRAMEWIDTH ); 
  }
  
  return ESP_OK;
}


// Print ascii-out to serial, i.e. cam_outframe[CAM_OUTFRAMEHEIGHT*CAM_OUTFRAMEWIDTH]
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
    Serial.printf(" |");
    // next print ascii impression
    for( int y=CAM_OUTFRAMEHEIGHT-1; y>=0; y-- ) {
      Serial.printf("%c",level[cam_outframe[x+CAM_OUTFRAMEWIDTH*y]/32]);
    }
    Serial.printf("|\n");
  }
}


// TensorFlow Lite micro ===================================================

// I made the decision that the cam_printframe() prints the pixels to Serial in the "natural order": the 
// frame is printed how I see the scene. The camera was physically rotated with respect to the scene and 
// top and bottom were mirrored, so the printout order needed some tweaking - see comments in cam_printframe().
// This is the order
//         y=45 y=44 y=43 ... y=2 y=1 y=0
//    x=0  
//    x=1  
//    ...
//    x=27
//
// Since the Neural Net was trained on the images as they were printed, we need to convert the 
// cam_outframe[] order to the tflu_frame[] order. We do that in the tflu_norm() function that 
// already does the normalization 0..255 -> -1.0..+1.0

#define TFLU_HEIGHT 28
#define TFLU_WIDTH  46

// The output frame buffer
float tflu_frame[TFLU_HEIGHT][TFLU_WIDTH];

// Converts  cam_outframe[] 0..255 unit8_t  to  tflu_frame[] -1.0..+1.0 float
// Also does the buffer reorientation.
void tflu_norm() {
    // Convert to float
    for( int y=0; y<TFLU_HEIGHT; y++ ) {
        for( int x=0; x<TFLU_WIDTH; x++ ) {
            // Buffer Reorientation
            int xx = y;
            int yy = TFLU_WIDTH-1 - x;
            tflu_frame[y][x] = 2.0 * cam_outframe[xx+CAM_OUTFRAMEWIDTH*yy] / 255.0 - 1.0;
        }
    }
}

// From an array of 4 floats returns the index of the greatest.
int tflu_ixmax( float out[4] ) {
    float max = -FLT_MAX;
    int   ix;
    for( int i=0; i<4; i++ )
        if( out[i]> max ) {
          ix = i;
          max = out[i];
        }
     return ix;
}

#define NUMBER_OF_INPUTS  (TFLU_HEIGHT*TFLU_WIDTH)
#define NUMBER_OF_OUTPUTS 4
#define TENSOR_ARENA_SIZE (90*1024) 

Eloquent::TinyML::TfLite<
    NUMBER_OF_INPUTS, 
    NUMBER_OF_OUTPUTS, 
    TENSOR_ARENA_SIZE
> tflu;

// returns 1 for success, 0 for fail
int tflu_setup() {
    if( CAM_OUTFRAMEHEIGHT!=TFLU_WIDTH || CAM_OUTFRAMEWIDTH!=TFLU_HEIGHT) {
        Serial.printf("tflu : setup failed (buffer size mismatch camera versus tensorflow)\n"); 
        return 0;
    }

    int res = tflu.begin(rps32model_data); 
    if( res ) 
        Serial.printf("tflu : setup success\n"); 
    else 
        Serial.printf("tflu : setup FAIL\n"); 
      
    return res;
}

// Category names that match rps32model
const char * tflu_categories[] = {"none","paper","rock","scissors"};

void tflu_big(int cat, int ledison ) {
    if( cat==0 ) {
    } else if( cat==1 ) {
        Serial.printf("\n");    
        Serial.printf("########     ####    #######    ########  ######## \n");
        Serial.printf("#########   ######   #########  ########  #########\n");
        Serial.printf("##     ##  ##    ##  ##     ##  ##        ##     ##\n");
        Serial.printf("#########  ########  #########  ####      ######## \n");
        Serial.printf("########   ########  ########   ####      #######  \n");
        Serial.printf("##         ##    ##  ##         ##        ##   ##  \n");
        Serial.printf("##         ##    ##  ##         ########  ##    ## \n");
        Serial.printf("##         ##    ##  ##         ########  ##     ##\n");
        Serial.printf("\n");    
        fled_set(ledison?0:100); delay(150); fled_set(ledison?100:0); delay(150); 
        fled_set(ledison?0:100); delay(150); fled_set(ledison?100:0); delay(150); 
        delay(1000);
    } else if( cat==2 ) {
        Serial.printf("\n");    
        Serial.printf("########     ####     ######   ##   ##\n");
        Serial.printf("#########   ######   ########  ##  ## \n");
        Serial.printf("##     ##  ##    ##  ##    ##  ## ##  \n");
        Serial.printf("########   ##    ##  ##        ####   \n");
        Serial.printf("#######    ##    ##  ##        ####   \n");
        Serial.printf("##   ##    ##    ##  ##    ##  ## ##  \n");
        Serial.printf("##    ##    ######   ########  ##  ## \n");
        Serial.printf("##     ##    ####     ######   ##   ##\n");
        Serial.printf("\n");    
        fled_set(ledison?0:100); delay(150); fled_set(ledison?100:0); delay(150); 
        delay(1000);
    } else if( cat==3 ) {
        Serial.printf("\n");    
        Serial.printf(" ######    ######    #####   ######    ######     ####    ########    ###### \n");    
        Serial.printf("########  ########   #####  ########  ########   ######   #########  ########\n");  
        Serial.printf("##        ##    ##    ##    ##        ##        ##    ##  ##     ##  ##      \n");  
        Serial.printf("#######   ##          ##    #######   #######   ##    ##  ########   ####### \n");  
        Serial.printf(" #######  ##          ##     #######   #######  ##    ##  #######     #######\n");  
        Serial.printf("      ##  ##    ##    ##          ##        ##  ##    ##  ##   ##          ##\n");  
        Serial.printf("########  ########    ##    ########  ########   ######   ##    ##   ########\n");  
        Serial.printf(" ######    ######    ####    ######    ######     ####    ##     ##   ###### \n");  
        Serial.printf("\n");    
        fled_set(ledison?0:100); delay(150); fled_set(ledison?100:0); delay(150); 
        fled_set(ledison?0:100); delay(150); fled_set(ledison?100:0); delay(150); 
        fled_set(ledison?0:100); delay(150); fled_set(ledison?100:0); delay(150); 
        delay(1000);
    } else {
        Serial.printf("Error\n");    
    }
}

void tflu_predict( int big, int ledison ) {
    float output[NUMBER_OF_OUTPUTS];
    uint32_t time0 = micros();
    tflu.predict((float*)tflu_frame,output);
    uint32_t time1 = micros();
    const char * cat = tflu_categories[ tflu_ixmax(output) ];
    if( big ) {
        tflu_big( tflu_ixmax(output), ledison );
    } else {
        Serial.printf("cat=%-8s n=%0.4f p=%0.4f r=%0.4f s=%0.4f time=%uus\n", cat, output[0],output[1],output[2],output[3], time1-time0);
    }
}


// Application =============================================================


int app_fled   = 5; // max flash led
int app_histeq = 1; // histogram equalization enabled
int app_count  = 0; // number of captures
int app_tflu   = 2; // predict using linked-in TensorFlow light model
int app_print  = 0; // show camera output
int app_auto   = 1; // Automatic shooting

void app_prompt() {
  Serial.printf("%d>> ",app_count);
}

void setup() {
  Serial.begin(115200);
  while( !Serial ) delay(100);
  Serial.printf("\n\n\n\n");
  Serial.printf("app : welcome to " APP_NAME "\n");
  Serial.printf("app : version " APP_VERSION "\n");
  int ok = 1;
  ok &= fled_setup();
  ok &= cam_setup();
  ok &= tflu_setup();
  if( ! ok ) { Serial.printf("Serious error HALTING\n"); while(1); }
  Serial.printf("\napp : type 'h' for help\n");
  app_prompt();
}

void loop() {
  if( app_auto ) {
    fled_set(app_fled*20);
  }
  if( app_auto || (Serial.available()>0) ) {
    char ch = Serial.read();
    if( app_auto && (ch==-1) ) ch=' ';
    if( ch>='0' && ch<='5'  ) {
      int old= app_fled;
      app_fled= ch-'0';
      Serial.printf("app : flash level %d -> %d\n",old,app_fled);
    } else if( ch=='-' || ch=='+' ) {
      int old= app_histeq;
      app_histeq= ch=='+';
      Serial.printf("app : histogram equalization %d -> %d\n",old,app_histeq);
    } else if( ch=='t' ) {
      int old= app_tflu;
      app_tflu= (app_tflu+1)%3;
      Serial.printf("app : prediction %d -> %d\n",old,app_tflu);
    } else if( ch=='p' ) {
      int old= app_print;
      app_print= ! app_print;
      Serial.printf("app : printing camera frame %d -> %d\n",old,app_print);
    } else if( ch=='a' ) {
      int old= app_auto;
      app_auto= ! app_auto;
      Serial.printf("app :automatic shooting %d -> %d\n",old,app_auto);
    } else if( ch=='v' ) {
      Serial.printf("app : name " APP_NAME "\n");
      Serial.printf("app : version " APP_VERSION "\n");
    } else if( ch=='i' ) {
      Serial.printf("app : captured frame        : %d×%d pixels (8 bit mono)\n", CAM_INFRAMEWIDTH, CAM_INFRAMEHEIGHT);
      Serial.printf("app : region of interest    : %d×%d - %d×%d\n", APP_CROP_X0,APP_CROP_Y0,APP_CROP_X1,APP_CROP_Y1);
      Serial.printf("app : pre-subsampling       : %dx%d\n", APP_CROP_X1-APP_CROP_X0,APP_CROP_Y1-APP_CROP_Y0);
      Serial.printf("app : subsampling           : %d times\n", APP_BLOCKSIZE);
      Serial.printf("app : histogram equalization: %s\n", app_histeq ? "yes" : "no");
      Serial.printf("app : post-subsampling      : %d×%d pixels, print=%s\n", CAM_OUTFRAMEWIDTH, CAM_OUTFRAMEHEIGHT, (app_print?"yes":"no"));
      Serial.printf("app : prediction            : %s (model size %d, frame %d×%d)\n", (app_tflu==0?"no":(app_tflu==1?"detailed":"big")), RPS32MODEL_LEN, TFLU_WIDTH, TFLU_HEIGHT);
    } else if( ch=='h' ) {
      Serial.printf("app : '0'..'5' to set the flash level\n");
      Serial.printf("app : '-'/'+' to disable/enable histogram equalization\n");
      Serial.printf("app : 't' toggle prediction\n");
      Serial.printf("app : 'p' toggle printing camera frame\n");
      Serial.printf("app : 'a' toggle automatic shooting\n");
      Serial.printf("app : 'v' for version\n");
      Serial.printf("app : 'i' for frame info\n");
      Serial.printf("app : 'c', ' ', or an empty command, to capture an image\n");
      Serial.printf("app : 'h' for this help\n");
    } else if( ch=='c' || ch==' ' || ch==10 || ch==13 || app_auto ) {
      if( (!app_auto) && (app_fled>0) ) fled_set(app_fled*20);
          cam_capture(app_histeq); // saved in cam_outframe[CAM_OUTFRAMEHEIGHT*CAM_OUTFRAMEWIDTH]
      if( (!app_auto) && (app_fled>0) ) fled_set(0);
      if( app_print ) {
          cam_printframe();
      }
      if( app_tflu>0 ) {
          tflu_norm();
          tflu_predict(app_tflu==2,app_auto);
      }
      app_count++;
    } else {
      Serial.printf("app : unknown command (char '%c'/0x%02x)\n",ch,ch);            
    }
    app_prompt();
    // Remove a CR or LF after a command
    while( ch=Serial.peek(), ch==10 || ch==13 ) ch=Serial.read();
  }
}
