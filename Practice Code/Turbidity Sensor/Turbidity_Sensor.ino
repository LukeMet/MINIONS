/ Turbidity sensor output connected to GPIO 34 (Analog ADC1_CH6) 
const int turbPin = 34;

// variable for storing the turbidity value
int turbValue = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  // Reading turbidity value
  turbValue = analogRead(turbPin);
  Serial.println(turbValue);
  delay(500);
}
