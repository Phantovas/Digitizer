#define SET(x,y) (x |=(1<<y))					//-Bit set/clear macros
#define CLR(x,y) (x &= (~(1<<y)))       		// |
#define CHK(x,y) (x & (1<<y))           		// |
#define TOG(x,y) (x^=(1<<y))            		//-+

#define PULSE_ON_360 8000.0

//**************************************************************************
//	Hardware connections:
//**************************************************************************

//PORTB.0 (Arduino pin 8) Encoder 0 I channel
//PORTB.1 (Arduino pin 9) Encoder 0 Q channel
//PORTB.2 (Arduino pin 10) Encoder 1 I channel
//PORTB.3 (Arduino pin 11) Encoder 1 Q channel
//PORTB.4 (Arduino pin 12) Encoder 2 I channel
//PORTB.5 (Arduino pin 13) Encoder 2 Q channel

//в таком вид матрица считает четыре фронта, переход с 00 в 01 и 10, и с 11 в 01 и 10 и назад соотвественно
//точность 4х
volatile int encref[4][4] = {
  {  0,  -1,  1,  0 },      
  {  1,   0,  0, -1 },      
  { -1,   0,  0,  1 },      
  {  0,   1, -1,  0 }       
};
//старая матрица, почему то есть лишняя 1 [2;1]
//volatile int encref[4][4]=
//{
//  { 0, 1,-1, 0  },
//  {-1, 0, 0, 1  },
//  { 1, 1, 0,-1  },
//  { 0,-1, 1, 0  }
//};

void sendFloat(float f,unsigned t) {
  byte * b = (byte *) &f;
  Serial.write(t);
  Serial.write(b[0]);
  Serial.write(b[1]);
  Serial.write(b[2]);
  Serial.write(b[3]);

}
void sendFloat(float f, char* t) {
  Serial.print(t);
  Serial.print(" ");
  Serial.println(f);
}


volatile long encoder[3]={0,0,0};							//-Encoder value
volatile unsigned char encoder_state=0;				//-State machine variables
volatile unsigned char encoder_input;                             //-+
volatile unsigned char DIV=0;
volatile unsigned char flag=0;

SIGNAL(TIMER1_COMPA_vect) {
  OCR1A+=800;										//16MHz/800=20KHz.
  
  encoder_input=PINB;						//-Encoder state machine
  //крутимся против часовой угол уменьшается, подключение обратное: A-8; B-9
  encoder[0] += encref[(encoder_state&0x03)][(encoder_input&0x03)]; 
  // крутимся против часовой, угол уменьшается, подключение обратное: A-10; B-11 
  encoder[1] += encref[(encoder_state>>2)&0x03][(encoder_input>>2)&0x03];
  //крутимся по часовой, угол увеличивается, подключение прямое A-13; B-12
  encoder[2] += encref[(encoder_state>>4)&0x03][(encoder_input>>4)&0x03];
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
void setup() {
  
/*  encoder[0]=(45.0/360)*2400.0;//45deg
  encoder[1]=(32.048/360.0)*2400.0;
  encoder[2]=(-113.374/360.0)*2400.0;
*/

  /*
   * Расчет первоначальных углов в импульсах
   */
  encoder[0]=(45.0/360.0)*PULSE_ON_360;       //45deg
  encoder[1]=(32.043/360.0)*PULSE_ON_360;
  encoder[2]=(-113.174/360.0)*PULSE_ON_360;
  
  TCCR1A=0x00;						//-Timer 1 inerrupt
  TCCR1B=0x01;                        // |
  TCCR1C=0x00;                        // |
  SET(TIMSK1,OCIE1A);                 // |
  sei();                 // enable interrupts
  Serial.begin(19200);
  DDRB=0x00;
  PORTB=0xff;
}

#define ARM  176.777 //176.777
#define H0 (80.782-1.2)
//#define ARM 1767.77
//#define H0 (807.82-1.2)

void loop() {
  if(flag)
  {
    double A=(double)encoder[1]*2*M_PI/PULSE_ON_360;
    double B=(double)encoder[2]*2*M_PI/PULSE_ON_360;
    double C=(double)encoder[0]*2*M_PI/PULSE_ON_360;

    double r=cos(A)*ARM+cos(A+B)*ARM;
    double h=sin(A)*ARM+sin(A+B)*ARM+H0;

    double y=r*cos(C)-125;
    double x=r*sin(C)-125;
    double z=h;

    flag=0;
    while(!flag)
      asm("nop");    //Each 'nop' statement executes in one machine cycle (at 16 MHz) yielding a 62.5 ns (nanosecond) delay.  
    sendFloat(encoder[0], "enc 0");
    sendFloat(C, "rad 0");
    sendFloat(x, "x");
    flag=0;
    while(!flag)
      asm("nop");    
    sendFloat(encoder[1],"enc 1");
    sendFloat(A, "rad 1");
    sendFloat(y, "y");
    flag=0;
    while(!flag)
      asm("nop");    
    sendFloat(encoder[2],"enc 2");
    sendFloat(B, "rad 1");
    sendFloat(z, "z");
    //end block
    Serial.println("-----");
  } else {
    Serial.write("empty");
  }
}

