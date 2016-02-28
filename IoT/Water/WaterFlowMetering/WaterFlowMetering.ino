// reading liquid flow rate using Seeeduino and Water Flow Sensor from Seeedstudio.com
// Code adapted by Charles Gantt from PC Fan RPM code written by Crenn @thebestcasescenario.com
// http:/themakersworkbench.com http://thebestcasescenario.com http://seeedstudio.com

#include <LiquidCrystal.h>

volatile int NbTopsFan1;              // measuring the rising edges of
// the signal for meter 1
volatile int NbTopsFan2;              // measuring the rising edges of
// the signal for meter 2
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int loopDelay=500;

void setup() //
{
  Serial.begin(9600);                 // This is the setup function where
  // the serial port is initialised,
  initIO();
  initInterrupt();
  lcd.begin(16, 2);                   // start the library
  lcd.setCursor(0, 0);
  lcd.print("Flow (L/H)  v2.0");     // print a simple message
  lcd.setCursor(0, 1);
  lcd.print("M1:");
  lcd.setCursor(8, 1);
  lcd.print(",M2:");
}

void loop ()
{
  NbTopsFan1 = 0;                     // Set NbTops1 to 0 ready for calculations
  NbTopsFan2 = 0;                     // Set NbTops2 to 0 ready for calculations
  sei();                              // Enables interrupts
  delay (loopDelay);                  // Wait 1 second
  cli();                              // Disable interrupts
  printMeter(1, calculateFlow(NbTopsFan1));
  printMeter(2, calculateFlow(NbTopsFan2));
}

void initIO() {
  pinMode(A0, INPUT);                 // Pin A0 is input to which a switch is connected
  digitalWrite(A0, HIGH);             // Configure internal pull-up resistor
  pinMode(A1, INPUT);                 // Pin A1 is input to which a switch is connected
  digitalWrite(A1, HIGH);             // Configure internal pull-up resistor
  pinMode(A2, INPUT);                 // Pin A2 is input to which a switch is connected
  digitalWrite(A2, HIGH);             // Configure internal pull-up resistor
}

void initInterrupt() {
  cli();                              // switch interrupts off while messing with their settings
  PCICR = 0x02;                       // Enable PCINT1 interrupt
  PCMSK1 = 0b00000111;
  sei();                              // turn interrupts back on
}


ISR(PCINT1_vect) {                    // Interrupt service routine.
  // Every single PCINT8..14 (=ADC0..5) change
  // will generate an interrupt: but this will
  // always be the same interrupt routine
  if (digitalRead(A0) == 0) NbTopsFan1++;
  if (digitalRead(A1) == 0) NbTopsFan2++;
}

int calculateFlow(int NbTopsFan) {
  int Calc;
  Calc = (NbTopsFan * 60 / 7.5);      // (Pulse frequency x 60) / 7.5Q, = flow rate in L/hour
  return Calc;
}

void printMeter(int meter, int value) {
  static char formatedValue[5];
  sprintf(formatedValue, "%04d", value);
  Serial.print ("Value: ");
  Serial.print (formatedValue);     // Prints the number calculated above
  Serial.print (" L/hour\r\n");     // Prints "L/hour" and returns a  new line
  switch (meter) {
    case 1:
      lcd.setCursor(3, 1);
      lcd.print(formatedValue);
      break;
    case 2:
      lcd.setCursor(12, 1);
      lcd.print(formatedValue);
      break;
    default:
      break;
  }
}

