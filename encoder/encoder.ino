
//в таком вид матрица считает два фронта, переход с 00 в 01 и 10, и с 11 в 01 и 10
//точность 2х
//volatile int encref[4][4] = {
//  //0  1  2  3          
//  { 0,  -1, 1, 0 },      
//  { 0,  0,  0, 0 },      
//  { 0,  0,  0, 0 },      
//  { 0,  1, -1, 0 }       
//};

//в таком вид матрица считает четыре фронта, переход с 00 в 01 и 10, и с 11 в 01 и 10 и назад соотвественно
//точность 4х
volatile int encref[4][4] = {
  //0  1  2  3          
  {  0,  -1,  1,  0 },      
  {  1,   0,  0, -1 },      
  { -1,   0,  0,  1 },      
  {  0,   1, -1,  0 }       
};


volatile unsigned int encoder_input;
volatile unsigned int encoder_state; 
volatile int count = 0;

SIGNAL(TIMER1_COMPA_vect) {
  OCR1A+=800;                       //16MHz/800=20KHz.
  encoder_input = PINB;	
  count += encref[encoder_state&0x03][encoder_input&0x03];
  encoder_state = PINB;
}

void setup() {
  TCCR1A=0x00;              //-Timer 1 inerrupt
  TCCR1B=0x01;                        // |
  TCCR1C=0x00; 
  TIMSK1 |= 1 << OCIE1A;    // |
  sei();                    // enable interrupts
  
  DDRB= DDRB | 0x00;      //inputs port
  PORTB = 0xFF;           //set registr PINB in B11111111  
  encoder_state = PINB;
  
  Serial.begin(19200);
//  Serial.print("\x1B[?25l");                        // Выключение курсора
}


void loop() {
  
   //Чистим окно у клиента 
//  Serial.write(27);
//  Serial.print("[2J"); // clear screen
//  Serial.write(27);
//  Serial.print("[H"); // cursor to home
  
  Serial.println("===");
  Serial.println(PINB, BIN);
  Serial.println((encoder_input)&0x03, BIN);
  Serial.println(count);
}
