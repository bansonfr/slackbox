// ============================================
//  ____  _            _    ____            
// / ___|| | __ _  ___| | _| __ )  _____  __
// \___ \| |/ _` |/ __| |/ /  _ \ / _ \ \/ /
//  ___) | | (_| | (__|   <| |_) | (_) >  < 
// |____/|_|\__,_|\___|_|\_\____/ \___/_/\_\
// ============================================
// Keyboard stroke scheduler & client
// V0.8 - Beta 
// By banson - www.banson.fr
// ============================================

// library required : https://www.pjrc.com/teensy/td_libs_TimerOne.html
#include <TimerOne.h>

void slack_init() {
// ===================================================
// Spell Scheduler Configuration
// ===================================================
// use slack_add() to add keystrokes to the scheduler
//
// Syntax: slack_add( Priority , Cooldown , Global Cooldown, Cast time, Key)
// /!\ IMPORTANT : Time is ALWAYS expressed in 1/10th of seconds, i.e. 10 means 1 second, 50 stands for 5 seconds, and 600 stands for 1 minute, etc...
//
// Priority - Spell cast priority : 
// Spell with highest Priority goes first.
//
// Cooldown - Spell cooldown time :
// Defines the period (cooldown) to cast spell in 1/10th of seconds. A value of 0 makes spell always available, 
// useful to define a spam key. However, spam key priority should be low, otherwise it would mask other spells.
//
// Global Cooldown - Spell global cooldown time :
// If spell triggers global cooldown, specifies by how much in 1/10th of seconds. A value of 0 means that spell
// is off cooldown, and next spell will be instantly cast.
//
// Cast Time - Spell cast Time:
// Specifies spell cast time in 1/10th of seconds. Cast time is added to spell global coodown value, use either one or the other 
// depending on the context, but using both may double cast time.
//
// Key - Key to press :
// used to define the key to press associated to the spell being configured.
//
// Example 1 : An instant spell with 6 seconds cooldown, triggering 1 second GCD, a priority of 2, using the key 'g', is written:
//      slack_add( 2, 60, 10, 0, 'g');
// Example 2 : A 2.3 second cast spell with 14 seconds cooldown, triggering no GCD, a priority of 1, using the key 'f', is written:
//      slack_add( 1, 140, 0, 23, 'f');
//
// --- Add/modify your spells Here: ---
  slack_add(5,600,10,0,'1');
  slack_add(3,90,10,0,'3');
  slack_add(4,200,0,60,'6');  
  slack_add(2,0,0,15,'2');  
//
//
// ===================================================
// End Spell Scheduler Configuration
// ===================================================
}

// Interfaces
//===========
#define  LED1   9          // LED PIN
#define  LED2  10          // LED PIN
#define  BTN   13          // LED PIN

// Slack Engine Defines
//=====================
// Event Parameters
#define  CONF  0  // Priority, 0 means inactive
#define  CD  1    // envent cooldown
#define  SCD  2   // event spell cooldown
#define  GCD  3   // GCD after event activation, 0 equals no GCD triggering
#define  KEY  4   // Key to be pressed
#define  CAST  5 // Cast Time

// Slack Spell DB Size
//====================
#define  EVENT_SIZE    6
#define  EVENT_NUMBER  16
volatile unsigned int slack[EVENT_NUMBER][EVENT_SIZE];


// Slack Engine Variables
//=======================
unsigned char stateled=0;
unsigned char event = 0;
unsigned char serialkey;

volatile unsigned char gcd = 0;
volatile boolean rdy = false;
volatile boolean running = false;

// Setup
//======
void setup() {
  Keyboard.begin();
  
  pinMode(LED1, OUTPUT);     
  pinMode(LED2, OUTPUT);     
  pinMode(BTN,  INPUT);

  // Powering the Button
  pinMode(8, OUTPUT);    
  digitalWrite(8,HIGH);

  // Buttons & LED BIT
  digitalWrite(LED1,HIGH);
  digitalWrite(LED2,LOW);
  delay(250);
  digitalWrite(LED1,LOW);
  digitalWrite(LED2,HIGH);
  delay(250);
  digitalWrite(LED1,LOW);
  digitalWrite(LED2,LOW);

  // Slack init
  slack_init();

  // Timer Init - 100ms period
  Timer1.initialize(100000);
  Timer1.attachInterrupt(SlackInterrupt);  
  
  Serial.begin(9600);
}

//
void loop() {
  if (Serial.available()) {
    serialkey=Serial.read();
    Serial.write("Sending key:");
    Serial.write(serialkey);
    Serial.write("\n");
    Keyboard.write(serialkey);
    }
  if (running) digitalWrite(LED2,HIGH); 
  else digitalWrite(LED2,LOW);
  if (gcd==0){
    event = slack_seek();
    if (event<EVENT_NUMBER) {
      if(running) {
        Keyboard.write(char(slack[event][KEY]));
        digitalWrite(LED1,HIGH);
        slack[event][CD]=slack[event][SCD];
        gcd+=slack[event][GCD]+slack[event][CAST];
      }
    }
  } 
}

// Slack engine core functions below
// =================================

void SlackInterrupt() 
{
  if (gcd>0) gcd--;
  slack_refresh();
  digitalWrite(LED1,LOW);
  if(!digitalRead(BTN)&&running==false) running=true; 
  else {
    if(!digitalRead(BTN)&&running==true) running=false;
  }
}

void slack_refresh(void) {
  for (unsigned char i=0; i<EVENT_NUMBER; i++) {
    if (slack[i][CONF]>0){
      if (slack[i][CD]>0) slack[i][CD]--;
    }
  }
}

unsigned char slack_seek() {
  unsigned char j = 255;
  unsigned char prio = 0;
  for (unsigned char i=0; i<EVENT_NUMBER; i++) {
    if (slack[i][CONF]>prio) {
      if (slack[i][CD]==0) {
        j=i;            
        prio=slack[i][CONF];
      }
    }
  }
  return j;
}

unsigned char slack_add(unsigned char aconf, unsigned int ascd, unsigned char acast, unsigned char agcd, unsigned char akey) {
  unsigned char i=0;
  unsigned char j=255;

  while(i<EVENT_NUMBER) {
    if (slack[i][CONF]==0) j=i;
    i++;
  }

  if (j!=255) {
    slack[j][CONF]=aconf;
    slack[j][CD]=0;
    slack[j][SCD]=ascd;
    slack[j][GCD]=agcd;  
    slack[j][KEY]=akey;  
    slack[j][CAST]=acast;     
    return 1;
  } 
  else return 0;
}




