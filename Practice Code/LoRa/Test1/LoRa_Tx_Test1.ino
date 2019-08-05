#define ledPin 2

unsigned long lastTransmission;
const int interval = 1000;
bool first = true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  Serial.println("AT+ADDRESS=0");
  
}

void loop() {
  // put your main code here, to run repeatedly:

  if (first) {
    Serial.println("AT+SEND=1,6,HELLO!");
    digitalWrite(ledPin, HIGH);
    delay(3000);
    digitalWrite(ledPin, LOW);
    delay(3000);
  
    Serial.println("AT+SEND=1,6,MY");
    digitalWrite(ledPin, HIGH);
    delay(3000);
    digitalWrite(ledPin, LOW);
    delay(3000);
  
    Serial.println("AT+SEND=1,6,NAME");
    digitalWrite(ledPin, HIGH);
    delay(3000);
    digitalWrite(ledPin, LOW);
    delay(3000);
  
    Serial.println("AT+SEND=1,6,IS");
    digitalWrite(ledPin, HIGH);
    delay(3000);
    digitalWrite(ledPin, LOW);
    delay(3000);
  
    Serial.println("AT+SEND=1,6,LUKE!!");
    digitalWrite(ledPin, HIGH);
    delay(3000);
    digitalWrite(ledPin, LOW);
    delay(3000);
  
    first = false;
  }

}
