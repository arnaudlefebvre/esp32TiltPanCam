#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "secrets.h"

#define CAMERA_MODEL_AI_THINKER
#if defined(CAMERA_MODEL_AI_THINKER) //zie esp32-cam schema v1.6.pdf
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

//Servo tick conf, see https://github.com/jkb-git/ESP32Servo/blob/master/examples/Multiple-Servo-Example-ESP32/Multiple-Servo-Example-ESP32.ino
#define COUNT_LOW 1638
#define COUNT_HIGH 7864
#define TIMER_WIDTH 16

#include "esp32-hal-ledc.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

//Servos PINS
int SROTATE = 12; 
int STILT = 14;      
//Servos CHANNELS
//WARNING : USE THIS CHANNEL, Others conflict with cam
int CROTATE = 4;
int CTILT = 5;
//Servos fq
int FROTATE = 50;
int FTILT = 50;

//Max tilt & Rotate angle
int MAX_TILT = 55;
int MAX_ROTATE = 180;

//Margin tilt & rotate
int MARGIN_TILT = 5;
int MARGIN_ROTATE = 0;

//Dynamic Servos Pos
extern int CAM_TILT, CAM_ROTATE;

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

void handleMinMaxCam() {
   Serial.print("CAM_ROTATE : ");Serial.print(CAM_ROTATE);Serial.print("CAM_TILT : ");Serial.println(CAM_TILT);
   if (CAM_TILT > MAX_TILT + MARGIN_TILT) {
      CAM_TILT = MAX_TILT + MARGIN_TILT;
   } else if (CAM_TILT < MARGIN_TILT) {
     CAM_TILT = MARGIN_TILT;
   }   
   if (CAM_ROTATE > MAX_ROTATE + MARGIN_ROTATE) {
      CAM_ROTATE = MAX_ROTATE + MARGIN_ROTATE;
   } else if (CAM_ROTATE < MARGIN_ROTATE) {
     CAM_ROTATE = MARGIN_ROTATE;
   }
   Serial.print("CAM_ROTATE : ");Serial.print(CAM_ROTATE);Serial.print("CAM_TILT : ");Serial.println(CAM_TILT);  
}

void doServo() {
   handleMinMaxCam(); 
   int cr = map(CAM_ROTATE,0,180,COUNT_LOW,COUNT_HIGH);
   int ct = map(CAM_TILT,0,180,COUNT_LOW,COUNT_HIGH);       
   Serial.print("cr : ");Serial.print(cr);Serial.print("ct : ");Serial.println(ct);
   ledcWrite(CROTATE, cr);
   ledcWrite(CTILT, ct);
}
