#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "func.h"

char* modes[] = {"1.normal", "2.strobo", "3.varTime_strobo", "4.S.O.S."};

//główny ekran
void start(LiquidCrystal_I2C lcd){
    lcd.setCursor(0,0);
    lcd.print("V:       T:");
    lcd.setCursor(0,1);
    lcd.print("M:    ");
}

//ekran menu wyboru trybu
void menu(LiquidCrystal_I2C lcd, uint8_t mode){
    lcd.setCursor(0,0);
    lcd.print("MODE:           ");
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print(modes[mode]);
}

//wlasna funkcja sleep do przyjmowania zmiennej jako argumentu
void sleep(uint16_t time){
while(time>0){
_delay_ms(1);
time--;
}
}

//załącznie PWM na PB1
void pwm_ON(){
// 10bit Fast PWM
TCCR1A |= (1<<(WGM10))|(1<<(WGM11));
TCCR1B |= (1<<(WGM12));
// PWM SET ON PB1 NON-INV
TCCR1A |= (1<<(COM1A1));
//prescaller: 64
TCCR1B |= (1<<(CS11))|(1<<(CS10));
OCR1A = 1;
}

//wyłącznie funkcji pwm
void pwm_OFF(){
TCCR1A = 0;
TCCR1B = 0;
}

//załączenie 10-bit ADC
void adc_start(){

    ADMUX &= ~(1<<REFS1);
    ADMUX |= (1<<REFS0) | (1<<ADLAR);
    //enable ADC
    ADCSRA |= (1<<ADEN);
}

//odczyt napięcia z podanego kanału
uint16_t read_voltage(uint8_t channel){
    ADMUX = (ADMUX & 0xf0) | channel;
    ADCSRA |= (1<<ADSC); // start conversion
    while(ADCSRA & (1<<ADSC));

    uint16_t tmp = ADCL>>6;
    uint16_t conv_result = ADCH<<2 | tmp;

    return conv_result;
}

uint16_t map(uint16_t V_in, uint16_t MIN_in, uint16_t MAX_in, uint16_t MIN_out, uint16_t MAX_out)
{
    return ((((V_in - MIN_in)*(MAX_out - MIN_out))/(MAX_in - MIN_in)) + MIN_out);
}

//pokazanie napiecia na wyswietlaczu
//MAX adc=942~25,5V~4,6V na dzielniku
//MIN adc=665~18V~3,246V na dzielniku
void show_voltage(uint8_t channel, LiquidCrystal_I2C lcd){

    uint16_t sum=0, adc_read=0;
    for(int i=0; i<3; i++){
        adc_read=read_voltage(channel);
        sum=sum+adc_read;
    }

    adc_read=sum/3;
   
    float voltage_value=adc_read;
    float real_voltage;
    lcd.setCursor(6,1);
    real_voltage = (voltage_value*5)/1024;

    if(channel==1){
      
      adc_read=map(adc_read, 660, 940, 1, 100);
      

        if(adc_read<10){
            lcd.print("LOW BATTERY");
        }
        else{
            lcd.print("BATT:    ");
            lcd.setCursor(11,1);
            lcd.print(adc_read);
            lcd.setCursor(14,1);
            lcd.print("%");            
        }

      real_voltage = real_voltage*122/22;
    }
    //wyswietlanie spadku napiecia na diodzie
    /*
    else{
        lcd.print(" LED:    "); 
        lcd.setCursor(11,1);
        lcd.print(real_voltage);  
    }
    */
 
}

//odczyt temperatury i ustawienie odpowiedniej flagi wyjściowej
uint8_t temperature_read(float temperature) 
{ 
  if(temperature >= TEMP_HOT ) return 2;
  else if(temperature >= TEMP_LOW && temperature < TEMP_HOT) return 1;
  else return 0;
}

//odczyt wartości z enkodera
uint8_t encoder_read(void) 
{ 
 uint8_t val=0; 

  if(!bit_is_clear(PIND, OUTA)) 
	val |= (1<<1); 

  if(!bit_is_clear(PIND, OUTB)) 
	val |= (1<<0); 

  return val; 
}