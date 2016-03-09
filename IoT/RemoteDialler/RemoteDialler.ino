//ARDUINO 1.0+ ONLY
//ARDUINO 1.0+ ONLY


#include <Ethernet.h>
#include <SPI.h>
boolean reading = false;

////////////////////////////////////////////////////////////////////////
//CONFIGURE
////////////////////////////////////////////////////////////////////////
//byte ip[] = { 146, 250, 181, 201 };    //Manual setup only
//byte gateway[] = { 146, 250, 180, 1 }; //Manual setup only
//byte subnet[] = { 255, 255, 252, 0 };  //Manual setup only

// if need to change the MAC address (Very Rare)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetServer server = EthernetServer(80); //port 80
////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);

  //Pins 10,11,12 & 13 are used by the ethernet shield

  pinMode(13, OUTPUT);

  Ethernet.begin(mac);
  //Ethernet.begin(mac, ip, gateway, subnet); //for manual setup

  server.begin();
  Serial.println(Ethernet.localIP());

}

void loop() {

  // listen for incoming clients, and process qequest.
  checkForClient();

}

void checkForClient() {

  EthernetClient client = server.available();

  if (client) {

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;

    while (client.connected()) {
      if (client.available()) {

        if (!sentHeader) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          sentHeader = true;
        }

        char c = client.read();

        if (reading && c == ' ') reading = false;
        if (c == '?') reading = true; //found the ?, begin reading the info

        if (reading) {
          Serial.print(c);
          switch (c) {
            case '1':
              triggerPin(13, HIGH, client);
              break;
            case '0':
              triggerPin(13, LOW, client);
              break;
          }

        }

        if (c == '\n' && currentLineIsBlank)  break;

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }

      }
    }

    delay(1); // give the web browser time to receive the data
    client.stop(); // close the connection:

  }

}

void triggerPin(int pin, int mode, EthernetClient client) {
  //blink a pin - Client needed just for HTML output purposes.
  if (mode == LOW) {
    client.print("Turning OFF pin ");
  }
  if (mode == HIGH) {
    client.print("Turning ON pin ");
  }
  client.println(pin);
  client.print("<br>");

  digitalWrite(pin, mode);
}
