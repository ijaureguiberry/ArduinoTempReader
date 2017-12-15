/*
 */

#include <dht.h>

#define DHT11_S1_PIN 5
#define DHT11_S2_PIN 6
dht DHT;

#include <SoftwareSerial.h>
SoftwareSerial softSerial(2, 3); // RX, TX

/**
 * Read data from ESP and write it to serial.
 */
void read_reply()
{
    while(softSerial.available() > 0) 
    {
        char a = softSerial.read();
        if(a == '\0')
            continue;
        if(a != '\r' && a != '\n' && (a < 32))
            continue;
        Serial.print(a);
    }
}

void connect_wifi()
{
    Serial.println("Connecting");
    softSerial.println("AT+CWMODE=3");
    delay(500);
    read_reply();
    softSerial.println("AT+CWJAP=\"SSID\",\"PASSWORD\"");
    delay(5000); // WAIT 5 seconds to pair.
    read_reply();
}

void send_temp(float t0, float h0, float t1, float h1)
{
    String msg = "t0=" + String(t0, 2) + "&h0=" + String(h0, 2);
    msg += "&t1=" + String(t1, 2) + "&h1=" + String(h1, 2);
    String text = "GET /report/?" + msg + " HTTP/1.0\n\n";
    int len = text.length();
  
    // Connecting
    softSerial.println("AT+CIPSTART=\"TCP\",\"192.168.1.80\",8000");
    delay(500);
    read_reply();
  
    // Send data
    softSerial.println("AT+CIPSEND=" + String(len));
    delay(500);
    softSerial.print(text);
    delay(500);
    softSerial.println("AT+IPD"); // request data.
    delay(2000);
    read_reply();

    //softSerial.println("AT+CIPCLOSE");
    //delay(500);
    //read_reply();
}

void setup() 
{
  uint32_t baud = 9600;
  Serial.begin(baud);
  softSerial.begin(baud);
  Serial.print("SETUP!! @");
  Serial.println(baud);
  connect_wifi();
}

/**
 * Read temp and humid, retrying some times...
 */
bool read_dht_vals(int pin, int &temp, int &humid)
{
    int chk;
    int retries = 3;
    while (retries > 0) {
        chk = DHT.read11(pin);
        --retries;
        if (chk == DHTLIB_OK) {
            retries = 0; // exit loop
            temp = DHT.temperature;
            humid = DHT.humidity;
        } else {
            Serial.println("Unable to read DHT...");
            if (retries > 0) {
                delay(1000);
                Serial.println("  - retrying...");
            }
        }
    }
    return chk == DHTLIB_OK;
}
void loop() 
{
    int chk;
    int t0, t1, h0, h1;
    t0 = t1 = h0 = h1 = -1;

    read_dht_vals(DHT11_S1_PIN, t0, h0);
    read_dht_vals(DHT11_S2_PIN, t1, h1);
    Serial.println("TEMP0: " + String(t0) + " - " + String(h0));
    Serial.println("TEMP1: " + String(t1) + " - " + String(h1));
    send_temp(t0, h0, t1, h1);

    while(softSerial.available() > 0) 
    {
      char a = softSerial.read();
      if(a == '\0')
        continue;
      if(a != '\r' && a != '\n' && (a < 32))
        continue;
      Serial.print(a);
    }
    
    while(Serial.available() > 0)
    {
      char a = Serial.read();
      // Serial.write(a);
      softSerial.write(a);
    }
    delay(120000);
}
