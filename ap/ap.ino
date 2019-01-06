#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "AntaresSkyNet";
const char *pass = "nimbus2000";

// This is Access Point "Server" code

IPAddress ServerIP(192, 168, 4, 1);
IPAddress ClientIP(192, 168, 4, 2);

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
unsigned int udpPort = 2000;

#define bufferSize 8192
const byte bufferCharSize = 255;
char incomingBuffer[bufferCharSize];
char outgoingBuffer[bufferSize]; //Where we get the UDP data
uint8_t iofs = 0;

bool newData = false;

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println("Welcome to SkyNet.");
  WiFi.softAP(ssid, pass); //Create Access point

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //Start UDP
  Serial.println("Starting UDP");
  udp.begin(udpPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}

void loop()
{
  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    // We've received a UDP packet, send it to serial
    int len = udp.read(incomingBuffer, 255);
    if (len > 0)
    {
      incomingBuffer[len] = 0;
    }
    Serial.print("Received From UDP: ");
    Serial.println(incomingBuffer);
  }
  else
  {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '@';
    char endMarker = ';';
    char rc;

    while (Serial.available() > 0)
    {
      rc = Serial.read();

      if (recvInProgress == true)
      {
        if (rc != endMarker)
        {
          incomingBuffer[ndx] = rc;
          ndx++;
          if (ndx >= bufferCharSize)
          {
            ndx = bufferCharSize - 1;
          }
        }
        else
        {
          incomingBuffer[ndx] = '\0'; // terminate the string
          recvInProgress = false;

          Serial.print("Received From Serial: ");
          Serial.println(incomingBuffer);

          udp.beginPacket(ClientIP, udpPort); //Send Data to Slave unit
          udp.write(incomingBuffer, ndx);
          udp.endPacket();
          ndx = 0;
        }
      }

      else if (rc == startMarker)
      {
        recvInProgress = true;
      }
    }
  }
}
