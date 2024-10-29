#define F_CPU 16000000UL   // Частота тактового генератора, например, 16 МГц
#define PRESCALER 1024     // Делитель для таймера
#define INTERVAL_HZ 1      // Частота измерительного интервала (1 Гц, т.е. 1 секунда)

#define OCR1A_VALUE ((F_CPU / PRESCALER / INTERVAL_HZ) - 1)  // Вычисление значения для регистра сравнения

volatile int32_t count, overfl;  // Глобальные переменные для подсчета импульсов и переполнений
uint8_t countReady = 0;          // Флаг для вывода готовых данных

void setup() {

#if defined(__AVR_ATmega88PA__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  // Настройка TIMER0 для внешнего сигнала на пине T0
#define TIMER_CNT_ISR TIMER0_COMPA_vect
#define TIMER_INTERVAL_ISR TIMER1_COMPA_vect
#define TIMER_CNT TCNT0
  TCCR0A = (1 << WGM01);   // CTC mode (Clear Timer on Compare Match)
  TCCR0B = 7;              // Внешний источник тактового сигнала на T0
  TIMSK0 = (1 << OCIE0A);  // Разрешаем прерывание TIMER0 по совпадению
  OCR0A = 0xff;            // Устанавливаем максимальное значение сравнения

  // Настройка TIMER1 как интервала в 1 секунду
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | 5;  // Режим CTC | Делитель F_CLK/1024
  OCR1A = OCR1A_VALUE;        // Вычисленное значение для 1-секундного интервала
  TIMSK1 = (1 << OCIE1A);     // Разрешение прерывания TIMER1 по совпадению

#elif defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__)
  // Настройка TIMER0 для внешнего сигнала на пине T0
#define TIMER_CNT_ISR TIMER0_COMP_vect
#define TIMER_INTERVAL_ISR TIMER1_COMPA_vect
#define TIMER_CNT TCNT0
  TCCR0 = (1 << WGM01) | 7;  // Режим CTC | Внешний источник на T0
  TIMSK = (1 << OCIE0);      // Разрешение прерывания TIMER0 по совпадению
  OCR0 = 0xff;               // Максимальное значение сравнения

  DDRD |= (1 << PD5);  // Настраиваем пин OC1A (PD5) как выход
  TCCR1A = (1 << COM1A0); // Toggle OC1A
  TCCR1B = (1 << WGM12) | 5;  // CTC mode | Prescaler F_CLK/1024
  OCR1A = OCR1A_VALUE;    // Вычисленное значение для 1-секундного интервала
  TIMSK |= (1 << OCIE1A); // Enable TIMER1 Compare Match interrupt

#elif defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
  // Настройка TIMER3 для внешнего сигнала на пине T3 (PE6)
#define TIMER_CNT_ISR TIMER3_OVF_vect
#define TIMER_INTERVAL_ISR TIMER1_COMPA_vect
#define TIMER_CNT TCNT3
  TCCR3A = 0;
  TCCR3B = 7;  // Нормальный режим | Внешний источник на T3

  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | 5;  // CTC mode | Prescaler F_CLK/1024
  OCR1A = OCR1A_VALUE;        // Вычисленное значение для 1-секундного интервала
  TIMSK = (1 << OCIE1A);      // Разрешение прерывания TIMER1 по совпадению
  ETIMSK = (1 << TOIE3);      // Разрешение прерывания TIMER3 по переполнению

#else
#error "Use a compatible microcontroller or check the index (e.g., __AVR_ATmega168PA__ instead of __AVR_ATmega168__)."
#endif

  Serial.begin(9600);  // Инициализация последовательного соединения
}

void loop() {
  if (countReady) {            // Если данные готовы
    Serial.print(count * 16);  // Выводим количество импульсов
    Serial.println(" Hz");     // Выводим единицы измерения (Гц)
    countReady = 0;            // Сбрасываем флаг
  }
}

ISR(TIMER_CNT_ISR) {
  overfl++;  // Увеличиваем счетчик переполнений
}

ISR(TIMER_INTERVAL_ISR) {
  count = (int32_t)(overfl << 8) + TIMER_CNT;  // Считаем количество импульсов
  TIMER_CNT = 0;                               // Сбрасываем счетчик
  overfl = 0;                                  // Сбрасываем счетчик переполнений
  countReady = 1;                              // Устанавливаем флаг готовности
}
