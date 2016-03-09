/* 
 * Reading liquid flow rate using Arduino UNO and Water Flow Sensor
 * For additional information, please refer:
 * http://www.seeedstudio.com/wiki/index.php?title=G1/2_Water_Flow_sensor
 * http://www.protostack.com/blog/2010/09/external-interrupts-on-an-atmega168/
 * http://thewanderingengineer.com/2014/08/11/arduino-pin-change-interrupts/
 * 
 */

#include <LiquidCrystal.h>
#include <Ethernet.h>

boolean reading = false;

volatile int nbTopsFan1 = 0;              // measuring the rising edges of the signal for meter 1
volatile int nbTopsFan2 = 0;              // measuring the rising edges of the signal for meter 2
volatile int m1State_Last = 0;
volatile int m2State_Last = 0;

char nbTopsFan1_Formated[5];
char nbTopsFan2_Formated[5];

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int loopDelay = 500;

/*------------------------------------------------
 * Ethernet settings 
 */
byte addressMAC[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server = EthernetServer(80);


/*------------------------------------------------
 * Initial Setup function
 */
void setup() {
  Serial.begin(9600);                 // This is the setup function where the serial port is initialised,

  //Ethernet settings
  Ethernet.begin(addressMAC);
  server.begin();
  Serial.print(Ethernet.localIP());
  //End
  
  //Interrupt changes
  byte pins[] = {A1, A2};
  initPin(pins, INPUT_PULLUP);
  initInt(pins);
  //End

  //LCD start
  lcd.begin(16, 2);
  //End 

  //LCD Display strings
  displayString(0,0,"Flow (L/H)  v3.0");
  displayString(1,0,"M1:");
  displayString(1,8,",M2:");
  //End
}

/*------------------------------------------------
 *  Loop function
 */
void loop () {
  nbTopsFan1 = 0;                     // Set NbTops1 to 0 ready for calculations
  nbTopsFan2 = 0;                     // Set NbTops2 to 0 ready for calculations
  delay (loopDelay);                  // Wait 1 second
  sprintf(nbTopsFan1_Formated, "%04d", calculateFlow(nbTopsFan1));
  sprintf(nbTopsFan2_Formated, "%04d", calculateFlow(nbTopsFan2));
  if (nbTopsFan1 > 0 || nbTopsFan2 > 0) {
    Serial.print("[");
    Serial.print(Ethernet.localIP());
    Serial.print("] ");
    Serial.print ("Sensors (L/hour) => M1: ");
    Serial.print (nbTopsFan1_Formated); // Prints the number calculated above
    Serial.print (" ,M2: ");
    Serial.print (nbTopsFan2_Formated); // Prints the number calculated above
    Serial.print ("\r\n");              // Prints "L/hour" and returns a  new line
    webClient("M1: " + String(nbTopsFan1_Formated) + " ,M2: " + String(nbTopsFan2_Formated));
  }
  displayString(1, 3,  nbTopsFan1_Formated);
  displayString(1, 12, nbTopsFan2_Formated); 
}


/*------------------------------------------------
 * Display support function
 * line: line to print
 * column: column to print
 * str: string to print
 */
void displayString(int line, int column, String str) {
  lcd.setCursor(column, line);
  lcd.print(str);
}

/*------------------------------------------------
 * Calcule flow function
 * frequency: line to print
 */ 
int calculateFlow(int frequency) {    
  return frequency * 60 / 7.5;
}

/*------------------------------------------------
 * Initiate Pins
 * pins: array of desired pins
 * state: desired state
 */
void initPin(byte pins[], int state) {
  for (int n = 0; n <= sizeof(pins); n++) {
    pinMode(pins[n], state);                 // Pin A0 is input to which a switch is connected
    //digitalWrite(pins[n], state);             // Configure internal pull-up resistor
  }
}

/*------------------------------------------------
 * Initiate interrupting for desired pins
 * pins: array of desired pins
 */
void initInt(byte pins[]) {
  for (int n = 0; n <= sizeof(pins); n++) {
    *digitalPinToPCMSK(pins[n]) |= bit (digitalPinToPCMSKbit(pins[n]));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pins[n])); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pins[n])); // enable interrupt for the group
  }
}

/*------------------------------------------------
 * Function for dealing with interruption on 
 * port PORT C
 */
ISR(PCINT1_vect) {
  int m1State = digitalRead(A1);
  if (m1State != m1State_Last) {
    m1State_Last = m1State;
    nbTopsFan1++;
  }
  int m2State = digitalRead(A2);
  if (m2State != m2State_Last) {
    m2State_Last = m2State;
    nbTopsFan2++;
  }
}


void webClient(String str) {

  EthernetClient client = server.available();

  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;

    while (client.connected()) {
      if (client.available()) {
        //if (!sentHeader) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          sentHeader = true;
        //}

        client.println(str);
        client.print("<br>");

      }
    }
    delay(1); // give the web browser time to receive the data
    client.stop(); // close the connection:
  }

}

