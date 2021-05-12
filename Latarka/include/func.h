#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DS18B20.h>

//Definicje operacji bitowych (dla wygody)
#ifndef _BV
#define _BV(bit)				(1<<(bit))
#endif

#ifndef sbi
#define sbi(reg,bit)		reg |= (_BV(bit))
#endif
 
#ifndef cbi
#define cbi(reg,bit)		reg &= ~(_BV(bit))
#endif

#ifndef fbi
#define fbi(reg,bit)		reg ^= (_BV(bit))
#endif


#define FAN PB2         //załącznie wentylatora
#define LED PB1         //pwm led
#define SW PD4          //przycisk na enkoderze
#define OUTA PD5        //wejście A enkodera
#define OUTB PD6        //wejście B enkodera

#define ONEWIRE_PIN 7   //pin termometru DS18B20
#define TEMP_HOT 35     //temperatura załączenia wentylatora
#define TEMP_LOW 30     //temperatura wyłączenia wentylatora


//definicje funkcji
void start(LiquidCrystal_I2C lcd);
void pwm_ON();
void pwm_OFF();
void adc_start();
uint16_t read_voltage(uint8_t channel);
void show_voltage(uint8_t channel, LiquidCrystal_I2C lcd);
void sleep(uint16_t time);
void menu(LiquidCrystal_I2C lcd, uint8_t mode);
uint8_t temperature_read(float temperature);
uint8_t encoder_read(void);