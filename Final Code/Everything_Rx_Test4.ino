//-------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------

// MQTT Includes
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// OLED Includes
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//-------------------------------------------------------------------------------------------
// VARIABLES
//-------------------------------------------------------------------------------------------

// MQTT Constants and objects
const char* ssid = "Reformat Hard Drive"; // SSID/Password combination
const char* password = "Totolink1234nomore@number4";
const char* mqtt_server = "192.168.1.101"; // MQTT Broker IP address
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// OLED Constants and Objects
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int OLEDcount = 0;

// Receive LED
#define ledPin1 2

// Tx State Variables
#define ledPin2 14
String TxState = "SLEEP";

// Sample Period 
String SamplePeriod = "4";
String NewSamplePeriod;
bool ClearToSend = false;
bool QueuedSample = false;
bool QueuedRestart = false;

// OLED Pins
// SCL -> GPIO22
// SDA -> GPIO21

// Text Extraction Variables
String incomingString;
String sendaddress;
String messagesize;
String Temperature;
String Turbidity;
String Voltage;
String PacketNo;
String rssi;
String snr;

//-------------------------------------------------------------------------------------------
// SETUP BELOW
//-------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  // LoRa Rx Setup
  Serial.println("AT+RESET");
  delay(20);
  Serial.println("AT+ADDRESS=1");
  delay(20);
  Serial.println("AT+NETWORKID=5");

  // OLED Setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // MQTT Setup
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  PublishSamplePeriod();

}

//-------------------------------------------------------------------------------------------
// CODE BEFORE LOOP
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// LOOP BELOW
//-------------------------------------------------------------------------------------------

void loop() {

  // Check that Rx is connected to Broker
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  CheckForMessage();
  CheckClearToSend();

  ScreenSaver();
  
}

//-------------------------------------------------------------------------------------------
// ALL OTHER FUNCTIONS
//-------------------------------------------------------------------------------------------

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Print to OLED
  display.setCursor(0, 5);
  display.clearDisplay();
  display.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Print to Serial
    Serial.print(".");    

    // Print to OLED
    display.print(".");
    display.display(); 
  }

  // Print to OLED
  display.setCursor(0, 5);
  display.clearDisplay();
  display.println("Connected");
  display.display(); 

  // Print to Serial
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      // TxWake();
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      // TxSleep();
    }
  }

  if (String(topic) == "esp32/SamplePeriod") {
    // Serial.print("Changing sample period to ");
    // Serial.println(messageTemp);
    NewSamplePeriod = messageTemp;
    QueuedSample = true;
    Serial.println("Sample Period changed to " + NewSamplePeriod + "s - waiting for Tx to wake");
    //ChangeSamplePeriod();
  }

  if (String(topic) == "esp32/restart") {
    QueuedRestart = true;
      // Serial.println("+RCV=0,7,RESTART,-125,-31");
  }
  
}




void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
      client.subscribe("esp32/SamplePeriod");
      client.subscribe("esp32/restart");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void CheckForMessage(){
  if (Serial.available()>0){

    incomingString = Serial.readString();
  
    if(incomingString.indexOf("RCV") > 0){
      // Filter Messages
      // Data
      if(incomingString.indexOf("DATA") > 0){
        ExtractData();
        Serial.println("Data received");
      }

      // State
      if(incomingString.indexOf("STATE") > 0){
        
        if(incomingString.indexOf("WAKE") > 0){
          digitalWrite(ledPin2, HIGH);
          TxState = "WAKE";
          delay(100);
          Serial.println("Tx awake");
        }
        
        if(incomingString.indexOf("SLEEP") > 0){
          digitalWrite(ledPin2, LOW);
          TxState = "SLEEP";
          delay(100);
          Serial.println("Tx asleep");
          ClearToSend = false;
        }
        UpdateTxStateLED();
      }

      //Clear to Send
      if((incomingString.indexOf("CLEARTOSEND")) > 0){
        ClearToSend = true;
      }

      //Tx Shutdown
      if((incomingString.indexOf("SHUTDOWN")) > 0){
        Serial.println("Tx Battery drained - Tx shutting down");
        PublishShutdown();
      }
      
    }
  }
}

