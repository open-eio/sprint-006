//soil moisture

#include <SHT1x.h>

// Specify data and clock connections and instantiate SHT1x object
#define dataPin  6
#define clockPin 7
SHT1x sht1x(dataPin, clockPin);

float temp_c;
  float temp_f;
  float humidity;


  
// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <SPIFlash.h> //get it here: https://www.github.com/lowpowerlab/spiflash

#define NODEID        2    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define GATEWAYID     1
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY   RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30 // max # of ms to wait for an ack
#ifdef __AVR_ATmega1284P__
  #define LED           15 // Moteino MEGAs have LEDs on D15
  #define FLASH_SS      23 // and FLASH SS on D23
#else
  #define LED           9 // Moteinos have LEDs on D9
  #define FLASH_SS      8 // and FLASH SS on D8
#endif

#define SERIAL_BAUD   115200

int TRANSMITPERIOD = 150; //transmit a packet to gateway so often (in ms)
char payload[] = "temp:30.3";
char buff[20];
byte sendSize=0;
boolean requestACK = false;
SPIFlash flash(FLASH_SS, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)
RFM69 radio;


  
void setup()  
{
    
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  //radio.setFrequency(919000000); //set frequency to some custom frequency
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  //Serial.println("Adafruit GPS library basic test!");

  delay(1000);

}

uint32_t timer = millis();
void loop()                     // run over and over again
{
  

  delay(2000);
  
  //float humidity = getHumidity();
  
  float humidity=23.5;
  float temperature=30.1;
   
   // Read values from the sensor
  temperature = sht1x.readTemperatureC();
  humidity = sht1x.readHumidity();
  
  
    char sendstring[61];
int sendlength = 61;  // default

    sprintf(sendstring, "%06ld,%06ld",(long) (temperature*100), (long) (humidity*100));


sendlength=49;

    Serial.println(sendstring);
radio.sendWithRetry(GATEWAYID, sendstring, sendlength);
//Serial.println("SEND COMPLETE");

sendlength=42;

Blink(LED,3);

  delay(500);
  
  }


void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

float getTemperature(){
  //Return Temperature in Celsius
  int val=10;
  return (float)val * 0.01 - 40; //convert to celsius
}

float getHumidity(){
  //Return  Relative Humidity
 int val=10;
  return -4.0 + 0.0405 * val + -0.0000028 * val * val; 
}


