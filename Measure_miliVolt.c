//************************************************************************//
//*******************LAB REPORT : QuesTion 2******************************//
//************************************************************************//

#include<16f88.h>
#device ADC = 10
#fuses NOWDT, INTRC_IO, NOMCLR
#use delay(clock = 8M)
#include <string.h>

struct lcd_pin_map {
	boolean enable;
	boolean rs;
	boolean rw;
	boolean unused;
	int data : 4;
} lcd;
void Enable(boolean v)
{
	lcd.enable = v;
	if (v) output_high(PIN_B0);
	else  output_low(PIN_B0);
}
void RS(boolean v)
{
	lcd.rs = v;
	if (v) output_high(PIN_B1);
	else  output_low(PIN_B1);
}
void RW(boolean v)
{
	lcd.rw = v;
	if (v) output_high(PIN_B2);
	else  output_low(PIN_B2);
}
void Data(int d)
{
	lcd.data = d;
	output_b((int8)lcd);
}
boolean readBusy()
{
	boolean bf;
	set_tris_b(0xf0);
	RS(0);
	delay_us(1);
	RW(1);
	delay_us(1);
	Enable(1);
	delay_us(1);
	bf = input(PIN_B7);
	Enable(0);
	delay_us(1);
	Enable(1);
	delay_us(1);
	Enable(0);
	set_tris_b(0x00);
	return bf;
}
//RS is adjusted before
void sendNibble(int d)
{
	RW(0);
	delay_us(1);
	Data(d);
	delay_us(1);
	Enable(1);
	delay_us(2);
	Enable(0);
}
void sendByte(int d, boolean IorD)
{
	while (readBusy());
	RS(IorD);
	delay_us(1);
	sendNibble(d >> 4);
	sendNibble(d & 0x0f);
}
void lcd_initial()
{
	int i;
	delay_ms(15);
	RS(0);
	for (i = 0; i < 3; i++)
	{
		sendNibble(0b0011);
		delay_ms(5);
	}
	sendNibble(0b0010);
	sendByte(0b00100000, 0);
	sendByte(0b00001100, 0);
	sendByte(0b00000001, 0);
	sendByte(0b00000110, 0);
}

void lcdPutc(char ch)
{
	if (ch == '\f')
	{
		sendByte(0b00000001, 0);
		delay_ms(2);
	}
	else
	{
		sendByte(ch, 1);
	}
}

void main()
{
	char mystring[20];
	setup_oscillator(OSC_8MHZ | OSC_INTRC);
	setup_adc_ports(sAN0 | sAN1 | sAN2 | sAN3);
	setup_adc(ADC_CLOCK_DIV_16);

	lcd_initial();
	lcdPutc('\f');
	lcdPutc('D');
	lcdPutc('r');
	lcdPutc('.');
	lcdPutc('A');
	lcdPutc('d');
	lcdPutc('e');
	lcdPutc('l');
	delay_ms(2000);
	while (1)
	{
		int16 digital;
		float analog, v, b1;
		int8 i, isMiliVolt; //isMiliVolt = 1 then it is small voltage in mv, else it is large voltage


		isMiliVolt = 0;
		set_adc_channel(1);
		delay_ms(1);
		digital = read_adc();

		if (digital < 10) //indicates small voltage (v<0.1v)
		{
			isMiliVolt = 1;
			set_adc_channel(4);
			delay_ms(1);
			digital = read_adc();
			if (digital < 1023) //measure values smaller than 10mv
			{
				analog = digital * 5 / 1024.0;
				v = analog * 2;
			}
			else //measure from 10mv to 100mv
			{
				set_adc_channel(0);
				delay_ms(1);
				digital = read_adc();
				analog = digital * 5 / 1024.0;
				v = analog * 2 * 10;

			}

		}
		else //large value
		{
			isMiliVolt = 0;

			if (digital < 1023) // from .1v to 10v
			{
				analog = digital * 5 / 1024.0;
				v = analog * 2;

			}
			else // from 10v to 25v
			{
				set_adc_channel(2);
				delay_ms(1);
				digital = read_adc();
				if (digital < 1023)
				{
					analog = digital * 5 / 1024.0;
					v = analog * 5;
				}
				else // from 25v to 45v
				{
					set_adc_channel(3);
					delay_ms(1);
					digital = read_adc();
					analog = digital * 5 / 1024.0;
					v = analog * 9;
				}
			}
		}



		lcdPutc('\f');
		sprintf(mystring, "%5.2f", v);
		for (i = 0; i < strlen(mystring); i++)
			lcdPutc(mystring[i]);

		if (isMiliVolt)
		{
			lcdPutc('m');
			lcdPutc('v');
		}
		else
		{
			lcdPutc('v');
		}
		delay_ms(1000);
	}
}

