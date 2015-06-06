/*
 * Dwengo_connect_demo.cpp
 * 
 * Serial communication example for the Raspberry Pi.
 * Connects to the Dwengo board and shows some fuctions.
 * 
 * v1.2 - 31/05/2015
 * Arne Baeyens - 2arne.baeyens@gmail.com
 * 
 * Notes:
 * - for compiling add: -lwiringPi
 * - USB serial port => ttyUSB0
 * - hardware UART => ttyAMA0
 * 
 * Changelog
 * v1.2 updates:
 * - Added stopMotors() function.
 */

#include <string>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include <wiringPi.h>
#include <wiringSerial.h>

#define delay_ms( variable ) usleep( variable*1000 )
#define delay_s( variable ) sleep( variable )

using namespace std;

// Functions for serial communication
void initSerialPort( void );
  int fd;	// file descriptor for serial port.

void send_command();
void add_command( string data );

// Dwengo function declarations
void send_handshake();
void appendStringToLCD( string text );
void appendIntToLCD( int number );
void setCursorLCD( char line, char pos );
void printStringToLCD( string text, char line, char pos );
void printIntToLCD( int number, char line, char pos );
void clearLCD();
void setLeds( char number );
void setVariable( char var, int number );
void setSpeedMotor1( int speed );
void setSpeedMotor2( int speed );
void stopMotors();

// Global variables also used in functions
string command = "$";

// Main program
int main(int argc, char **argv)
{
	signed int i;
	char array_check[6] = {0};

	initSerialPort();
	
	// Check if there's a connection between the devices.
	do{
		serialFlush(fd);
		send_handshake();
		delay_ms(500);
		if(serialDataAvail(fd)>4){
			for( i=0; i<5; i++) array_check[i]=serialGetchar(fd);
		}
	} while(string(array_check)!="$HRC*");

	cout << "Connection ready!" << endl;
	
	// OK, everything connected! We can start with our program.
	setLeds(0);
	clearLCD();
	printStringToLCD( "Hello World!",0,0 );
	send_command();
	delay_s(2);

	clearLCD();
	printStringToLCD( "Ready? ",0,1 );
	send_command();
	delay_s(2);
	
	for ( i=3; i>0; i--){
		setLeds(255);
		printIntToLCD(i,0,9);
		send_command();
		delay_ms(100);
		setLeds(0);
		send_command();
		delay_ms(900);
	}

	printStringToLCD( "GO!",0,9 );
	printStringToLCD( "Dwengo lightshow",1,0);
	send_command();
	delay_ms(50);
	
	for(i=5; i<115; i+=2){
		float time_high=1.0/255/10*i;
		float time_low=1.0/255/10*(120-i);

		setLeds(255);
		send_command();
		delay_ms(time_high*1000);
		setLeds(0);
		send_command();
		delay_ms(time_low*1000);
	}

	setLeds(255);
	send_command();
	delay_s(1);

	for(i=0; i<16; i++){
		setLeds( 0b00000001<<(i%8) );
		send_command();
		delay_ms(100);
	}
	for(i=0; i<10; i++){
		setLeds( 0b01001001<<(i%3) );
		send_command();
		delay_ms(120);
	}
	for(i=0; i<20; i++){
		setLeds(255);
		send_command();
		delay_ms(10);
		setLeds(0);
		send_command();
		delay_ms(30);
	}
	for(i=0; i<15; i++){
		setLeds(255);
		send_command();
		delay_ms(30);
		setLeds(0);
		send_command();
		delay_ms(10);
	}
	for(i=0; i<5; i++){
		setLeds( 0b10101010 );
		send_command();
		delay_ms( 100 );

		setLeds( 0b01010101 );
		send_command();
		delay_ms( 100 );
	}
	for(i=0; i<256; i++){
		setLeds( i );
		send_command();
		delay_ms(10);
	}
	for(i=0; i<5; i++){
		setLeds(255);
		send_command();
		delay_ms(250);
		setLeds(0);
		send_command();
		delay_ms(250);
	}
	return 0;
}

void initSerialPort( void ){
  if((fd = serialOpen("/dev/ttyUSB0", 115200)) < 0) {
    cout << "Unable to open serial device: " << fd << endl;
    cout << "Check connections." << endl;
  }
  else cout << "Port open!" << endl;
}

void send_command(){
	command+="*";
	cout << "Sent:	" << command << endl;
	serialPuts( fd, command.c_str() );
	command = "$";
	//serialFlush( fd );					// Clean the serial port.
}

void add_command( string data ){
	if((command.length()+data.length())<120){
		if(command[command.length()-1]!='$') command+=",";
		command+=data;
	}
}
void send_handshake(){
	add_command( "HSK" );
	send_command();
	delay_ms(50);
}
void appendStringToLCD( string text ){
	string quote = "'";
	add_command( "LCDT,"+quote+text+quote );
}
void appendIntToLCD( int number ){
	char hex_number[5];
	sprintf( hex_number, "%X", number);
	add_command( "LCDI,"+string(hex_number) );
}
void setCursorLCD( char line, char pos ){
	char hex_1[2], hex_2[2];
	sprintf( hex_1, "%X", line);
	sprintf( hex_2, "%X", pos);
	add_command( "SCR,"+string(hex_1)+string(hex_2) );
}
void printStringToLCD( string text, char line, char pos ){
	setCursorLCD( line, pos );
	appendStringToLCD( text );
}
void printIntToLCD( int number, char line, char pos ){
	setCursorLCD( line, pos );
	appendIntToLCD( number );
}
void clearLCD(){
	add_command( "LCDC" );
}
void setLeds( char number ){
	char hex_number[3];
	sprintf( hex_number, "%X", number);
	add_command( "LED,"+string(hex_number) );
}
void setVariable( char var, int number ){
	char hex_var[3];
	char hex_number[5];
	sprintf( hex_var, "%X", number);
	sprintf( hex_number, "%X", number);
	add_command( "V"+string(hex_var)+","+string(hex_number) );
}
void setSpeedMotor1( int speed ){
	char hex_number[5];
	sprintf( hex_number, "%X", speed);
	add_command( "M1,"+string(hex_number) );
}
void setSpeedMotor2( int speed ){
	char hex_number[5];
	sprintf( hex_number, "%X", speed);
	add_command( "M2,"+string(hex_number) );
}
void stopMotors(){
	add_command( "M1,0" );
	add_command( "M2,0" );
}
