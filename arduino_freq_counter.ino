/*
  This program is designed to measure the frequency of external pulses using a timer.
  It uses an external signal source on the T0 pin and calculates the frequency of this signal,
  which is then output to the serial monitor.

  Key features:
  - Timer0 is used to count external pulses (Timer3 for ATmega64/128).
  - To avoid conflicts with Arduino functions (like delay(), millis()), 
    Compare Match interrupt is used instead of the overflow interrupt, comparing with the maximum value 0xFF.
  - Timer1 is used as an interval timer with a 1-second interval.
    After each interval, the pulse count is calculated and displayed in Hz.

  The program supports several AVR microcontrollers (ATmega88PA, ATmega168, ATmega328P, ATmega16, ATmega32, ATmega64, ATmega128).
*/

volatile int32_t count, overfl;  // Global variables for counting pulses and overflows
uint8_t countReady = 0;          // Flag to indicate when data is ready for output

void setup() {

#if defined(__AVR_ATmega88PA__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  // Configuring TIMER0 to use an external signal on the T0 pin
  // On microcontrollers like the ATmega328P, Timer0 overflow interrupts conflict 
  // with Arduino functions like delay() and millis(). 
  // Therefore, Compare Match interrupt with the maximum value 0xFF is used instead.
  // This allows external pulses to be counted without interfering with system functions.
  #define TIMER_CNT_ISR TIMER0_COMPA_vect
  #define TIMER_INTERVAL_ISR TIMER1_COMPA_vect
  #define TIMER_CNT TCNT0
  TCCR0A = (1 << WGM01); // CTC mode (Clear Timer on Compare Match)
  TCCR0B = 7; // External clock source on the T0 pin
  TIMSK0 = (1 << OCIE0A); // Enable TIMER0 Compare Match interrupt
  OCR0A = 0xff; // Set the compare value to 0xFF (maximum)

  // Timer accuracy: Timer0 operates in CTC mode, ensuring precise interrupts 
  // each time the counter reaches the value 0xFF.

  // Timer1 is used as an interval timer. 
  // The system clock (16 MHz) is divided by 1024 (prescaler) and the timer counts to 15624,
  // resulting in an interval of exactly 1 second:
  // 16,000,000 / 1024 / 15624 ≈ 1 Hz (1 interrupt per second).
  TCCR1A = 0; 
  TCCR1B = (1 << WGM12) | 5; // CTC mode | Prescaler F_CLK/1024
  OCR1A = 15624; // 1-second interval
  TIMSK1 = (1 << OCIE1A); // Enable TIMER1 Compare Match interrupt

#elif defined(__AVR_ATmega16__)  || defined(__AVR_ATmega32__)
  // Configuring TIMER0 to use an external signal on the T0 pin
  #define TIMER_CNT_ISR TIMER0_COMP_vect
  #define TIMER_INTERVAL_ISR TIMER1_COMPA_vect
  #define TIMER_CNT TCNT0
  TCCR0 = (1 << WGM01) | 7; // CTC mode | External clock source on the T0 pin
  TIMSK = (1 << OCIE0); // Enable TIMER0 Compare Match interrupt
  OCR0 = 0xff; // Set the compare value to 0xFF (maximum)

  // Timer1 is used to create an interval timer (16 MHz / 1024 / 15624),
  // resulting in a 1-second interval.
  TCCR1A = 0; 
  TCCR1B = (1 << WGM12) | 5; // CTC mode | Prescaler F_CLK/1024
  OCR1A = 15624; // 1-second interval
  TIMSK |= (1 << OCIE1A); // Enable TIMER1 Compare Match interrupt

#elif defined(__AVR_ATmega64__)  || defined(__AVR_ATmega128__)
  // Configuring TIMER3 to use an external signal on the T3 pin (PE6)
  #define TIMER_CNT_ISR TIMER3_OVF_vect
  #define TIMER_INTERVAL_ISR TIMER1_COMPA_vect
  #define TIMER_CNT TCNT3
  TCCR3A = 0;
  TCCR3B = 7; // Normal mode | External clock source on the T3 pin

  // Timer1 is used to create an interval timer (16 MHz / 1024 / 15624),
  // resulting in a 1-second interval.
  TCCR1A = 0; 
  TCCR1B = (1 << WGM12) | 5; // CTC mode | Prescaler F_CLK/1024
  OCR1A = 15624; // 1-second interval
  TIMSK = (1 << OCIE1A); // Enable TIMER1 Compare Match interrupt
  ETIMSK = (1 << TOIE3); // Enable TIMER3 Overflow interrupt

#else
  #error "Use a compatible microcontroller or check the index (e.g., __AVR_ATmega168PA__ instead of __AVR_ATmega168__)."
#endif

  Serial.begin(9600);  // Initialize serial communication
}

void loop() {
  if (countReady) {  // If data is ready for output
    Serial.print(count);  // Output the pulse count
    Serial.println("Hz");  // Output the unit (Hz)
    countReady = 0;  // Reset the ready flag
  }
}

// Interrupt service routine for the pulse counter (TIMER)
ISR(TIMER_CNT_ISR) {
  overfl++;  // Increment the overflow counter
}

// Interrupt service routine for the interval timer
ISR(TIMER_INTERVAL_ISR) {
  count = (int32_t) (overfl << 8) + TIMER_CNT;  // Calculate the pulse count
  TIMER_CNT = 0;  // Reset the counter
  overfl = 0;  // Reset the overflow counter
  countReady = 1;  // Set the flag indicating data is ready
}
