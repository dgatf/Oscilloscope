//uint8_t val = 5;

#define MS_TO_COMP(SCALER) (F_CPU / (SCALER * 1000.0))

void setup() {
  pinMode(A0, INPUT);
  Serial.begin(1000000);
  //DDRB |= _BV(DDB3);
  //TCCR2A = _BV(COM2A1) | _BV(COM2A0) | _BV(WGM21) | _BV(WGM20);
  //TCCR2B = _BV(WGM22) | _BV(CS22)| _BV(CS21) | _BV(CS20);
  //OCR2A = 120;

  DDRB |= _BV(DDB2);
  TCCR1A = _BV(WGM11) | _BV(WGM10);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // scaler 8

  TCCR1A |= _BV(COM1B1);
  OCR1A = 10 * MS_TO_COMP(8);
  OCR1B = 0.5 * OCR1A;
  
}

void loop() {
  static uint16_t ts = 0;
  if ((uint16_t)micros() - ts > 100)
  {
    ts = micros();
    uint8_t val = analogRead(A0) >> 2; 
    Serial.write(val);
  }
}
