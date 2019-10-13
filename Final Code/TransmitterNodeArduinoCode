// Temp Includes
#include <OneWire.h>
#include <DallasTemperature.h>

// Temp Constants and Objects
const int oneWireBus = 4;     
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// Turb Constant
const int turbPin = 34;

const int battPin = 33;

//Sensor Variables
float Temperature;
float Voltage;
int Turbidity;

// LoRa Tx Variables
#define ledPin1 2
//String str = "AT+SEND=";
//String Rx_address = "1";
String mess_length;
String temp;
String message;

// Text Extraction Variables
String incomingString;

// Tx State Variables
#define ledPin2 26
String TxState = "WAKE";

// Deep Sleep Variables
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define M_TO_S_FACTOR 60  /* Conversion factor for minutes to seconds */
// #define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int SamplePeriod = 5; // minutes
RTC_DATA_ATTR int bootCount = 0;

void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);
  delay(1000);

  // LED Setup
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  // LoRa Tx setup
  Serial.println("AT+RESET");
  delay(20);
  Serial.println("AT+ADDRESS=0");
  delay(20);
  Serial.println("AT+NETWORKID=5");
  delay(200);

  //Increment boot number
  ++bootCount;

  // Tell Rx about Wake
  TxState = "WAKE";
  digitalWrite(ledPin2, HIGH);
  SendState();
  delay(2000);
  
  // Temp Setup
  sensors.begin();
  delay(50);

  // Transmit Data
  LoRaTx();
  delay(2000);

  // Listen to Tx
  Serial.println("AT+SEND=1,12,CLEARTOSEND");
  delay(2000);
  
  // Listen
  for (int i=1; i<10; i=i+1){
      CheckForMessage();
      delay(1000);
  }

  delay(2000);
  CheckBattery();
  delay(2000);

  // Configure wake up source
  esp_sleep_enable_timer_wakeup(SamplePeriod * uS_TO_S_FACTOR * M_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for " + String(SamplePeriod) + " minute(s)");
  delay(3000);
  
  // Tell Rx about Sleep
  TxState = "SLEEP";
  digitalWrite(ledPin2, LOW);
  SendState();
  delay(2000);

  // Enter Deep Sleep
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
  
}

void loop() {
    //This will never be called
}



void LoRaTx(){
  GetAllReadings();
  message = CreateMessage();
  Serial.println(message);
  Serial.flush();
  delay(2000);
  
  digitalWrite(ledPin1, HIGH);
  delay(100);
  digitalWrite(ledPin1, LOW);
}

String CreateMessage(){
  String temp = String(Temperature);
  String volt = String(Voltage);
  String turb = String(Turbidity);
  String count = String(bootCount);
  
  mess_length = temp.length() + turb.length() + volt.length() + count.length() + String(SamplePeriod).length() + 4 + 4;
  message = "AT+SEND=1," + mess_length + ",DATA"+ temp + "+" + turb + "+" + volt + "+" + String(SamplePeriod) + "+" + count;
  return message;
}


void SendState(){
  String message = "AT+SEND=1," + String(TxState.length()+5) + ",STATE" + TxState;
  Serial.println(message);
  Serial.flush();
}

void CheckForMessage(){
  if (Serial.available()>0){

    incomingString = Serial.readString();
  
    if(incomingString.indexOf("RCV") > 0){
      Serial.println(incomingString);
      // Filter Messages
      // Data
      if(incomingString.indexOf("CHANGESAMPLE") > 0){
        int beginning = incomingString.indexOf("CHANGESAMPLE") + 12;
        int comma = incomingString.indexOf(',', beginning + 1);
        String SP_string = incomingString.substring(beginning, comma);
        Serial.println("Changing Sample Period to " + SP_string + " minute(s)");
        SamplePeriod = SP_string.toInt();
      }

      // Restart
      if(incomingString.indexOf("RESTART") > 0){
        Serial.println("Tx entering forced restart");
        delay(1000);
        ESP.restart();
      }

    }
  }
}

void GetAllReadings(){
  // Temperature
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  Temperature = temperatureC;

  //Battery Voltage
  float batteryLevel = map(analogRead(battPin), 0.0f, 4095.0f, 0, 100);
  float UC_percent = (batteryLevel-80)*5;   // UC = Usable Capacity
  Voltage = UC_percent;

  // Turbidity
  Turbidity = analogRead(turbPin);
}

void CheckBattery(){
  if (Voltage <= 15){
    Serial.println("AT+SEND=1,8,SHUTDOWN");
    delay(3000);
    Serial.println("Battery drained - shutting down");
    delay(3000);
    esp_deep_sleep_start();
  }
}
