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
char incomingBuffer[255];
char outgoingBuffer[bufferSize]; //Where we get the UDP data
uint8_t iofs = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("Welcome to SkyNet.");

    connectToAP();
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Disconnected.");
        while (1)
        {
            if (connectToAP())
                break;
        }
    }
    else
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

            Serial.print(incomingBuffer);
        }
        else
        {
            iofs = 0;
            //If serial data is recived send it to UDP
            if (Serial.available())
            {
                while (1)
                {
                    if (Serial.available())
                    {
                        outgoingBuffer[iofs] = char(Serial.read()); // read char from UART
                        if (iofs < bufferSize - 1)
                        {
                            iofs++;
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                udp.beginPacket(ServerIP, udpPort); //Send Data to Master unit
                udp.write(outgoingBuffer, iofs);
                udp.endPacket();
            }
        }
    }
}

bool connectToAP()
{
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
