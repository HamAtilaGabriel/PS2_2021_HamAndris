#define FCPU 16000000
#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#define MSG_MAX_LEN 32

#include <avr/io.h>

int volatile time_count = 0;

int pwm_val[3][2];
bool pwm_parsing = false;
int pwm_pos = 0;

void setup() { 
  OCR2A = 65535; // 100 ms
  TCCR2A |= 1 << WGM21;
  TCCR2B |= (1 << CS22) | (1 << CS20) | (1 << CS21);
  TIMSK2 = (1 << OCIE2A);
  
  USART_Init(MYUBRR);
  PWM_Init();
  
  
  sei();
}

void loop() {

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

void PWM_Init(void)
{
  // 1. LED ca iesire.
  DDRB |= 1 << PB2; //red
  DDRB |= 1 << PB1; //green
  DDRD |= 1 << PD6; //blue
  
  // 2. Alegem Timer PWM CS
  TCCR0B = (1 << CS02);
  TCCR1B = (1 << CS12);
      
  // 3. Mod FAST PWM
  TCCR0A |= (1 << WGM00) | (1 << WGM01);
  TCCR1A |= 1 << WGM10;
  TCCR1B |= 1 << WGM12;
  
  // 4. Setam OCR output mode
  TCCR0A |= (1 << COM0A1);
  TCCR1A |= (1 << COM1A1);
  TCCR1A |= (1 << COM1B1);
 
  
  // 5. Test valori OCR   // P 22 44 98 W 
  OCR1B = 0;    // Red
  OCR1A = 0;    // Green
  OCR0A = 0;    // Blue
}

ISR(USART_RX_vect)
{
   char c = USART_Receive();
    if (c == '$')
   {
     // incepem parsare PWM
     pwm_parsing = true;
     pwm_pos = 0;
   }
   else if (c == '&')
   {
      // incheiem parsarea PWM
      pwm_parsing = false;
      
      OCR1B = pwm_val[0][0] * 10 + pwm_val[0][1];    // Red 
      OCR1A = pwm_val[1][0] * 10 + pwm_val[1][1];    // Green 
      OCR0A = pwm_val[2][0] * 10 + pwm_val[2][1];    // Blue
   }
   else
   {
    if (pwm_parsing == true)
    {
      // salvam valorile pwm...
      pwm_val[pwm_pos/2][pwm_pos%2] = c - '0';     
      pwm_pos++;
    } 
   }
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
