#define FREQ 100 // HZ
#define DUTY 0.5 // %/100

#define MS_TO_COMP(SCALER) (F_CPU / (SCALER * 1000.0))

void setup() {
    pinMode(A0, INPUT);
    Serial.begin(1000000);
    
    // PWM 100Hz, 50% DUTY, PIN B2
    DDRB |= _BV(DDB2);
    TCCR1A = _BV(WGM11) | _BV(WGM10);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // scaler 8
    TCCR1A |= _BV(COM1B1);
    OCR1A = 1000 / 100 * MS_TO_COMP(8);
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
