//use EEPROM library for data storage
#include <EEPROM.h> 
//use LCD library
#include <LiquidCrystal.h>

//define digital pins
#define incLimit 0 //interriupt 0, pin 2
#define decLimit 1 //interrupt 1, pin 3
#define pulseInput 4 //input pin for pulse, pin 4
#define buzzer 5 //output pin for buzzer, pin 5
#define contrastAdjustment 13 //pin for LCD Ro, pin 13

#define rNumber "09059274020" //receiver's number

/*****Function prototype*****/
void initPorts();
void sendSMS(String, String);
void increaseWattageLimit();
void decreaseWattageLimit();
void countPulse();
void updateLimitandCount();

/*****Global Variables*******/
float wattageLimit = 0.00; //temp storage for current wattage
float pulseCount = 0.00; //temp storage for current pulse count
float currWattage = 0.00;

//instantiate LCD library
LiquidCrystal lcd(12, 11, 10, 9, 8, 7, 6);

//required function setup for initialization
void setup(){
  initPorts(); //call initPorts function
}

//required function countPulse
void loop(){
  countPulse(); //call countPulse repeateadly
}

void initPorts(){
  Serial.begin(19200); //initialize baud rate of GSM
  delay(1000); //wait 50ms for initialization to finish
  Serial.println("AT+CMGF=1\r\n"); //send command to GSM to enable all functions
  delay(50);
  pinMode(incLimit, INPUT); //declare incLimit pin as input
  pinMode(decLimit, INPUT);  //declare decLimit pin as input
  pinMode(pulseInput, INPUT); //declare pulseInput pin as input
  pinMode(buzzer, OUTPUT); //declare buzzer pin as input
  pinMode(contrastAdjustment, OUTPUT); //declare contrastAdjustment pin as input
  analogWrite(contrastAdjustment, 80); //output analogvalue of 80 to digital pin
  digitalWrite(buzzer, LOW); //turn buzzer pin off
  attachInterrupt(0, increaseWattageLimit, RISING); //attach interrupt to int0, set ISR to increaseWattageLimit function, and interrupt on PGT
  attachInterrupt(1, decreaseWattageLimit, RISING); //attach interrupt to int1, set ISR to decreaseWattageLimit function, and interrupt on PGT
  wattageLimit = EEPROM.read(0); //read memory location 0x00 and assign to wattageLimit to get previous value of wattage Limit
  pulseCount = EEPROM.read(1); //read memory location 0x01 and assign to pulseCount to get previous value of pulse
  currWattage = EEPROM.read(2); //read memory location 0x02 and assign to current Wattage to get previous value of current Wattage
  lcd.begin(16,2); //declare that LCD is 16x2
  lcd.print("Program Start"); //output Program Start in LCD
  delay(1000); //wait for 1 sec
  updateLimitandCount(); //call updateLimitandCount
}

void sendSMS(String message, String number){
  //send command to send SMS
  Serial.println("AT+CMGS=\"" + number + "\"\r\n");
  delay(500); //wait for 500 ms for GSM to process command
  Serial.println(message); // send sms to gsm
  delay(500);  //wait for 500 ms for GSM to process command
  Serial.println(char(26)); //send ctrl+z to send sms
  delay(500);
}

void increaseWattageLimit(){
  wattageLimit += 1.00; //increase wattage limit
  if(wattageLimit >= 65000){
    wattageLimit = 65000;
  }
  EEPROM.write(0, wattageLimit); //regiter in memory location 0x00
  updateLimitandCount(); //update LCD
  delay(75); //wait 75ms to prevent an increase of more than 1
}

//same functionality as increaseWattageLimit, except this one decreases
void decreaseWattageLimit(){
  wattageLimit -= 1.00; 
  if(wattageLimit <= 0){
    wattageLimit = 0;
  }
  EEPROM.write(0, wattageLimit);
  updateLimitandCount();
  delay(75);
}

//main function for counting pulses and converting to wattage
void countPulse(){
  int temp; //create temporary variable
  //while pulse count is less than or equal to wattage limit
  //execute block
  while(currWattage <= wattageLimit){ 
    //check pulseInput pin's current value 
    temp = digitalRead(pulseInput);
    if(temp == true){ //if pulseInput pin is high execute block
       while(digitalRead(pulseInput) == true){ 
          delay(75); //white until pulseInput pin is low
       }
    }
    pulseCount++;//inc pulse count
    EEPROM.write(1,pulseCount); //store value of pulseCount in memory location 0x01
    currWattage = pulseCount/1.6;
    EEPROM.write(2,currWattage);
    delay(75); //wait for 75 ms
    updateLimitandCount(); //update values in LCD
  }
  //at this point, wattageLimit has already been reached
  digitalWrite(buzzer, HIGH);//turn on buzzer  
  delay(5000); //for 5 sec
  digitalWrite(buzzer, LOW); //turn off buzzer
  sendSMS("Wattage Limit has been reached", rNumber); //send message to registered number
  pulseCount = 0; //reset pulse count
  currWattage = 0; //reset current Wattage
  EEPROM.write(1,pulseCount); //save pulseCount to 0x01
  EEPROM.write(2,currWattage); //save pulseCount to 0x02
  delay(75);
  updateLimitandCount(); //update LCD
  
}

void updateLimitandCount(){
  lcd.clear(); //clear LCD
  lcd.setCursor(0,0); //move pointer to column 0 row 0
  lcd.print("Limit: " + String(wattageLimit)); //output wattageLimit
  lcd.setCursor(0,1); //move pointer to column 0 row 1
  lcd.print("W: " + String(currWattage) + " Wh"); //output pilseCount
}
