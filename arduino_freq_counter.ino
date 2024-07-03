volatile int32_t count, overfl;
uint8_t countReady = 0;

void setup() {

#if defined(__AVR_ATmega88PA__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  //use TIMER0 as input pin T0
#define TIMER_CNT_ISR TIMER0_COMPA_vect
#define TIMER_INTERVAL_ISR TIMER1_COMPA_vect
#define TIMER_CNT TCNT0
  TCCR0A = (1 << WGM01); // CTC mode
  TCCR0B = 7; //External clock source on T0 pin
  TIMSK0 = (1 << OCIE0A); // TIMER0 compA isr
  OCR0A = 0xff; //full
  //use TIMER1 as interval 16MHz /1024 /15624
  TCCR1A = 0; //
  TCCR1B = (1 << WGM12) | 5; // CTC mode | F_CLK/1024
  OCR1A = 15624; //16ms
  TIMSK1 = (1 << OCIE1A); // TIMER1 compA isr

#elif defined(__AVR_ATmega16__)  || defined(__AVR_ATmega32__)
  //use TIMER0 as input pin T0
#define TIMER_CNT_ISR TIMER0_COMP_vect
#define TIMER_INTERVAL_ISR TIMER1_COMPA_vect
#define TIMER_CNT TCNT0
  TCCR0 = (1 << WGM01) | 7; // CTC mode | External clock source on T0 pin
  TIMSK = (1 << OCIE0); // TIMER0 comp isr
  OCR0 = 0xff; //full
  //use TIMER1 as interval  16MHz /1024 /15624
  TCCR1A = 0; //
  TCCR1B = (1 << WGM12) | 5; // CTC mode | F_CLK/1024
  OCR1A = 15624; //16ms
  TIMSK |= (1 << OCIE1A); // TIMER1 compA isr

#elif defined(__AVR_ATmega64__)  || defined(__AVR_ATmega128__)
  //use TIMER2 as input pin T2 / PD7  [32]
  //use TIMER3 as input pin T3 / PE6  [8]
#define TIMER_CNT_ISR TIMER3_OVF_vect
#define TIMER_INTERVAL_ISR TIMER1_COMPA_vect
#define TIMER_CNT TCNT3
  TCCR3A = 0;
  TCCR3B = 7; // Normal mode | External clock source on T3 pin
  //TIMSK = (1 << OCIE2); // TIMER2 comp isr
  //OCR2 = 0xff; //full
  //use TIMER1 as interval  16MHz /1024 /15624
  TCCR1A = 0; //
  TCCR1B = (1 << WGM12) | 5; // CTC mode | F_CLK/1024
  OCR1A = 15624; //1s  
  TIMSK = (1 << OCIE1A); // TIMER1 compA isr
  ETIMSK = (1 << TOIE3); // TIMER3 Overflow Interrupt Enable

#else
#error "Check use controller or  replace __AVR_ATmega168__ to  __AVR_ATmega168PA__ "
#endif

Serial.begin(9600);
}



void loop() {
  if (countReady) {
    Serial.print(count);
    Serial.println("Hz");
    countReady = 0;
  }
}




ISR(TIMER_CNT_ISR) {
  overfl++;
}

ISR(TIMER_INTERVAL_ISR) {
  count = (int32_t) (overfl << 8) + TIMER_CNT;  
  TIMER_CNT = 0;
  overfl = 0;
  countReady = 1;
}
