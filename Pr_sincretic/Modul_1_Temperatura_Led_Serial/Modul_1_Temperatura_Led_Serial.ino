#define FCPU 16000000
#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

int volatile time_count = 0;

void setup() {
  DDRB |= 1 << PD4; 
  USART_Init(MYUBRR);
  adc_init();

  OCR2A = 65535; // 100 ms
  TCCR2A |= 1 << WGM21;
  TCCR2B |= (1 << CS22) | (1 << CS20) | (1 << CS21);
  TIMSK2 = (1 << OCIE2A);
  sei();
}

void loop() {
   
}

ISR(USART_RX_vect)
{
   char c = USART_Receive();
   if(c == 'A')
   {
     PORTB |= 1 << PD4;
   }
   else if (c == 'S')
   {
     PORTB &= ~(1 << PD4);
   } 
   
}

ISR(TIMER2_COMPA_vect)
{
  time_count++;
  if (!(time_count % 50))
  {
    Transmitere_Temperatura();
  }
}

void USART_Init(unsigned int ubrr)
{
  /* Set baud rate */
  UBRR0H = (unsigned char)(ubrr>>8);
  UBRR0L = (unsigned char)ubrr;
  /* Enable receiver and transmitter */
  UCSR0B = (1<<RXEN0)|(1<<TXEN0) | (1 << RXCIE0);
  /* Set frame format: 8data, 2stop bit */
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

unsigned char USART_Receive(void)
{
  /* Wait for data to be received */
  while (!(UCSR0A & (1<<RXC0)))
  ;
  /* Get and return received data from buffer */
  return UDR0;
}

void USART_Transmit(unsigned char data)
{
  /* Wait for empty transmit buffer */
  while (!(UCSR0A & (1<<UDRE0)))
  ;
  /* Put data into buffer, sends the data */
  UDR0 = data;
}


void adc_init()
{
  ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));
  ADMUX  |= (1<<REFS0);
  ADCSRA |= (1<<ADEN);
  ADCSRA |= (1<<ADSC);
}


uint16_t read_adc(uint8_t channel)
{
  ADMUX &= 0xf0;
  ADMUX |= channel;
  ADCSRA |= (1<<ADSC);
  while(ADCSRA & (1<<ADSC));
  return ADC;
}

double read_temp() //read and return temperature
{                                                                                                               
 double voltage=0;                      
 double Temperature=0;                     
 double temperatureCelsius=0; int reading;
 reading=read_adc(1);
 voltage=reading*5.0; voltage/=1024.0; 
 temperatureCelsius=(voltage-0.5)*100;
 return temperatureCelsius;
}

void Transmitere_Temperatura() //read and return temperature
{                                                                                                               
 float voltage=0;                                          
 float temperatureCelsius=0; int reading;
 reading=read_adc(1);
 voltage=reading*5.0; voltage/=1024.0; 
 temperatureCelsius=(voltage-0.5)*100;
 
 int parte_intreaga, parte_zecimala; 
 int int_temperatureCelsius = temperatureCelsius * 100;
  
  parte_intreaga = int_temperatureCelsius/100;
  parte_zecimala = int_temperatureCelsius % 100;
  
  char buf[50];
  memset(buf, 0, sizeof(buf));
  sprintf(buf, "T=%d.%d\n", parte_intreaga, parte_zecimala);
  //sprintf(buf, "T=%f\n", temperatureCelsius); aparent nu mere %f pe serial nuj de ce
  
  for (int i=0; i < strlen(buf); i++)
  {
    USART_Transmit(buf[i]);
  }
}
