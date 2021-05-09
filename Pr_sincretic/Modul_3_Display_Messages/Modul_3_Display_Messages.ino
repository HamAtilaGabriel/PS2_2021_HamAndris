#define FCPU 16000000
#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#define MSG_MAX_LEN 32

#include <LiquidCrystal.h>
#include <avr/io.h>

int volatile time_count = 0;

const int rs = 12, en = 11, d4 = 4, d5 = 3, d6 = 2, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);//Adresa, coloane, linii



char msg_val[MSG_MAX_LEN + 1]; 
bool msg_parsing = false;
int msg_pos = 0;
volatile bool msg_rec_completed = false;
volatile bool refresh_needed = false;

static char row_up[17] = "PS 2";
static char row_down[17] = "Semestrul II";

void setup() {
  OCR2A = 65535; // 100 ms
  TCCR2A |= 1 << WGM21;
  TCCR2B |= (1 << CS22) | (1 << CS20) | (1 << CS21);
  TIMSK2 = (1 << OCIE2A);

  USART_Init(MYUBRR);
  Disp_Init();
  
  sei();
}

void loop() {
   if (true == msg_rec_completed)
  {
      
      memset(row_up, 0, sizeof(row_up));
      memset(row_down, 0, sizeof(row_down));
      memcpy(row_up, &msg_val[0], 16);
      memcpy(row_down, &msg_val[16], 16);
      
      msg_rec_completed = false;
      memset(msg_val, 0, sizeof(msg_val));
      
      Debug_Display_Rows();
  }
  if (true == refresh_needed)
  {
    Refresh_Display();
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

void Disp_Init(void)
{
   // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Sincretic 2021");
}

ISR(USART_RX_vect)
{
   char c = USART_Receive();
    if (c == '#')
   {
       msg_parsing = true;  
       msg_pos = 0;
   }
   else if (c == '^')
   {
       msg_parsing = false;
       msg_rec_completed = true;
   }
   else
   {
    if (msg_parsing == true)
    {
      if (('\n' != c) && (msg_pos < MSG_MAX_LEN))
      {
        msg_val[msg_pos++] = c;   
      }
    }
  }
   
}

ISR(TIMER2_COMPA_vect)
{
  time_count++;
  if (!(time_count % 100))
  {
     refresh_needed = true;
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

void Debug_Display_Rows()
{
  for (int i=0; i < 16; i++)
  {
    USART_Transmit(row_up[i]);
  }
  USART_Transmit('\n');
  
  for (int i=0; i < 16; i++)
  {
    USART_Transmit(row_down[i]);
  }
  
  USART_Transmit('\n');
  USART_Transmit('\n');
}

void Refresh_Display()
{
  cli();
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(row_up);
  lcd.setCursor(0, 1);
  lcd.print(row_down);
  refresh_needed = false;
  sei();
}
