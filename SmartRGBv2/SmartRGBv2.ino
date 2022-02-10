#include <WiFi.h>
#include <WiFiUdp.h>

#define WIFI_SSID "BlueRoom"
#define WIFI_PASSWORD "Psv2097531"
#define PORT 555

// Ping & Pong
// C -> S data: none
// S -> C data: none
#define PACKET_CODE_PING 0x00
#define PACKET_CODE_PONG 0x01

// Get color
// C -> S data: none
// S -> C data: current R, current G, current B
#define PACKET_CODE_GET 0x10
#define PACKET_CODE_GET_RES 0x11

// Set immediate
// C -> S data: goal R, goal G, goal B
// S -> C data: control goal R, control goal G, control goal B
#define PACKET_CODE_SET 0x20
#define PACKET_CODE_SET_RES 0x21

// Set transition
// C -> S data: goal R, goal G, goal B
// S -> C data: control goal R, control goal G, control goal B
#define PACKET_CODE_TRANSITION 0x30
#define PACKET_CODE_TRANSITION_RES 0x31

// Burst
// C -> S data: goal R, goal G, goal B
#define PACKET_CODE_BURST 0x40

// Error
// S -> C data: none
#define PACKET_CODE_ERROR 0xff

// PWM Pins
#define LED_PIN_R 16
#define LED_PIN_G 17
#define LED_PIN_B 21

// PWM Channels
#define LED_CHANNEL_R 0
#define LED_CHANNEL_G 1
#define LED_CHANNEL_B 2

// PWM Frequency and Resolution
#define LED_PWM_FREQ 5000
#define LED_PWM_RES 8

WiFiUDP udp;

int currentR;
int currentG;
int currentB;

int startR;
int startG;
int startB;

int goalR;
int goalG;
int goalB;

int startTime;
int goalTime;

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting...");
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED);

  Serial.println("Connected.");

  udp.begin(PORT);
  
  Serial.print("Listening at: ");
  Serial.print(WiFi.localIP());
  Serial.print(" on port ");
  Serial.println(PORT);

  ledcSetup(LED_CHANNEL_R, LED_PWM_FREQ, LED_PWM_RES);
  ledcSetup(LED_CHANNEL_G, LED_PWM_FREQ, LED_PWM_RES);
  ledcSetup(LED_CHANNEL_B, LED_PWM_FREQ, LED_PWM_RES);

  ledcAttachPin(LED_PIN_R, LED_CHANNEL_R);
  ledcAttachPin(LED_PIN_G, LED_CHANNEL_G);
  ledcAttachPin(LED_PIN_B, LED_CHANNEL_B);

  ledcWrite(LED_CHANNEL_R, 0);
  ledcWrite(LED_CHANNEL_G, 0);
  ledcWrite(LED_CHANNEL_B, 0);

  startTime = millis();
}

void loop() {
  int len = udp.parsePacket();
  if (len) {
    uint8_t buf[len];
    udp.read(buf, len);

    // Burst optimization
    if (buf[0] == PACKET_CODE_BURST && len == 4) {
      setTransition(buf[1], buf[2], buf[3], 0);
      updateColor();
      return;
    }

    IPAddress ip = udp.remoteIP();
    uint16_t port = udp.remotePort();

    udp.beginPacket(ip, port);

    switch (buf[0]) {
      case PACKET_CODE_PING: {
          if (len == 1) {
            udp.write(PACKET_CODE_PONG);
          } else {
            udp.write(PACKET_CODE_ERROR);
          }
        } break;
      case PACKET_CODE_GET: {
          if (len == 1) {
            udp.write(PACKET_CODE_GET_RES);
            udp.write(currentR);
            udp.write(currentG);
            udp.write(currentB);
          } else {
            udp.write(PACKET_CODE_ERROR);
          }
        } break;
      case PACKET_CODE_SET: {
          if (len == 4) {
            setTransition(buf[1], buf[2], buf[3], 0);
            udp.write(PACKET_CODE_SET_RES);
            udp.write(buf[1]);
            udp.write(buf[2]);
            udp.write(buf[3]);
          } else {
            udp.write(PACKET_CODE_ERROR);
          }
        } break;
      case PACKET_CODE_TRANSITION: {
          if (len == 5) {
            setTransition(buf[1], buf[2], buf[3], buf[4]);
            udp.write(PACKET_CODE_TRANSITION_RES);
            udp.write(buf[1]);
            udp.write(buf[2]);
            udp.write(buf[3]);
            udp.write(buf[4]);
          } else {
            udp.write(PACKET_CODE_ERROR);
          }
        } break;
      default: {
          udp.write(PACKET_CODE_ERROR);
        } break;
    }

    udp.endPacket();
  }

  updateColor();
}

int normalize(int value) {
  return min(max((int)(pow(value / 255.0, 3) * 255), 0), 255);
}

void updateColor() {
  int currentTime = millis();
  if ((goalTime + 1000) < currentTime) {
    return;
  }

  double percentage = min((double)(millis() - startTime) / ((goalTime - startTime) + 1), 1.0);
  
  currentR = startR + (goalR - startR) * percentage;
  currentG = startG + (goalG - startG) * percentage;
  currentB = startB + (goalB - startB) * percentage;
  
  ledcWrite(LED_CHANNEL_R, normalize(currentR));
  ledcWrite(LED_CHANNEL_G, normalize(currentG));
  ledcWrite(LED_CHANNEL_B, normalize(currentB));
}

void setTransition(int r, int g, int b, int t) {
  startR = currentR;
  startG = currentG;
  startB = currentB;

  goalR = r;
  goalG = g;
  goalB = b;

  startTime = millis();
  goalTime = startTime + t * 1000;
}
