#include <avr/io.h>
#include <util/delay.h>
#define F_CPU 1000000UL

#define SCLK PB0
#define RCLK PB1
#define DATA PB2

void port_init(void){
// Инициализировать выходной порт для дисплея
// PB0 SCLK, PB1 RCLK, PB2 DIO
	DDRB |= 0b111; //PB0-PB2 выходы
	PORTB &=0b000;
}

void DAC_init(void){
//	Настроить и включить АЦП на PB4 - пин 3 - ADC2

	ADMUX &= ~((1 << REFS1) | (1 << REFS0)); // 0 в REFS0 и REFS1, опорное напряжение Vcc
	ADMUX &= ~((1 << ADLAR)); // 0 в ADLAR - полностью использовать 10-битную точность

	ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX0)); // 0010 - выбор ADC2 без дополнительных функций
	ADMUX |= (1<< MUX1);

	ADCSRA |= (1 << ADPS1) | (1 << ADPS0); // 011 делитель 8, частота АЦП 1000/8 = 125 КГц
	ADCSRA |= (1 << ADEN); // включение АЦП
}

int raw_ADC(void){
// считать показания ADC, сконвертировать в int

	ADCSRA |= (1 << ADSC); // начать преобразование
	while (ADCSRA & (1 << ADSC)); // ждать сброса бита - окончания преобразования
//	int readout = ADCL;
//	readout += (ADCH << 7);
//	return readout;
	return ADC;
}

const uint8_t digit_pos[4] =
{ 0b00000001,
  0b00000010,
  0b00000100,
  0b00001000
};

const uint8_t digit_val[10] =
{ 0b11000000,
  0b11111001,
  0b10100100,
  0b10110000,
  0b10011001,
  0b10010010,
  0b10000010,
  0b11111000,
  0b10000000,
  0b10011000
};

void push_byte(uint8_t byte2push){
// задвинуть 8 бит в сдвиговый регистр

  PORTB &= ~(1<<SCLK);
  PORTB &= ~(1<<DATA);

  for (int8_t bit_num=7; bit_num>=0; bit_num--){ //SIGNED!!!!!!!!!!!!!

	PORTB &= ~(1<<SCLK);
	if ( (byte2push >> bit_num) &0b1 ) { PORTB |= (1<<DATA); } else { PORTB &= ~(1<<DATA); }
	PORTB |= (1<<SCLK);
  }
}

void led_digit(uint8_t num, uint8_t val){
// зажечь в заданной позиции заданное число

  PORTB &= ~(1<<RCLK);

  push_byte(digit_val[val]);
  push_byte(digit_pos[num]);

  PORTB |= (1<<RCLK);

}

void led_out(uint16_t val){
  // отобразить четырехзначное число

  uint8_t vals[4];
  vals[0] = val % 10;
  vals[1] = (val % 100) / 10;
  vals[2] = (val % 1000) / 100;
  vals[3] = val / 1000;

  for (uint8_t dig_num = 0; dig_num <4; dig_num++){
    led_digit(dig_num, vals[dig_num]);
  }

}

int main(void){

	port_init();
	DAC_init();

	uint16_t ADC_readout;

  	while(1) {

  		// надо периодически опрашивать АЦП и постоянно обновлять индикацию
  		// поскольку таймер настраивать лениво, будет так
  		ADC_readout = raw_ADC();
  		for (uint16_t dummy_counter=0; dummy_counter<1000; dummy_counter++) { led_out(ADC_readout); }
  	}

	return -1; //никогда
}
