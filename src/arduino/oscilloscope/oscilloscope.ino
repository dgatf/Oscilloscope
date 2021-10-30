/* Interval between analog readings. It has to be the same as defined in the Oscilloscope settings */
#define INTERVAL  100// microseconds (us)

/* PWM signal properties */
#define PWM_FREQ 100 // HZ
#define PWM_DUTY 0.5 // %/100

#define MS_TO_COMP(SCALER) (F_CPU / (SCALER * 1000.0))

void setup() {
    pinMode(A0, INPUT);
    Serial.begin(1000000);

#if defined(ARDUINO_ARCH_AVR)
    /*  PWM signal is available at pin OC1B. To test the Oscilloscope app connect pin OC1B to pin A0
        Adjust PWM signal with PWM_FREQ and PWM_DUTY. Default: PWM 100Hz, 50% DUTY
        Pin OC1B:
          Uno:        pin 10 (PB2)
          Leonardo:   pin 10 (PB6)
          ATmega2560: pin 12 (PB6)
    */
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__)
    DDRB |= _BV(DDB6);
#else
    DDRB |= _BV(DDB2);
#endif
    TCCR1A = _BV(WGM11) | _BV(WGM10);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // scaler 8
    TCCR1A |= _BV(COM1B1);
    OCR1A = 1000 / PWM_FREQ * MS_TO_COMP(8);
    OCR1B = PWM_DUTY * OCR1A;
#endif

}

void loop() {
    static uint16_t ts = 0;
    if ((uint16_t)micros() - ts > INTERVAL)
    {
        ts = micros();
        uint8_t val = analogRead(A0) >> 2;
        Serial.write(val);
    }
}
