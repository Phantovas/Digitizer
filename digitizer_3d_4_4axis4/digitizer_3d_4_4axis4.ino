#define SET(x,y) (x |=(1<<y))					//-Bit set/clear macros
#define CLR(x,y) (x &= (~(1<<y)))       		// |
#define CHK(x,y) (x & (1<<y))           		// |
#define TOG(x,y) (x^=(1<<y))            		//-+

//**************************************************************************
//	Hardware connections:
//**************************************************************************

//PORTD.7  (Arduino pin 7)  Encoder 3 I Channel
//PORTD.6  (Arduino pin 6)  Encoder 3 Q Channel
//PORTB.0 (Arduino pin 8) Encoder 0 I channel
//PORTB.1 (Arduino pin 9) Encoder 0 Q channel
//PORTB.2 (Arduino pin 10) Encoder 1 I channel
//PORTB.3 (Arduino pin 11) Encoder 1 Q channel
//PORTB.4 (Arduino pin 12) Encoder 2 I channel
//PORTB.5 (Arduino pin 13) Encoder 2 Q channel


volatile int encref[4][4]=
{
  //  0  1  2  3
  { 
    0, 1,-1, 128          }
  ,//0
  {
    -1, 0, 128, 1          }
  ,//1
  { 
    1, 128, 0,-1          }
  ,//2
  { 
    128,-1, 1, 0          }// 3
};


void sendFloat(float f,unsigned t) {
  byte * b = (byte *) &f;
  Serial.write(t);
  Serial.write(b[0]);
  Serial.write(b[1]);
  Serial.write(b[2]);
  Serial.write(b[3]);
}

volatile long encoder[4]={
  0,0,0,0};							//-Encoder value
volatile unsigned char encoder_state=0;				//-State machine variables
volatile unsigned char encoder_input;                             //-+
volatile unsigned char DIV=0;
volatile unsigned char flag=0;
volatile int offset;
volatile unsigned int error=0;
SIGNAL(TIMER1_COMPA_vect)
{
  SET(PORTC,0);
  OCR1A+=800;										//16MHz/800=20KHz.
  //  OCR1A+=1600;										//16MHz/1600=10KHz.
  encoder_input=(PINB&0b00111111)|(PIND&0b11000000);						//-Encoder state machine

  offset=encref[(encoder_state   &0x03)][(encoder_input   &0x03)];// |
  if(offset==128)
    error++;
  else
    encoder[0]+=offset;

  offset=encref[(encoder_state>>2)&0x03][(encoder_input>>2)&0x03];// |
  if(offset==128)
    error++;
  else
    encoder[1]-=offset;

  offset=encref[(encoder_state>>4)&0x03][(encoder_input>>4)&0x03];// |
  if(offset==128)
    error++;
  else
    encoder[2]-=offset;

  offset=encref[(encoder_state>>6)&0x03][(encoder_input>>6)&0x03];
  if(offset==128)
    error++;
  else
      encoder[3]-=offset;

    encoder_state=encoder_input;                    //-+
  if(DIV==0)
  {
    DIV=200;
    flag=1;    
  }
  else
    DIV--;
  CLR(PORTC,0);
}

double A;
double B;
double C;
void setup()
{

  SET(DDRC,0);

  encoder[0]=(45.0/360)*2400.0;//45deg
  encoder[1]=(32.048/360.0)*2400.0;
  encoder[2]=(-113.174/360.0)*2400.0;

  TCCR1A=0x00;						//-Timer 1 inerrupt
  TCCR1B=0x01;                        // |
  TCCR1C=0x00;                         // |
  SET(TIMSK1,OCIE1A);                 // |
  sei();                              //-+

  TCCR0A=0;
  TCCR0B=0;


  Serial.begin(19200);

  DDRD=0b00000000;
  DDRB=0b00000000;
  PORTB=0b00111111;
  PORTD=0b11000000;

}

#define ARM 176.777

#define H0 (80.782-1.2)
//#define ARM 1767.77
//#define H0 (807.82-1.2)
void loop()
{


  if(flag)
  {
    double A=(double)encoder[1]*M_PI/1200.0;
    double B=(double)encoder[2]*M_PI/1200.0;
    double C=(double)encoder[0]*M_PI/1200.0;
    double D=(double)encoder[3]*M_PI/1200.0;

    double r=cos(A)*ARM+cos(A+B)*ARM;
    double h=sin(A)*ARM+sin(A+B)*ARM+H0;

    double y=r*cos(C)-125;
    double x=r*sin(C)-125;
    double z=h;

    flag=0;
    while(!flag)
      asm("nop");    
    sendFloat(x,'x');
    flag=0;
    while(!flag)
      asm("nop");    
    sendFloat(y,'y');
    flag=0;
    while(!flag)
      asm("nop");    
    sendFloat(z,'z');
    flag=0;
    while(!flag)
      asm("nop");    
    sendFloat(D,'a');
    //    sendFloat(error  ,'a');

  }
}





