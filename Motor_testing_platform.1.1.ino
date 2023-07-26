/* 
Software for mobile platform for testing electric motors
  version 1.1 dated 21.07.15
*/
int pinReverse=13;
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean t=true;
unsigned long times;
int R1=104060;
int R2=9630;
int R3=107000;
int R4=9570;
//float VCoeff1=0.084704;   // Measured voltage to voltage at analog input R1/(R1+R2)
//float VCoeff2=0.082097;   
float VCoeff2=17.18343;	//17.28653;   // ADC steps in one measured volt
float VCoeff1=15.99393;	//16.65448;//16.75441;  

						//0.04 Measured current to analog input voltage, 1024 ADC steps at 5 Volts

float ACoeff1=8.163265;       //ADC steps in one measured ampere
float ACoeff2=8.163265;
int input;
int timeint=200;

long Distance;
long PrevDistance;
float Speed;

const int pinFoto=A0;

int FotoFreqInterval=1;
long FotoTimer;
int FotoCount;
int currFoto;
int prevFoto;
const int BlackWhiteDiff=600;
void FotoChange(int state)
{
  
  currFoto=state;
  //Serial.println((currFoto));
  
  if (abs(prevFoto-currFoto)>BlackWhiteDiff)
  {
    //Serial.print ("line -");
    //Serial.println((prevFoto-currFoto));
    FotoCount++;
  }
  prevFoto=currFoto;
}

class SensorMotor
{
  public:
int pinPWM;
//byte pin2;
int pinEN;

int pinEnc;
int pinI;
int pinU;

int EncPerRotation;
int EncCount;
int EncState;
int EncPrevState;
int prevEncCount;

public:
int p; // Мощность двигателя 1
float s; // перемещение двигателя 1
float v; // скорость двигателя 1
float i; // ток двигателя 1
float u; // напряжение двигателя 1
SensorMotor(byte pPwm, byte pEn,byte pEnc, byte pI,byte pU)
{

  pinPWM=pPwm;
  pinEN=pEn;

  pinEnc=pEnc;
  pinI=pI;
  pinU=pU;
  
  EncPerRotation=20;
  EncCount=0;
  prevEncCount=0;
  prevFoto=false;
  currFoto=false;
  FotoCount=0; 
}
void SetSpeed(int NewSpeed)
{
  p=NewSpeed;
  int motorE=abs((int)NewSpeed*2.55);
  if (NewSpeed==0)   // Выбег
  {
  analogWrite(pinPWM,0);
  digitalWrite(pinEN,LOW);
  }
  else
  if (NewSpeed>=0)    // one way rotation
  {
  analogWrite(pinPWM,motorE);
  digitalWrite(pinEN,HIGH);
  }
  else  // reverse rotation
  {
  analogWrite(pinPWM,motorE);
  digitalWrite(pinEN,LOW);
  }

}


};

SensorMotor M1(5,4,2,2,4);
SensorMotor M2(6,7,3,3,5);
void Encoder1Change()	// 1st Encoder change
{
  M1.EncState=digitalRead(M1.pinEnc);
  if (M1.EncState!=M1.EncPrevState)
  {
  M1.EncCount++;
    //Serial.print("m1 encoder: ");
    //Serial.println(M1.EncCount);
    M1.EncPrevState=M1.EncState;
  }
  
  
}
void Encoder2Change()	// 2nd Encoder change
{
  M2.EncState=digitalRead(M2.pinEnc);
  if (M2.EncState!=M2.EncPrevState)
  {
  M2.EncCount++;
    //Serial.print("m2 encoder: ");
    //Serial.println(M2.EncCount);
    M2.EncPrevState=M2.EncState;
  }
}

void SetBeginingState()	// Set parameters by default
{
  t=false;
  times=0;
  FotoCount=0;
  Distance=0;
  Speed=0;
  M1.EncCount=0;
  M1.prevEncCount=0;
  M1.p=0;
  M1.s=0;
  M1.v=0;
  M1.i=0;
  M1.u=0;
  M2.EncCount=0;
  M2.prevEncCount=0;
  M2.p=0;
  M2.s=0;
  M2.v=0;
  M2.i=0;
  M2.u=0;
  
}
void(* resetFunc) (void) = 0;			// Reset controller

boolean reverse;

void setup() {

   pinMode(pinReverse,OUTPUT);
   pinMode(pinFoto,INPUT);


   
   pinMode(M1.pinPWM,OUTPUT);
   pinMode(M1.pinEN,OUTPUT);

   pinMode(M2.pinPWM,OUTPUT);
   pinMode(M2.pinEN,OUTPUT);

   pinMode(pinReverse,OUTPUT);
   pinMode(2,INPUT);
   pinMode(3,INPUT);
   
   attachInterrupt(0, Encoder1Change,CHANGE);
   attachInterrupt(1, Encoder2Change,CHANGE);
  Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }  

    SetBeginingState();
}

