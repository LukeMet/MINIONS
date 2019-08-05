#define ledPin 2

String incomingString;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

    Serial.println("AT+ADDRESS=1");
    
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()){
    incomingString = Serial.readString();
    if (incomingString.indexOf("Hello!") > 0) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      Serial.println("Received message:");
      Serial.println(incomingString);
    }
  }

}
