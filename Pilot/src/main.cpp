#include <Arduino.h>
#include <avr/io.h>
#include <RCSwitch.h>

#ifndef _BV
#define _BV(bit)        (1<<(bit))
#endif

#ifndef sbi
#define sbi(reg,bit)    reg |= (_BV(bit))
#endif
 
#ifndef cbi
#define cbi(reg,bit)    reg &= ~(_BV(bit))
#endif

#ifndef fbi
#define fbi(reg,bit)    reg ^= (_BV(bit))
#endif

#define B_MODE PD2
#define B_PLUS PD3
#define B_MINUS PD4
#define LED PB5

  int val=0, vp=100, vm=-100;

RCSwitch mySwitch = RCSwitch();

void setup() {

  //zdefiniowanie portów
  cbi(DDRD, B_MODE);
  cbi(DDRD, B_PLUS);
  cbi(DDRD, B_MINUS);
  sbi(DDRB, LED);

  //pull up
  sbi(PORTD, B_MODE);
  sbi(PORTD, B_PLUS);
  sbi(PORTD, B_MINUS);

  cbi(PORTB, LED);

  //konfiguracja nadajnika
  mySwitch.enableTransmit(8);
  mySwitch.setRepeatTransmit(4);
  
}

int main(){
  
  init();
  setup();

  //pętla główna
  while(true){

  //obsługa przycisku wyboru trybu
  if(bit_is_clear(PIND, B_MODE)){
    val++;
    if(val>2){val=0;}
    _delay_ms(10);
    while(bit_is_clear(PIND, B_MODE));
  }

  //dioda informująca o ostatnim trybie
  if(val==2){
   sbi(PORTB, LED);
  }
  else{
   cbi(PORTB, LED);
  }

  //obsługa przycisku +
  if(bit_is_clear(PIND, B_PLUS)){
    if(val==0){vp=100;}
    if(val==1){
      vp=200;
      _delay_ms(10);
      while(bit_is_clear(PIND, B_PLUS));
      }
    if(val==2){
      vp=300;
      _delay_ms(10);
      while(bit_is_clear(PIND, B_PLUS));
      }
    mySwitch.send(vp, 24);
  }

  //obsługa przycisku -
  if(bit_is_clear(PIND, B_MINUS)){
    if(val==0){vm=-100;}
    if(val==1){
      vm=-200;
      _delay_ms(10);
      while(bit_is_clear(PIND, B_MINUS));
      }
    if(val==2){
      vm=-300;
      _delay_ms(10);
      while(bit_is_clear(PIND, B_MINUS));
      }
    mySwitch.send(vm, 24);
  }

  _delay_ms(20);   

  }
}