int encm1;
int encm1old;
unsigned long enctimer;
void loop() {

 
  if ((millis()-FotoTimer)>FotoFreqInterval)
 {     
 FotoChange(analogRead(pinFoto)); 
 FotoTimer=millis();
 }
 

 if (t)
 if ((millis()-times)>timeint)
 {
    M1.i=((float)analogRead(M1.pinI)-510)/ACoeff1-0.16;
    M1.u=(float)analogRead(M1.pinU)/VCoeff1;
    M2.i=((float)analogRead(M2.pinI)-510)/ACoeff2;
    M2.u=(float)analogRead(M2.pinU)/VCoeff2;
    
    Distance+=FotoCount;
    FotoCount=0;
    Speed=((float)(Distance-PrevDistance))/(float)(timeint*0.001);
    PrevDistance=Distance;
    if (M1.p>=0)
    {
    M1.s+=(float)(M1.EncCount-M1.prevEncCount)/(float)M1.EncPerRotation;
    M1.v=((float)((M1.EncCount-M1.prevEncCount)/(float)M1.EncPerRotation)/(float)(timeint*0.001))*60;
    }
    else
    {
        M1.s-=(float)(M1.EncCount-M1.prevEncCount)/(float)M1.EncPerRotation;
        M1.v=-((float)((M1.EncCount-M1.prevEncCount)/(float)M1.EncPerRotation)/(float)(timeint*0.001))*60;
    }
   // M1.EncCount=0;
   M1.prevEncCount=M1.EncCount;
       if (M2.p>=0)
       {
    
    M2.s+=(float)(M2.EncCount-M2.prevEncCount)/(float)M2.EncPerRotation;
    M2.v=((float)((M2.EncCount-M2.prevEncCount)/(float)M2.EncPerRotation)/(float)(timeint*0.001))*60;
       }
       else
 {
     M2.s-=(float)(M2.EncCount-M2.prevEncCount)/(float)M2.EncPerRotation;
    M2.v=-((float)((M2.EncCount-M2.prevEncCount)/(float)M2.EncPerRotation)/(float)(timeint*0.001))*60;
 }
   // M1.EncCount=0;
    M2.prevEncCount=M2.EncCount;
    
    if (reverse) 
      digitalWrite(pinReverse,HIGH); 
    else
      digitalWrite(pinReverse,LOW); 
 
    times=millis();
    //Serial.print("t=");
    Serial.print(times);
    Serial.print(",");
    //Serial.print("d=");
    Serial.print(Distance);
    Serial.print(",");
    //Serial.print("s=");
    Serial.print(Speed);
    Serial.print(",");
    //Serial.print("r=");
    Serial.print(reverse);
    Serial.print(",");
    //Serial.print("p1=");
    Serial.print(M1.p);
    Serial.print(",");
    //Serial.print("s1=");
    Serial.print(M1.s);
    Serial.print(",");
    //Serial.print("v1=");
    Serial.print(M1.v);
    Serial.print(",");
    //Serial.print("i1=");
    Serial.print(M1.i);
    Serial.print(",");
    //Serial.print("u1=");
    Serial.print(M1.u);
    Serial.print(",");
    //Serial.print("p2=");
    Serial.print(M2.p);
    Serial.print(",");
    //Serial.print("s2=");
    Serial.print(M2.s);
    Serial.print(",");
    //Serial.print("v2=");
    Serial.print(M2.v);
    Serial.print(",");
    //Serial.print("i2=");
    Serial.print(M2.i);
    Serial.print(",");
    //Serial.print("u2=");
    Serial.print(M2.u);
    Serial.println(";");

 }
    
  //serialEvent(); //call the function
  // print the string when a newline arrives:
 /* if (stringComplete) {
    Serial.print("command:");
    Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }*/
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void RunCommand(String command)
{
  if ((command.substring(0,3)).equals("ti="))
  {
    timeint=command.substring(3).toInt();
    Serial.print("Set timeint=");
    Serial.println(timeint);
    
  } else
    if ((command.substring(0,3)).equals("p1="))
  {
    M1.p=command.substring(3).toInt();
    M1.SetSpeed(M1.p);
    //Serial.print("Set p1=");
   // Serial.println(M1.p);
    
  } else
    if ((command.substring(0,3)).equals("p2="))
  {
    M2.p=command.substring(3).toInt();
    M2.SetSpeed(M2.p);
    //Serial.print("command: set p2=");
   // Serial.println(M2.p);
    
  } else
  if ((command.substring(0,2)).equals("r="))
  {
    reverse=command.substring(2).toInt();
    digitalWrite(pinReverse,(reverse)?HIGH:LOW);
    //Serial.print("command: set reverse=");
    //Serial.println(reverse);
  } else
  if (command.equals("run"))
  {
    //int val=command.substring(3).toInt();
    //Serial.println("command: set t=true;");
    t=true;
  }else
  if (command.equals("stop"))
  {
    //int val=command.substring(3).toInt();
    //Serial.println("command: set t=false;");
    t=false;
  }
  if (command.equals("reset"))
  {
    //int val=command.substring(3).toInt();
    //Serial.println("command: set t=false;");
    SetBeginingState();
    resetFunc();
    
  }

}


void serialEvent() {
   while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    if (inChar == ';') {
    //stringComplete = true;
    //Serial.print("command: ");
    //Serial.println(inputString);
    RunCommand(inputString);
    inputString="";
    }
    else
    {
    inputString += inChar;
    }
  }
  /*
  while (Serial.available()) 
  {
    char inChar = (char)Serial.read();
    Serial.print("command: ");
    Serial.println(inChar);
    times+=2000;  
    t=!t;
  }*/
}


