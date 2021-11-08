#define serial Serial

/* PWM signal */
#define PWM_FREQ 1000 // HZ (1000HZ=1ms)
#define PWM_DUTY 0.5 // %/100

#define MS_TO_COMP(SCALER) (F_CPU / (SCALER * 1000.0))

#if defined(ARDUINO_ARCH_AVR)

ISR(ADC_vect) {
    serial.write((uint8_t)ADCH);
}

void setup() {
    serial.begin(1000000);

    /* ADC init
       Capture time: F_CPU / ADC_PRESCALER / 13 ADC TICKS:
                     Prescaler 32: 1.6M / 32 / 13 = 76923 -> 1 / 38461 = 26 us
                     Prescaler 16: 1.6M / 16 / 13 = 76923 -> 1 / 76923 = 13 us (this is too fast for some devices)
    */
    ADMUX = _BV(REFS0) | _BV(ADLAR);
    //ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2);            // prescaler 16
    ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS0); // prescaler 32
    ADCSRB = 0;
    ADCSRA |= _BV(ADSC);

    /*  PWM signal is available at pin OC1B. To test the Oscilloscope app connect pin OC1B to pin A0
        Adjust PWM signal with PWM_FREQ and PWM_DUTY. Default: PWM 1000Hz, 50% DUTY
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
    DDRB |= _BV(DDB2);
    TCCR1A = _BV(WGM11) | _BV(WGM10);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // scaler 8
    TCCR1A |= _BV(COM1B1);
    OCR1A = 1000.0 / PWM_FREQ * MS_TO_COMP(8);
    OCR1B = PWM_DUTY * OCR1A;

}

void loop() {

}

#else

/* Interval between analog readings (for non ATmega boards. It has to be the same as defined in the Oscilloscope settings */
#define INTERVAL  200// microseconds (us). Minimun is 200

void setup() {
    serial.begin(1000000);
}

void loop() {
    static uint16_t ts = 0;
    if ((uint16_t)micros() - ts > INTERVAL)
    {
        ts = micros();
        uint8_t val = analogRead(A0) >> 2;
        serial.write(val);
    }
}

#endif
