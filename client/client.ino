#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "AntaresSkyNet";
const char *pass = "nimbus2000";

// This is client code

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

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println("Welcome to SkyNet.");

  connectToAP();
}

void loop()
{
  // Check WiFi connectivity;
  if (WiFi.status() != WL_CONNECTED)
  {
    // Reconnect
    Serial.println("Disconnected.");
    while (1)
    {
      if (connectToAP())
        break;
    }
  }
  else
  {
    // We're connected, rock on!

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

            udp.beginPacket(ServerIP, udpPort); //Send Data to Slave unit
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
}

bool connectToAP()
{
  Serial.println("Connecting ...");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass); //Connect to access point
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Start UDP
  Serial.println("Starting UDP");
  udp.begin(udpPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}
