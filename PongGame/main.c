#define F_CPU 16000000
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <util/delay.h>
#include "lcd.h"
#include "uart.h"

#define FREQ 16000000
#define BAUD 9600
#define HIGH 1
#define LOW 0
#define BUFFER 1024
#define BLACK 0x000001

char displayChar = 0;
char s1[12];
char s2[12];
unsigned int x_axis = 0;
unsigned int y_axis = 0;
volatile unsigned int x_direct = 0;
volatile unsigned int y_direct = 0;
volatile unsigned int paddle_y1 = 27;
volatile unsigned int paddle_y2 = 27;
volatile unsigned int paddle_speed1 = 0;
volatile unsigned int paddle_speed2 = 0;
volatile unsigned int score1 = 0;
volatile unsigned int score2 = 0;
volatile unsigned int flag = 0;

struct ball
{
	int8_t x;
	int8_t y;
	int8_t flagx;
	int8_t flagy;
}ball1;

void init_game()
{
	PORTB |= (1<<PORTB0);                                //open PB0 to change color
	fillrect(buff,1,paddle_y1,2,10,displayChar);         //draw paddle1
	fillrect(buff,125,paddle_y2,2,10,displayChar);       //draw paddle2
	
	drawrect(buff,0,0,128,64,displayChar);
	drawline(buff,64,0,64,2,displayChar);
	drawline(buff,64,6,64,8,displayChar);
	drawline(buff,64,12,64,14,displayChar);
	drawline(buff,64,18,64,20,displayChar);
	drawline(buff,64,24,64,26,displayChar);
	drawline(buff,64,37,64,39,displayChar);
	drawline(buff,64,43,64,45,displayChar);
	drawline(buff,64,49,64,51,displayChar);
	drawline(buff,64,55,64,57,displayChar);
	drawline(buff,64,61,64,63,displayChar);

	OCR1A = (rand()%6 + 1) * 100 + 750;                 //create random velocity
	TCNT1 = 0;                                          //initial timer1
	
	if(score1 >= 9 || score2 >= 9){                    //reset the game after scrore 9
		score1 = 0;
		score2 = 0;
	}
	
	sprintf(s1, "%d", score1);                       //convert score from int to string
	sprintf(s2, "%d", score2);
	drawstring(buff,50,0,s1);                        //print score
	drawstring(buff,76,0,s2);
	
	ball1.x = 64;                                   //ball initialization
	ball1.y = 32;
	ball1.flagx = rand()%2;                         //create random direction
	ball1.flagy = rand()%2;
	if(ball1.flagx == 0){
		x_direct = -1;
	}
	else{
		x_direct = 1;
	}
	if(ball1.flagy == 0){
		y_direct = -1;
	}
	else{
		y_direct = 1;
	}
	
	fillcircle(buff,ball1.x,ball1.y,2,displayChar);             //draw the ball
	write_buffer(buff);
	PORTB &= ~(1<<PORTB0);                                     //change the background color back
}

void init_x()
{
	DDRC &= ~(1 << PORTC3);	
	DDRC |= (1 << PORTC0) | (1 << PORTC2);
	PORTC &= ~(1 << PORTC2);
	PORTC |= (1 << PORTC0);
	DDRC &= ~(1 << PORTC1);	
	
	PORTC |= (1 << PORTC1);
	ADMUX |= (1 << MUX0);
	ADCSRA |= (1 << ADSC);
}

unsigned int get_x(int x)
{
	ADCSRA &= ~(1 << ADSC);
	return ((x - 89) * 33/(860 - 89))*4;
}

void init_y()
{
	DDRC &= ~(1 << PORTC2);
	DDRC |= (1 << PORTC1) | (1 << PORTC3);
	PORTC &= ~(1 << PORTC3);
	PORTC |= (1 << PORTC1);
	DDRC &= ~(1 << PORTC0);
	
	PORTC |= (1 << PORTC0);
	ADMUX &= ~(1 << MUX0);
	ADCSRA |= (1 << ADSC);
}

unsigned int get_y(int y)
{
	ADCSRA &= ~(1 << ADSC);
	return ((y - 125) * 33/(640 - 125))*2;
}

