#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  Dwengo_connect_demo.py
#  Serial communication example for interaction between PC or Raspberry Pi and microcontroller.
#  Connects to the Dwengo board and shows some fuctions.
#  
#  v1.2 - 31/05/2015
#
#  Arne Baeyens - 2arne.baeyens@gmail.com
#
#  Notes:
#  - you may be unable to open the serial port if you have no permission. Run with sudo ~.
#  - USB serial port => ttyUSB0
#  - hardware UART => ttyAMA0 (Raspberry Pi)
#
#  Changelog
#  v1.2 updates:
#  - Added stopMotors() function.
#  
#  

import serial
import time

# Open the serial port
port = serial.Serial("/dev/ttyUSB0", baudrate=115200, timeout=3.0)
print("Port open!")

# Global variable also used in functions
command = "$"

# Functions
def send_command():
	global command
	command+="*"
	print( "Sent:		"+command )
	port.write(command)
	command = "$"

def add_command( data ):
	global command
	if(len(command+data)<120):
		if(command[-1])!='$': command+=","
		command+=data

def send_handshake():
	add_command( "HSK" )
	send_command()
	time.sleep(0.05)

def appendStringToLCD( text ):
	add_command( 'LCDT,"'+text+'"' )

def appendIntToLCD( number ):
	add_command( "LCDI,"+format(number,"X") )

def setCursorLCD( line, pos ):
	add_command( "SCR,"+format(line,"X")+format(pos,"X") )

def printStringToLCD( text, line, pos ):
	setCursorLCD( line, pos )
	appendStringToLCD( text )
	
def printIntToLCD( number, line, pos ):
	setCursorLCD( line, pos )
	appendIntToLCD( number )
	
def clearLCD():
	add_command( "LCDC" )
	#printStringToLCD("                ",0,0)
	#printStringToLCD("                ",1,0)

def setLeds( number ):
	add_command( "LED,"+format(number,"X") )

def setVariable( var, number ):
	add_command( "V"+format(var,"X")+","+format(number,"X") )

def setSpeedMotor1( speed ):
	add_command( "M1,"+format(speed,"X") )

def setSpeedMotor2( speed ):
	add_command( "M2,"+format(speed,"X") )
	
def stopMotors():
	add_command( "M1,0" )
	add_command( "M2,0" )


# Main program
# Check if there's a connection between the devices.
while True:
	send_handshake()
	if(port.read(5)=="$HRC*"):
		break

print( "Connection ready!\n" )

# OK, everything connected! We can start with our program.
setLeds(0)
clearLCD()
# Try to put a command in the string to let the Dwengo board crash!
# add_command( 'jg,"ghba.68" ," *,,hj*hjgjf!e,$e/h<8$86969;;::..//' )
printStringToLCD( "Hello World!",0,0 )
send_command()
time.sleep(2)

clearLCD()
printStringToLCD( "Ready? ",0,1 )
send_command()
time.sleep(2)

for i in range(0,3):
	printIntToLCD(3-i,0,9)
	setLeds(255)
	send_command()
	time.sleep(0.1)
	setLeds(0)
	send_command()
	time.sleep(0.9)

printStringToLCD( "GO!",0,9 )
printStringToLCD( "Dwengo lightshow",1,0)
send_command()
time.sleep(0.05)

for i in range(5,115,2):
	time_high=1.0/255/10*i
	time_low=1.0/255/10*(120-i)

	setLeds(255)
	send_command()
	time.sleep(time_high)
	setLeds(0)
	send_command()
	time.sleep(time_low)

setLeds(255)
send_command()
time.sleep(1)

for i in range (16):
	setLeds( 0b00000001<<(i%8) )
	send_command()
	time.sleep(0.1)

for i in range(10):
	setLeds( 0b01001001<<(i%3) )
	send_command()
	time.sleep(0.12)

for i in range(20):
	setLeds(255)
	send_command()
	time.sleep(0.01)
	setLeds(0)
	send_command()
	time.sleep(0.03)

for i in range(15):
	setLeds(255)
	send_command()
	time.sleep(0.03)
	setLeds(0)
	send_command()
	time.sleep(0.01)

for i in range(5):
	setLeds( 0b10101010 )
	send_command()
	time.sleep( 0.1 )

	setLeds( 0b01010101 )
	send_command()
	time.sleep( 0.1 )

for i in range(256):
	setLeds( i )
	send_command()
	time.sleep(0.01)

for i in range(5):
	setLeds(255)
	send_command()
	time.sleep(0.250)
	setLeds(0)
	send_command()
	time.sleep(0.250)

# Show some data the board sent back
rcv = port.read(10)
print( "Received:	"+rcv )
