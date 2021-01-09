#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//this edit code doesn't use a gzipped html source code
//the html -code is open and is easy to update or modify  on the other tab page = app_httpd.cpp

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_M5STACK_PSRAM
#define CAMERA_MODEL_AI_THINKER

const char* ssid = "RoiDesLoutres";
const char* password = "C4E6A360";


#if defined(CAMERA_MODEL_WROVER_KIT)
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    15
#define XCLK_GPIO_NUM     27
#define SIOD_GPIO_NUM     25
#define SIOC_GPIO_NUM     23

#define Y9_GPIO_NUM       19
#define Y8_GPIO_NUM       36
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       39
#define Y5_GPIO_NUM        5
#define Y4_GPIO_NUM       34
#define Y3_GPIO_NUM       35
#define Y2_GPIO_NUM       32
#define VSYNC_GPIO_NUM    22
#define HREF_GPIO_NUM     26
#define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_AI_THINKER) //zie esp32-cam schema v1.6.pdf
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#else
#error "Camera model not selected"
#endif


#define COUNT_LOW 1638
#define COUNT_HIGH 7864

#define TIMER_WIDTH 16

#include "esp32-hal-ledc.h"

//Servos PINS
int SROTATE = 12; 
int STILT = 14;      
//Servos CHANNELS
int CROTATE = 4;
int CTILT = 5;
//Servos fq
int FROTATE = 50;
int FTILT = 50;

//Servos current pos
int rpos = -1;
int tpos = -1;

//Max tilt & Rotate angle
int MAXTILT = 50;
int MAXTILT = 180;

//Margin tilt & rotate
int MARGINTILT = 10;
int MARGINROTATE = 0;

//Dynamic Servos Pos
extern int CAMTILT, CAMROTATE, CAMFAKE;

void startCameraServer();

void initChannels() {
  ledcSetup(CROTATE, FROTATE, TIMER_WIDTH);  
  ledcSetup(CTILT, FTILT, TIMER_WIDTH);  
}

void initServos() {
  ledcAttachPin(STILT, CTILT);   
  ledcAttachPin(SROTATE, CROTATE);   
}

void resServos() {
  ledcDetachPin(SROTATE);
  ledcDetachPin(STILT); 
}

void initCam() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_UXGA);
}

void initWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}


void setup() {
  pinMode(STILT, OUTPUT);
  pinMode(SROTATE, OUTPUT);
  
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  initChannels();
  initCam(); 
  initWifi();
  initServos();
  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect , de stream zit op een andere poortkanaal 9601 ");
  Serial.print("stream Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println(":9601/stream ");
  Serial.print("image Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("/capture ");
}

void loop() {
  doServo();
  delay(500);
}

void doServo() {
   Serial.print("CAMROTATE : ");Serial.print(CAMROTATE);Serial.print("CAMTILT : ");Serial.println(CAMTILT);
   if (CAMTILT > MAXTILT + MARGINTILT) {
      CAMTILT = MAXTILT + MARGINTILT;
   } else if (CAMTILT < MARGINTILT) {
	   CAMTILT = MARGINTILT;
   }   
   if (CAMROTATE > MAXROTATE + MARGINROTATE) {
      CAMROTATE = MAXROTATE + MARGINROTATE;
   } else if (CAMROTATE < MARGINROTATE) {
	   CAMROTATE = MARGINROTATE;
   }
   Serial.print("CAMROTATE : ");Serial.print(CAMROTATE);Serial.print("CAMTILT : ");Serial.println(CAMTILT);
   int cr = map(CAMROTATE,0,180,COUNT_LOW,COUNT_HIGH);
   int ct = map(CAMTILT,0,180,COUNT_LOW,COUNT_HIGH);       
   Serial.print("cr : ");Serial.print(cr);Serial.print("ct : ");Serial.println(ct);
   if ((rpos == -1 && tpos == -1) || (rpos != -1 && tpos != -1 && rpos != cr && tpos != ct)) {
     ledcWrite(CROTATE, cr);
     ledcWrite(CTILT, ct);
     rpos = cr;
     tpos = ct;
   }
}
