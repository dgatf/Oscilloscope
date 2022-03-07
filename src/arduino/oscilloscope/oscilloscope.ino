#define serial Serial

/* PWM signal */
#define PWM_FREQ 20000 // HZ (1000HZ=1ms)
#define PWM_DUTY 0.5 // %/100

/* Capture buffer */
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__)
#define BUFFER_LEN 1500
#endif
#if defined(__AVR_ATmega32U4__)
#define BUFFER_LEN 2000
#endif
#if defined(__AVR_ATmega2560__)
#define BUFFER_LEN 5000
#endif

/* Data bits */
#define DATABITS 8 // 8, (4), 1

/* ADC Prescaler */
#define PRESCALER 32 // 32, 16, 8, (4)

/* Mode */
#define REALTIME 0
#define CAPTURE 1
#define MODE REALTIME // REALTIME, CAPTURE

#define MS_TO_COMP(SCALER) (F_CPU / (SCALER * 1000.0))

volatile bool isBufferFull = false;
volatile uint8_t buffer[BUFFER_LEN];

void (*ADC_vect_handlerP)() = NULL;

ISR(ADC_vect)
{
    if (ADC_vect_handlerP)
        ADC_vect_handlerP();
    /*static uint16_t contByte = 0;
    buffer[contByte] = ADCH;
    if (contByte == BUFFER_LEN)
    {
        isBufferFull = true;
        contByte = 0;
        ADCSRA &= ~_BV(ADIE);

    }
    contByte++;*/
}

/* Capture */

void capture_8b()
{
    static uint16_t contByte = 0;
    buffer[contByte] = ADCH;
    if (contByte == BUFFER_LEN)
    {
        isBufferFull = true;
        contByte = 0;
        ADCSRA &= ~_BV(ADIE);

    }
    contByte++;
}

void capture_4b()
{
    static uint16_t contByte = 0;
    static bool newByte = false;
    if (newByte)
    {
        buffer[contByte] |= (ADCH & 0xF0);
        contByte++;
    }
    else
        buffer[contByte] = ADCH >> 4;
    if (contByte == BUFFER_LEN)
    {
        isBufferFull = true;
        contByte = 0;
        ADCSRA &= ~_BV(ADIE);

    }
    newByte = !newByte;
}

void capture_1b()
{
    static uint16_t contByte = 0;
    static uint8_t data = 0;
    static uint8_t contBit = 0;
    data |= (ADCH & 0x80);
    contBit++;
    if (contBit == 8)
    {
        buffer[contByte] = data;
        contBit = 0;
        contByte++;
    }
    if (contByte == BUFFER_LEN)
    {
        isBufferFull = true;
        contByte = 0;
        ADCSRA &= ~_BV(ADIE);
    }
    data >>= 1;
}

/* Realtime */

void realtime_8b()
{
    serial.write((uint8_t)ADCH);
}

void realtime_4b()
{
    static uint8_t data = 0;
    static bool sendByte = false;
    if (sendByte)
        serial.write(data | (ADCH & 0xF0));
    else
        data = ADCH >> 4;
    sendByte = !sendByte;

}

void realtime_1b()
{
    static uint8_t data = 0;
    static uint8_t cont;
    data |= (ADCH & 0x80);
    cont++;
    if (cont == 8)
    {
        serial.write(data);
        data = 0;
        cont = 0;
    }
    data >>= 1;
}

void sendData()
{
    for (uint16_t i = 0; i < BUFFER_LEN; i++) {
        while (!serial.availableForWrite());
        serial.write(buffer[i]);
    }
    isBufferFull = false;
    delay(2000);
    ADCSRA |= _BV(ADIE);

}

void setup() {
    /*
          ADC init
          Capture time: F_CPU / ADC_PRESCALER / 13 ADC TICKS: (1 byte at 1000000 = 10us)
                      Prescaler 32: 1.6M / 32 / 13 = 38461 -> 1 / 38461 = 26 us -> realtime 8 data bits
                      Prescaler 16: 1.6M / 16 / 13 = 76923 -> 1 / 76923 = 13 us -> realtime 4 data bits
                      Prescaler 8: 1.6M / 8 / 13 = 153846 -> 1 / 153846 = 6.5 us -> capture
                      Prescaler 4: 1.6M / 4 / 13 = 307692-> 1 / 307692 = 3.25 us -> too fast
                      Prescaler 2: 1.6M / 2 / 13 = 615385-> 1 / 615385 = 1.62 us -> too fast
    */
    switch (PRESCALER)
    {
        case 4:
            ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS1);
            break;
        case 8:
            ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS1) | _BV(ADPS0);
            break;
        case 16:
            ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2);
            break;
        case 32:
            ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS0);
            break;
    }
    ADMUX = _BV(REFS0) | _BV(ADLAR);
    ADCSRB = 0;
    ADCSRA |= _BV(ADSC);

    if (MODE == REALTIME)
    {
        serial.begin(1000000, SERIAL_8N1);
        if (DATABITS == 8)
            ADC_vect_handlerP = realtime_8b;
        if (DATABITS == 4)
            ADC_vect_handlerP = realtime_4b;
        if (DATABITS == 1)
            ADC_vect_handlerP = realtime_1b;
    }
    if (MODE == CAPTURE)
    {
        serial.begin(115200, SERIAL_8N1);
        if (DATABITS == 8)
            ADC_vect_handlerP = capture_8b;
        if (DATABITS == 4)
            ADC_vect_handlerP = capture_4b;
        if (DATABITS == 1)
            ADC_vect_handlerP = capture_1b;
    }

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

    TCCR1A = _BV(WGM11) | _BV(WGM10);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // scaler 8
    TCCR1A |= _BV(COM1B1);

    // PWM signal
    OCR1A = 1000.0 / PWM_FREQ * MS_TO_COMP(8);
    OCR1B = PWM_DUTY * OCR1A;

    /*  // sinus signal (1Hz)
        TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // scaler 1
        OCR1A = 1000.0 / 38000 * MS_TO_COMP(8);
        OCR1B = PWM_DUTY * OCR1A;*/

    //TIMSK1 = OCIE1A;

}

void loop() {
    if (isBufferFull)
        sendData();
}
