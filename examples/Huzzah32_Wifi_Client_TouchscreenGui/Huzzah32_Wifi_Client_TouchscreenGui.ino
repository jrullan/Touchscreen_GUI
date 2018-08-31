/*
 *  This sketch sends a message to a TCP server
 *
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <neotimer.h>
#include <Canvas_STMPE610.h>
#include <Terminal.h>

Canvas_STMPE610 canvas = Canvas_STMPE610(TFT_PORTRAIT,BLACK);
Terminal terminal = Terminal(240,70,TERMINAL_SCROLL_UP,1);

Neotimer timer = Neotimer(1000);
WiFiMulti WiFiMulti;

void setup()
{
    Serial.begin(115200);
    delay(10);
    canvas.init();
    terminal.setColors(BLACK,GREEN,WHITE);
    canvas.add(&terminal,0,0);
    
    // We start by connecting to a WiFi network
    WiFiMulti.addAP("Linksys03507", "4antgxupkm");
    WiFiMulti.addAP("Samsung Galaxy Note9_2618", "hvdl9798");
    WiFiMulti.addAP("frulrom2319", "familiarullan2319");

    terminal.print("Trying to connect to Wifi...");

    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    char buffer[terminal.maxCharacters];

    String ssid = WiFi.SSID();
    ssid.toCharArray(buffer,ssid.length()+1);
    terminal.print("Connected to");
    terminal.print(buffer);
    
    String ipAddress = WiFi.localIP().toString();
    ipAddress.toCharArray(buffer,ipAddress.length()+1);
    terminal.print("IP Address");
    terminal.print(buffer);
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(500);
}


void loop()
{

/*
    const uint16_t port = 80;
    const char * host = "192.168.1.1"; // ip or dns



    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;

    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        Serial.println("wait 5 sec...");
        delay(5000);
        return;
    }

    // This will send the request to the server
    client.print("Send this data to server");

    //read back one line from server
    String line = client.readStringUntil('\r');
    client.println(line);

    Serial.println("closing connection");
    client.stop();

    Serial.println("wait 5 sec...");
    delay(5000);
*/
}


