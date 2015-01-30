// demo runing two tasks round-robin
// must disable Brige
#include <ChibiOS_AVR.h>
#include <Firmata.h>
#include <SoftwareSerial.h>

/*
 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).
 */
 
SoftwareSerial mySerial(10, 11); // RX, TX

void stringCallback(char *myString)
{
  Firmata.sendString(myString);
}

void sysexCallback(byte command, byte argc, byte*argv)
{
  Firmata.sendSysex(command, argc, argv);
}

// declare and initialize a mutex for limiting access to threads
MUTEX_DECL(demoMutex);
//------------------------------------------------------------------------------
// 32 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread01, 32);
static msg_t Thread01(void *arg) {
  while (1) {
    
    chMtxLock(&demoMutex);//// wait to enter print region
    
    // copy from UART to USB-CDC
    int c = Serial1.read();                 // read from UART
    if (c != -1) {                         // got anything?
        if(c == 'H' || c == 'h')
        	digitalWrite(13, HIGH);
        else if(c == 'L' || c == 'l')
        	digitalWrite(13, LOW);
        	
        Serial.write(c);                  //    write to USB-CDC
    }  
    
    chMtxUnlock();//// exit protected region
  }
  return 0;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// 32 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread02, 32);
static msg_t Thread02(void *arg) {
  while (1) {
    chMtxLock(&demoMutex);// wait to enter print region
    // copy from USB-CDC to UART
    int c = Serial.read();                 // read from USB-CDC
    if (c != -1) {                        // got anything?
      Serial1.write(c);                  //    write to UART
    }
    chMtxUnlock();// exit protected region
    
  }
  return 0;
}

long linuxBaud = 500000;
void setup() {
  Serial.begin(115200);//Serial for Debug with USB2Serial
  while (!Serial) {} // wait for USB Serial  
  /*
  Firmata.setFirmwareVersion(FIRMATA_MAJOR_VERSION, FIRMATA_MINOR_VERSION);
  Firmata.attach(STRING_DATA, stringCallback);
  Firmata.attach(START_SYSEX, sysexCallback);
  Firmata.begin(linuxBaud);
*/
  Serial1.begin(linuxBaud);  // open serial connection to Linux
  
  // set the data rate for the SoftwareSerial port
  mySerial.begin(4800);
  mySerial.println("Hello, world?");
  
  chBegin(mainThread);// start kernel and continue main thread as loop
  while(1);
}

//------------------------------------------------------------------------------
// main thread and runs at NORMALPRIO
void mainThread() {

  // start Thread01 thead
  chThdCreateStatic(waThread01, sizeof(waThread01), NORMALPRIO, Thread01, NULL);  
  // start Thread02 thead
  chThdCreateStatic(waThread02, sizeof(waThread02), NORMALPRIO, Thread02, NULL);
  
  //main thread
  while (1) {
    //digitalWrite(13, HIGH);
    //chThdSleepMilliseconds(100);
    //digitalWrite(13, LOW);
    //chThdSleepMilliseconds(400);
    /*
      if (mySerial.available())
    		Serial.write(mySerial.read());
  		if (Serial.available())
    		mySerial.write(Serial.read());
    		*/
  }
}
//------------------------------------------------------------------------------
void loop() {
  // not used
}