int main(void)
{
	uart_init();
	//setting up the gpio for backlight
	DDRD |= 0x80;
	PORTD &= ~0x80;
	PORTD |= 0x00;
	
	DDRB |= 0x05;
	PORTB &= ~0x05;
	PORTB |= 0x00;
	
	DDRB |= (1<<PORTB1);                                           //Use PB1 as buzzer output
	PORTB &= ~(1<<PORTB1);
	
	ADMUX |= (1 << REFS0);                                            //reference 5V
	ADCSRA |= (1 << ADEN);                                            //enable ADC
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);             //factor 128
	ADCSRA &= ~(1 << ADATE);                                           //single ADC bit
	
	//lcd initialization
	lcd_init();
	lcd_command(CMD_DISPLAY_ON);
	lcd_set_brightness(0x18);
	write_buffer(buff);
	_delay_ms(1000);
	clear_buffer(buff);
	
	init_game();
	
	//timer 1 initialization
	TCCR1B |= (1 << CS12)|(1 << CS10)|(1 << WGM12);
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = (rand()%10 + 1) * 50 + 750;
	TCNT1 = 0;
	sei();
	
	while (1)
	{
		
		init_x();
		_delay_us(300);
		x_axis = get_x(ADC);
		
		init_y();
		_delay_us(300);
		y_axis = get_y(ADC);

		drawrect(buff,0,0,128,64,displayChar);
		drawline(buff,64,0,64,2,displayChar);
		drawline(buff,64,6,64,8,displayChar);
		drawline(buff,64,12,64,14,displayChar);
		drawline(buff,64,18,64,20,displayChar);
		drawline(buff,64,24,64,26,displayChar);
		drawline(buff,64,37,64,39,displayChar);
		drawline(buff,64,43,64,45,displayChar);
		drawline(buff,64,49,64,51,displayChar);
		drawline(buff,64,55,64,57,displayChar);
		drawline(buff,64,61,64,63,displayChar);

	}
}

ISR(TIMER1_COMPA_vect)
{
	if(flag == 1){                                         //change the state of buzzer
		flag = 0;
		PORTB &= ~(1<<PORTB1);
	}
	clearcircle(buff,ball1.x,ball1.y,2,displayChar);        //clear the old circle
	if(ball1.x == 5)                                       
	{
		if(ball1.y >= paddle_y1 && ball1.y <= paddle_y1 + 9){       //if the ball hits the paddle on the left
			x_direct = -x_direct;
			PORTB |= (1<<PORTB1);
			flag = 1;
		}
		else{                                                        //if the ball misses the paddle on the left
			score2++;
			clearrect(buff,1,paddle_y1,2,10,displayChar);
			clearrect(buff,125,paddle_y2,2,10,displayChar);
			paddle_y1 = 27;
			paddle_y2 = 27;
			init_game();
			clearcircle(buff,ball1.x,ball1.y,2,displayChar);
			PORTB |= (1<<PORTB1);
			flag = 1;
		}
	}
	if(ball1.x == 122)
	{
		clearrect(buff,125,paddle_y2,2,10);
		paddle_y2 = ball1.y - 4;
		fillrect(buff,125,paddle_y2,2,10,displayChar);
		if(ball1.y >= paddle_y2 && ball1.y <= paddle_y2 + 9){          //if the ball hits the paddle on the right
			x_direct = -x_direct;
			PORTB |= (1<<PORTB1);
			flag = 1;
		}
		else{                                                           //if the ball misses the paddle on the right
			score1++;
			clearrect(buff,1,paddle_y1,2,10,displayChar);
			clearrect(buff,125,paddle_y2,2,10,displayChar);
			paddle_y1 = 27;
			paddle_y2 = 27;
			init_game();
			clearcircle(buff,ball1.x,ball1.y,2,displayChar);
			PORTB |= (1<<PORTB1);
			flag = 1;
		}
	}
	if(ball1.y == 3 || ball1.y == 60)                                   //if the ball hits the upper or bottom bound
	{
		y_direct = -y_direct;
		PORTB |= (1<<PORTB1);
		flag = 1;
	}
	
	
	ball1.x += x_direct;                                           //change the direction of the ball
	ball1.y += y_direct;
	
	if(x_axis < 64 && x_axis >= 0){                                 //the paddle on the left
		clearrect(buff,1,paddle_y1,2,10);
		if(y_axis > 64 || y_axis < 0){
			paddle_speed1 = 0;
		}
		else if(y_axis > paddle_y1 + 5){                           //paddle go down
			paddle_speed1 = 1;
		}
		else{                                                      //paddle go up
			paddle_speed1 = -1;
		}
		paddle_y1 += paddle_speed1;
		fillrect(buff,1,paddle_y1,2,10,displayChar);
	}
	else{                                                            //what if the touching point is outside the box
		paddle_speed1 = 0;
		paddle_speed2 = 0;
		fillrect(buff,1,paddle_y1,2,10,displayChar);
		fillrect(buff,125,paddle_y2,2,10,displayChar);
	}
	
	fillcircle(buff,ball1.x,ball1.y,2,displayChar);                   //draw the new circle
	drawstring(buff,50,0,s1);                                         //print score
	drawstring(buff,76,0,s2);
	write_buffer(buff);
}


