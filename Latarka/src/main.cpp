#include <avr/io.h>
#include <util/delay.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "func.h"
#include <OneWire.h>
#include <DS18B20.h>
#include <RCSwitch.h>

// konfiguracja odbiornika RF433MHz
RCSwitch mySwitch = RCSwitch();

// Konfiguracja termometru cyfrowego DS18B20
byte address[8] = {0x28, 0xB2, 0x48, 0x75, 0xD0, 0x1, 0x3C, 0x49};
OneWire onewire(ONEWIRE_PIN);
DS18B20 sensors(&onewire);

// Konfiguracja Wyświetlacza LCD sterowanego magistralą I2C
LiquidCrystal_I2C lcd(0x27,16,2);

void setup(){
  init();

  sbi(DDRB, FAN);
  sbi(DDRB, LED);
  cbi(DDRD, SW);
  cbi(DDRD, OUTA);
  cbi(DDRD, OUTB);
  //pull-up
  sbi(PORTD, SW);
  sbi(PORTD, OUTA);
  sbi(PORTD, OUTB);

  pwm_ON();
  adc_start();
  start(lcd);

  lcd.init();
  lcd.backlight();
  sensors.begin();
  sensors.request(address);
  lcd.clear();
  mySwitch.enableReceive(0);
}

int main(){

  setup();

  int recived=0,rec_temp=0;
  int counter1=1;
  int counter2=0;
  int led_power=0;
  int loop_counter=0;
  uint8_t val=0, val_tmp=0, temp_st_1=0, temp_st_2=0, mode=0, sos_counter=1;
  val = encoder_read(); 
  bool change_adc=1;
  bool state=0;
  bool sos_state=1;

    if (sensors.available()){
      float temperature = sensors.readTemperature(address);
      lcd.setCursor(11,0);
      lcd.print(temperature);
      temp_st_1=temperature_read(temperature);
    }
//start(lcd);
//==================== main loop =====================//
    while(1){   
      
	   val_tmp = encoder_read(); 
	   if(val != val_tmp){
       // Prawo
		   if( (val==3 && val_tmp==1) || (val==0 && val_tmp==2) ) {
          if(state==0){
              if(led_power >= 255) led_power=255;
				      else led_power++;
          }          
          else{
            if(counter2 >= 3) counter2=3;
				    else counter2++;           
        }
		   }
       // Lewo
		   else if( (val==2 && val_tmp==0) || (val==1 && val_tmp==3) ) { 
        if(state==0){
            if(led_power <= 0) led_power=0;
				    else led_power--;          
        }
        else{
            if(counter2 <= 0) counter2=0;
				    else counter2--;
        }



		   }
		   val = val_tmp; 
	   } 


    if(bit_is_clear(PIND,SW)){
      _delay_ms(10);
      if(bit_is_clear(PIND,SW)){
      counter1++;
      lcd.setCursor(0,1);
      lcd.print("B:");
      lcd.print(counter1);
      }
      else if(bit_is_set(PIND,SW)){
        start(lcd);
        counter1=1;
      }
      //counter1=1;
    }

    if(counter1%125==0){
      state=!state;
      counter1=1;
    }


  if(state==0)
  {
    
//======================== start case 1

    
    if(mode==0){

      if(loop_counter%100==0)
      {

      start(lcd);
      lcd.setCursor(2,0);
      lcd.print(led_power);
      lcd.setCursor(2,1);
      lcd.print(mode);

      if (loop_counter%2000==0)
      {

        show_voltage(change_adc, lcd);
      //change_adc=!change_adc; //zmiana zródła adc

        float temperature = sensors.readTemperature(address);
        lcd.setCursor(11,0);
        lcd.print(temperature);

        temp_st_2=temperature_read(temperature);
        // chlodzenie z histereza
        if(temp_st_2==2){
          sbi(PORTB, FAN);
          }
        else if(temp_st_2 < temp_st_1){
          sbi(PORTB, FAN);
        }
        else if(temp_st_2==0){
          cbi(PORTB, FAN);
        }
        temp_st_1=temp_st_2;
      }
      sensors.request(address);
    }
     OCR1A = led_power;

    //tryb 2
    }
    if(mode==1){
        //classic strobo 25HZ
        if(loop_counter%40<=20){
          OCR1A = 0;
        }
        else{
          OCR1A = 200;
        }
    }
    //tryb 3
    if(mode==2){
        //var time strobo
        if(led_power==0){led_power=1;}
        if(recived==0){recived=1;}
        if(loop_counter%led_power<=rec_temp){
          OCR1A = 0;
        }
        else{
          OCR1A = 200;
        }
    }
    //tryb 4
    if(mode==3){
        //SOS
        uint8_t tmp_s=0;
        
        if(loop_counter%200==0){
          if(sos_counter<=6 || sos_counter>=19){
            if(sos_state==0){tmp_s=250;}
            if(sos_state==1){tmp_s=0;}
            OCR1A = tmp_s;
            sos_state=!sos_state;
          }
          if(sos_counter>=7 || sos_counter<=18){
            if(sos_state==0){tmp_s=250;}
            if(sos_state==1){tmp_s=0;}
            OCR1A = tmp_s;
            if(sos_counter%2==0){
              sos_state=!sos_state;
            }
          }
          if(sos_counter==24){sos_counter=0;}         
          sos_counter++;
        }
    }
  }
//======================== end case 1      
//======================== staet case 2    
    if(state==1){
      if(loop_counter%100==0){
        mode=counter2;
        menu(lcd, mode);
        OCR1A = 0;
      }
    
    }
//======================== end case 2     
if (mySwitch.available()) {
    recived=mySwitch.getReceivedValue();

    //normal
    if(mode==0){
    //++
    if(recived==100){
      if(led_power+1>255){led_power=255;}
      else led_power++;  
    }
    if(recived==200){
      if(led_power+25>255){led_power=255;}
      else led_power=led_power+25; 
    }
    if(recived==300){
      if(led_power+50>255){led_power=255;}
      else led_power=led_power+50; 
    }
    //--
    if(recived==-100){
      if(led_power-1<0){led_power=0;}
      else led_power=led_power-1; 
    }
    if(recived==-200){
      if(led_power-25<0){led_power=0;}
      else led_power=led_power-25; 
    }
    if(recived==-300){
      if(led_power-50<0){led_power=0;}
      else led_power=led_power-50; 
    }
    else{
    }
    recived=0;      
    }

    //vartime_strobo
    if(mode==2){
    //++
    if(recived==100){
      if(rec_temp+1>255){rec_temp=255;}
      else rec_temp++;  
    }
    if(recived==200){
      if(rec_temp+25>255){rec_temp=255;}
      else rec_temp=rec_temp+25; 
    }
    if(recived==300){
      if(rec_temp+50>255){rec_temp=255;}
      else rec_temp=rec_temp+50; 
    }
    //--
    if(recived==-100){
      if(rec_temp-1<0){rec_temp=0;}
      else rec_temp=rec_temp-1; 
    }
    if(recived==-200){
      if(rec_temp-25<0){rec_temp=0;}
      else rec_temp=rec_temp-25; 
    }
    if(recived==-300){
      if(rec_temp-50<0){rec_temp=0;}
      else rec_temp=rec_temp-50; 
    }
    else{
    }
    recived=0;      
    }

    mySwitch.resetAvailable();
  }
    loop_counter++;
    _delay_ms(1);

    }
  

}

