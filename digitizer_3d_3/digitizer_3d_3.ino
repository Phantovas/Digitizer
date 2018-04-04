//Macros
#define SET(x,y) (x |=(1<<y))               //Bit set
#define CLR(x,y) (x &= (~(1<<y)))       		//Bit clear
#define CHK(x,y) (x & (1<<y))           		//Bit check
#define TOG(x,y) (x ^= (1<<y))            	//x minus y

#define PULSE_ON_360 8000.0

//**************************************************************************
//	Hardware connections:
//**************************************************************************

//PORTB.0 (Arduino pin 8) Encoder 0 A channel
//PORTB.1 (Arduino pin 9) Encoder 0 B channel
//PORTB.2 (Arduino pin 10) Encoder 1 A channel
//PORTB.3 (Arduino pin 11) Encoder 1 B channel
//PORTB.4 (Arduino pin 12) Encoder 2 B channel
//PORTB.5 (Arduino pin 13) Encoder 2 A channel

#define poBTN_FIX = 2;

//подключаем антибребезг
#include <Bounce2.h>

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

//кнопка фиксации
Bounce btnFix;

void sendFloat(float f, unsigned t) {
  byte *b = (byte *) &f;
  Serial.write(t);
  Serial.write(b[0]);
  Serial.write(b[1]);
  Serial.write(b[2]);
  Serial.write(b[3]);

}
void sendFloat(float f, char* t) {
  Serial.print(t);
  Serial.print(f);
}


volatile long encoder[3] = { 0, 0, 0};				//-Encoder value
volatile unsigned char encoder_state=0;				//-State machine variables
volatile unsigned char encoder_input;                             //-+
volatile unsigned char DIV = 0;
volatile unsigned char flag = 0;

SIGNAL(TIMER1_COMPA_vect) {
  OCR1A+=800;										//16MHz/800=20KHz.
  
  encoder_input=PINB;						
  //крутимся против часовой угол уменьшается, подключение обратное: A-8; B-9
  encoder[0] += encref[(encoder_state&0x03)][(encoder_input&0x03)]; 
  // крутимся против часовой, угол уменьшается, подключение обратное: A-10; B-11 
  encoder[1] += encref[(encoder_state>>2)&0x03][(encoder_input>>2)&0x03];
  //крутимся по часовой, угол увеличивается, подключение прямое A-13; B-12
  encoder[2] += encref[(encoder_state>>4)&0x03][(encoder_input>>4)&0x03];
  encoder_state=encoder_input;   
  
//  if(DIV==0) {
//    DIV=200;
//    flag=1;    
//  } else
//    DIV--;
}

double A;
double B;
double C;

void setup() {
  
  //инициализируем пин закрытой очистки
  btnFix.attach(poBTN_FIX);
  btnFix.interval(10);
  
  //Расчет первоначальных углов в импульсах
  encoder[0]=(45.0/360.0)*PULSE_ON_360;       //45deg
  encoder[1]=(32.043/360.0)*PULSE_ON_360;
  encoder[2]=(-113.174/360.0)*PULSE_ON_360;
  
  //установка типа порта
  DDRB=0x00;
  PORTB=0xff;
  
  //прерывание по таймеру1
  TCCR1A=0x00;						
  TCCR1B=0x01;                        // |
  TCCR1C=0x00;                        // |
  SET(TIMSK1,OCIE1A);                 // |
  sei();                              // enable interrupts
  
  Serial.begin(19200);

}

#define ARM  176.777 //176.777
#define H0   80.72        //(80.782-1.2)
//#define ARM 1767.77
//#define H0 (807.82-1.2)

void loop() {
//  if(flag)
//  {
    A = (double)encoder[1] * 2*M_PI / PULSE_ON_360;
    B = (double)encoder[2] * 2*M_PI / PULSE_ON_360;
    C = (double)encoder[0] * 2*M_PI / PULSE_ON_360;

    double r = cos(A)* ARM + cos(A+B) * ARM;
    double h = sin(A) * ARM + sin(A+B) * ARM + H0;
  
    double y = r * cos(C) - 125;
    double x = r * sin(C) - 125;
    double z = h;

    //flag=0;
    //while(!flag)
    //  asm("nop");    //Each 'nop' statement executes in one machine cycle (at 16 MHz) yielding a 62.5 ns (nanosecond) delay.  
    sendFloat(x, 'x');
    //flag=0;
    //while(!flag)
    //  asm("nop");    
    sendFloat(y, 'y');
    //flag=0;
    //while(!flag)
    //  asm("nop");    
    sendFloat(z, 'z');
//  } else {
//    Serial.write("empty");
//  }
    Serial.println("$"); //$#13#10 окончание отправки
        
    
}