void ExtractData(){
  digitalWrite(ledPin1, HIGH);
  Serial.println(incomingString);
  
  // Find Placeholders
  // If message contains commas, could be problematic. So should use better extraction than commas to make more robust
  int equal = incomingString.indexOf('=');
  int comma1 = incomingString.indexOf(','); // Find comma - beginning of <Payload Length>
  int comma2 = incomingString.indexOf(',', comma1 + 1); // Find second comma, starting from index after the first comma
  int plus1 = incomingString.indexOf('+', comma2 + 1);
  int plus2 = incomingString.indexOf('+', plus1 + 1);
  int plus3 = incomingString.indexOf('+', plus2 + 1);
  int plus4 = incomingString.indexOf('+', plus3 + 1);
  int comma3 = incomingString.indexOf(',', plus4 + 1);
  int comma4 = incomingString.indexOf(',', comma3 + 1);

  // Extract Sender Address
  sendaddress = incomingString.substring(equal, comma1);
  // Extract Message Size
  messagesize = incomingString.substring(comma1 + 1, comma2); // message size lies between these two indexes
  // Extract Temperature
  Temperature = incomingString.substring(comma2 + 1 + 4, plus1); // message begins at (index after second comma), ends at (index after second comma + message size)
  // Extract Turbidity
  Turbidity = incomingString.substring(plus1 + 1, plus2);
  // Extract Voltage
  Voltage = incomingString.substring(plus2 + 1, plus3);
  // Extract Sample Period
  SamplePeriod = incomingString.substring(plus3 + 1, plus4);
  // Extract Packet Number
  PacketNo = incomingString.substring(plus4 + 1, comma3);
  //Extract RSSI
  rssi = incomingString.substring(comma3 + 1, comma4);
  // Extract SNR
  snr = incomingString.substring(comma4 + 1);

  PrintOLED(Temperature, Turbidity, Voltage, PacketNo, rssi, snr);
  
  PublishTemp(Temperature.toFloat());
  PublishTurb(Turbidity.toFloat());
  PublishVolt(Voltage.toFloat());
  PublishSamplePeriod();
  PublishRSSI(rssi.toFloat());
  PublishSNR(snr.toFloat());

  delay(100);
  digitalWrite(ledPin1, LOW);
}

void PrintOLED(String Temp, String Turb, String Volt, String PN, String rssi, String snr){
  OLEDcount += 1;
  display.setCursor(0, 5);
  display.clearDisplay();
  Volt = Volt.substring(0, Volt.length()-3);
  // Display static text
  display.println("Last Message:");
  display.println(" ");
  display.println("Temperature:  "+Temp+"*C");
  display.println("Turbidity:       "+Turb);
  display.println("Battery:         "+Volt+"%");
  display.println("Packet:           "+PN);
  // display.println("RSSI:            "+rssi);
  // display.println("SNR:             "+snr);
  // display.println("Tx State:       "+TxState);
  display.println("Sample Period:  "+SamplePeriod+"min");
  display.display(); 
}

void PublishTemp(float Temp){
    // Convert the value to a char array
    char tempString[8];
    dtostrf(Temp, 1, 2, tempString);
    client.publish("esp32/temperature", tempString);
}

void PublishTurb(float Turb){
    // Convert the value to a char array
    char turbString[8];
    dtostrf(Turb, 1, 2, turbString);
    client.publish("esp32/turbidity", turbString);
}

void PublishVolt(float Volt){
    // Convert the value to a char array
    char voltString[8];
    dtostrf(Volt, 1, 2, voltString);
    client.publish("esp32/battery", voltString);
}

void PublishRSSI(float rssi_){
    // Convert the value to a char array
    char rssiString[8];
    dtostrf(rssi_, 1, 2, rssiString);
    client.publish("esp32/rssi", rssiString);
}

void PublishSNR(float snr_){
    // Convert the value to a char array
    char snrString[8];
    dtostrf(snr_, 1, 2, snrString);
    client.publish("esp32/snr", snrString);
}

void PublishShutdown(){
    client.publish("esp32/shutdown", "Tx shutting down");
}

void UpdateTxStateLED(){
  if(TxState == "SLEEP"){
    digitalWrite(ledPin2, LOW);
    delay(100);
    client.publish("esp32/TxState", "sleep");
  }
  if(TxState == "WAKE"){
    digitalWrite(ledPin2, HIGH);
    delay(100);
    client.publish("esp32/TxState", "wake");
  } 
}

void ChangeSamplePeriod(){
  Serial.println("Changing sample period to " + NewSamplePeriod + " minutes");
  delay(2000);
  String Message = "AT+SEND=0," + String(NewSamplePeriod.length()+12) + ",CHANGESAMPLE" + NewSamplePeriod;
  Serial.println(Message);
  Serial.flush();
  
  // Publish Updated Period
  //PublishSamplePeriod();
}

void PublishSamplePeriod(){
  String SP = SamplePeriod;
  int str_len = SP.length() + 1;
  char char_array[str_len]; //buffer
  //SP.toCharArray(char_array, str_len);
  SP.toCharArray(char_array, str_len);
  client.publish("esp32/TxSamplePeriod", char_array);
}


void CheckClearToSend(){
  if (ClearToSend == true){
    if (QueuedSample == true){
      ChangeSamplePeriod();
      QueuedSample = false;
      delay(2000);
    }
    if (QueuedRestart == true){
      Serial.println("Restarting Tx");
      delay(2000);
      Serial.println("AT+SEND=0,7,RESTART");
      Serial.flush();
      QueuedRestart = false;
      delay(2000);
    }
  }
}

void ScreenSaver(){
    if (OLEDcount == 49){
    testfillrect();
    OLEDcount = 0;
  }
}

void testfillrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=3) {
    // The INVERSE color is used so rectangles alternate white/black
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, INVERSE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  delay(2000);
}
