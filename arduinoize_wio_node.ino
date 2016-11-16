//
// arduinoize_wio_node.ino
//

////////////////////////////////////////////////////////////////////////////////////
// for WioNode Pin Assign
#define PORT0_D0    3
#define PORT0_D1    1
#define PORT1_D0    5
#define PORT1_D1    4

#define I2C_SCL     PORT1_D0
#define I2C_SDA     PORT1_D1

#define BUTTON_FUNC 0

#define LED_BLUE    2
#define GROVE_PWR   15

// initialize functions for WioNode.
#define ENABLE_GROVE_CONNECTOR_PWR() {pinMode(GROVE_PWR, OUTPUT);digitalWrite(GROVE_PWR, HIGH);}
#define INIT_WIO_NODE() {pinMode(BUTTON_FUNC, INPUT);pinMode(LED_BLUE, OUTPUT);ENABLE_GROVE_CONNECTOR_PWR();}
#define LED_ON()  {digitalWrite(LED_BLUE, LOW);}
#define LED_OFF() {digitalWrite(LED_BLUE, HIGH);}

////////////////////////////////////////////////////////////////////////////////////
// for Wifi settings
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/

char *wifi_ssid = "ssid";
char *wifi_password = "password";

char *mqtt_server   = "iot.eclipse.org";
int  mqtt_port      = 1883;

char *mqtt_subscribe_topic = "mqtt_neo_pixel/wio_node/0001";

WiFiClient wifi_client;
void mqtt_sub_callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqtt_client(mqtt_server, mqtt_port, mqtt_sub_callback, wifi_client);

void setup_mqtt() {
  WiFi.begin(wifi_ssid, wifi_password);
  int wifi_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (wifi_count % 2 == 0) {
      LED_ON();
    }
    else {
      LED_OFF();
    }
    wifi_count ++;
    delay(500);
  }

  if (mqtt_client.connect("mqtt_neopixel") == false) {
    reboot();
  }
  LED_ON();
  mqtt_client.subscribe(mqtt_subscribe_topic);
}

void reboot() {
  for(int i = 0; i < 10; ++i) {
    LED_ON()
    delay(100);
    LED_OFF()
    delay(100);
  };

  ESP.restart();

  while (true) {
    LED_ON()
    delay(100);
    LED_OFF()
    delay(100);
  };
}

////////////////////////////////////////////////////////////////////////////////////
// Adafruit_NeoPixel
#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel

#define LED_NUM 8
Adafruit_NeoPixel led_strip = Adafruit_NeoPixel(LED_NUM, PORT1_D0, NEO_GRB + NEO_KHZ800);

byte *buf = new byte[LED_NUM];
uint16_t buf_idx = 0;

uint32_t led_colors[8];

void setup_led() {
  led_colors[0] = led_strip.Color( 0,  0,  0);
  led_colors[1] = led_strip.Color( 6,  0,  0);
  led_colors[2] = led_strip.Color( 0,  6,  0);
  led_colors[3] = led_strip.Color( 0,  0, 12);
  led_colors[4] = led_strip.Color( 6,  6,  0);
  led_colors[5] = led_strip.Color( 0,  6,  6);
  led_colors[6] = led_strip.Color( 6,  0,  6);
  led_colors[7] = led_strip.Color( 6,  6,  6);

  led_strip.begin();
}

void set_led_colors(byte* payload, unsigned int length) {
  int len = LED_NUM;
  if (length < LED_NUM) {
    len = length;
  }

  for (int i = 0; i < len; ++i) {
    int idx = 0;
    byte b = payload[i];
    if (0x30 <= b && b <= 0x37) {
      idx = (b - 0x30); // 0-7
    }
    led_strip.setPixelColor(i, led_colors[idx]);
  }

  if (len < LED_NUM) {
    int off_len = LED_NUM - len;
    for (int i = LED_NUM - off_len; i < LED_NUM; ++i) {
      led_strip.setPixelColor(i, led_strip.Color( 0, 0, 0));      
    }
  }

  led_strip.show();
}

////////////////////////////////////////////////////////////////////////////////////

void setup() {
  INIT_WIO_NODE();
  setup_led();
  setup_mqtt();
}

void loop() {
  // for MQTT
  if (!mqtt_client.connected()) {
    reboot();
  }
  mqtt_client.loop();
}

void mqtt_sub_callback(char* topic, byte* payload, unsigned int length) {
  LED_OFF()
  delay(100);
  LED_ON()

  set_led_colors(payload, length);
}

