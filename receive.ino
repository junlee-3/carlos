#include <SPI.h>
#include <RH_RF95.h>
#include <Servo.h>

#define RFM95_CS 15
#define RFM95_RST 16
#define RFM95_INT 5

#define RF95_FREQ 915.0

#define LED 2

RH_RF95 rf95(RFM95_CS, RFM95_INT);

Servo servo;
Servo ESC;

void setup() 
{

  pinMode(LED, OUTPUT);
  digitalWrite(LED,HIGH);   
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(9600);
  delay(100);

  servo.attach(5);
  ESC.attach(16 ,1000,2000);

  Serial.println();

  Serial.println("Gateway Module starting…");

  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);


  while (!rf95.init()) {

    Serial.println("LoRa radio init failed");

    while (1);

  }

  Serial.println("LoRa radio init OK!");

  if (!rf95.setFrequency(RF95_FREQ)) {

    Serial.println("setFrequency failed");

    while (1);

  }

  Serial.print("Set Freq to: ");

  Serial.println(RF95_FREQ);

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95 / 96/97/98 modules using the transmitter pin PA_BOOST, then
  // you can set transmission powers from 5 to 23 dBm:

  rf95.setTxPower(23, false);

}

void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED, HIGH);
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      String receivedMsg = String((char*)buf);
      Serial.println(receivedMsg);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      // Check if the received message starts with "92"
      if (receivedMsg.startsWith("92")) {
        // Extract steering and speed values
        int pos1 = receivedMsg.indexOf("steer");
        int pos2 = receivedMsg.indexOf("speed");
        
        if (pos1 != -1 && pos2 != -1) {
          int steerDegrees = receivedMsg.substring(pos1 + 5, pos2).toInt();
          int speed = receivedMsg.substring(pos2 + 5).toInt();
        
          // Control the servos
          servo.write(steerDegrees);
          ESC.write(speed);
        
          // Send a reply
          uint8_t data[] = "Why don't scientists trust atoms? Because they make up everything!";
          rf95.send(data, sizeof(data));
          rf95.waitPacketSent();
          Serial.println("Sent a reply");
          digitalWrite(LED, LOW);
        } else {
          Serial.println("Received message does not contain 'steer' and 'speed'");
        }
      } else {
        Serial.println("Received message does not start with '92'");
      }
      
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}
