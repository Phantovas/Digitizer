#define SET(x,y) (x |=(1<<y))					//-Bit set/clear macros
#define CLR(x,y) (x &= (~(1<<y)))       		// |
#define CHK(x,y) (x & (1<<y))           		// |
#define TOG(x,y) (x^=(1<<y))            		//-+

//**************************************************************************
//	Hardware connections:
//**************************************************************************

//PORTB.0 (Arduino pin 8) Encoder 0 I channel
//PORTB.1 (Arduino pin 9) Encoder 0 Q channel
//PORTB.2 (Arduino pin 10) Encoder 1 I channel
//PORTB.3 (Arduino pin 11) Encoder 1 Q channel
//PORTB.4 (Arduino pin 12) Encoder 2 I channel
//PORTB.5 (Arduino pin 13) Encoder 2 Q channel

volatile int encref[4][4]=
{
  //    0  1  2  3
  { 0, 1,-1, 0  },
  {-1, 0, 0, 1  },
  { 1, 1, 0,-1  },
  { 0,-1, 1, 0  }
};
void sendFloat(float f,unsigned t) {
  byte * b = (byte *) &f;
  Serial.write(t);
  Serial.write(b[0]);
  Serial.write(b[1]);
  Serial.write(b[2]);
  Serial.write(b[3]);
}

volatile long encoder[3]={0,0,0};							//-Encoder value
volatile unsigned char encoder_state=0;				//-State machine variables
volatile unsigned char encoder_input;                             //-+
volatile unsigned char DIV=0;
volatile unsigned char flag=0;

SIGNAL(TIMER1_COMPA_vect)
{
  OCR1A+=800;										//16MHz/800=20KHz.
  encoder_input=PINB;						//-Encoder state machine
  encoder[0]+=encref[(encoder_state&0x03)][(encoder_input&0x03)];// |
  encoder[1]-=encref[(encoder_state>>2)&0x03][(encoder_input>>2)&0x03];// |
  encoder[2]-=encref[(encoder_state>>4)&0x03][(encoder_input>>4)&0x03];// |
  encoder_state=encoder_input;                    //-+
  if(DIV==0)
  {
    DIV=200;
    flag=1;    
  }
  else
    DIV--;
}

double A;
double B;
double C;
void setup()
{
/*  encoder[0]=(45.0/360)*2400.0;//45deg
  encoder[1]=(32.048/360.0)*2400.0;
  encoder[2]=(-113.374/360.0)*2400.0;
*/

  encoder[0]=(45.0/360)*2400.0;//45deg
  encoder[1]=(32.043/360.0)*2400.0;
  encoder[2]=(-113.174/360.0)*2400.0;
  
  TCCR1A=0x00;						//-Timer 1 inerrupt
  TCCR1B=0x01;                        // |
  TCCR1C=0x00;                        // |
  SET(TIMSK1,OCIE1A);                 // |
  sei();                              //-+
  Serial.begin(19200);
  DDRB=0x00;
  PORTB=0xff;
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
  }
}

