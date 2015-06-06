/* Dwengo_connect.c
 * Code to run on the microcontroller side.
 *
 * V1.2 - 31/05/2015
 * Arne Baeyens - 2arne.baeyens@gmail.com
 *
 * Changelog
 * V1.2 updates:
 * - Fixed negative value problem. Signed integers are now correctly interpreted.
 */

#include <dwengoConfig.h>
#include <dwengoBoard.h>
#include <dwengoMotor.h>
#include <dwengoADC.h>
#include <stdlib.h>
#include <usart.h>
#include <math.h>


#define testbit_on(data,bitno) ((data>>bitno)&0x01) // To read bits out of a variable.

// Enkele functies.
void configISR(void);		// Voor het instellen van de seriële interrupt.
void initUsart(void);		// Voor het instellen van de seriële verbinding.

void appendBinaryToLCD( int number, BYTE bits);
char len( char* in);
char match_string( char* string, const FAR_ROM char* part);
void appendVarStringToLCD(char* message);

void change_quotes( char* array );

// Variables in the interupt for the serial connection.
  char buffer[125]={0};
  char list[7]={0};
  char j=0;		// For position in buffer array.
  char i=0, n=0;	// For small tasks.

  int data_integer=0;
  char data_string[15] = {0};
  int data_p[4] = {0};
  char minus_bit = 1;
  BYTE quotes = FALSE;
// Other variables.
  int variable[3] = {0};    // Variable array the values are written to.

// start ISR code
#pragma interrupt ISR

void ISR (void){
  if(PIR1bits.RCIF == 1){		// Indien er een een seriële interrupt getriggerd wordt.
///////////////////////
    //LED0 = 1;
    while (!DataRdyUSART());
    buffer[j] = getcUSART();
    change_quotes(buffer[j]);
    //j++;	// This is done at the end of the interrupt.
    if(j==0 && buffer[j]!='$'){		// Waiting for the $ (beginning of new message) sign.
        printStringToLCD("SDE1",1,0);
	quotes=FALSE;
        j=-1;
    }
    else if( buffer[j]=='$' && j!=0 && quotes==FALSE){	// Safety measure against corrupt strings for the unlikely event (?) the if() above would fail.
	buffer[0]=buffer[j];
	printStringToLCD("SDE2",1,0);
	quotes=FALSE;
	j=-1;
    }
    else if( (j>1 && buffer[j]=='*' && quotes==FALSE ) || j > 120 ){	    // Message is ready, so time to decode it!
      j=0;				    // Let's start at the first character.
      while(buffer[j]!='*' && quotes==FALSE && j<120){	    // We will decode a command, execute it, and then move on to the next command in the buffer array.
          j++;						    // The j<120 condition is to prevent that the decode loop runs infinitely.
          for( i=0; buffer[j]!='*' && buffer[j]!=',';){	    // The command consists of a name (saved in list array) and optionally some data.
              list[i]=buffer[j];
              j++;
              i++;
          }
	  list[i]=0;					    // Terminate the list array.

          // Show list[], for debugging.
	  /*appendCharToLCD(list[0]);
          appendCharToLCD(list[1]);
          appendCharToLCD(list[2]);
          appendCharToLCD(list[3]);
          appendCharToLCD(list[4]);
          appendCharToLCD('-');*/

	  // These lists have no data.
	  if( (match_string(list, (const FAR_ROM char*) "HSK")||match_string(list, (const FAR_ROM char*) "LCDC")) && buffer[j]!='*');
	  // But if it's a message for the LCD, extract string (text).
	  else if(match_string(list, (const FAR_ROM char*) "LCDT") && buffer[j]!='*'){
              j++;
              //data_string={0};
              for( i=0; buffer[j]!='*' && quotes==FALSE && buffer[j]!=',';){
		  change_quotes( buffer[j] );
                  if(i!=0) data_string[i-1]=buffer[j];				// Those quotes don't have to appear in the final message.
                  j++;
                  i++;
              }
              data_string[i-2]='\0';
          }
          // or else extract integer (numbers).
          else if(buffer[j]!='*'){
              j++;
              data_integer=0;
	      minus_bit=1;
	      if(buffer[j]=='-'){
		  minus_bit = -1;
		  j++;
	      }
              for( i=0; buffer[j]!='*' && buffer[j]!=',';){
		  if(buffer[j]>>4==0b00000011) data_p[i]=(buffer[j]&0b00001111);
                  else if(buffer[j]>>4==0b00000100) data_p[i]=((buffer[j]&0b00001111)+9);
                  j++;
                  i++;
              }
              for( n=i-1; n>=0; n--) data_integer += data_p[i-n-1]<<((n)*4);
	      data_integer*=minus_bit;
          }
	  // Now decide which action has to be executed.
          if(list[0]=='V' && list[1]>47 && list[1]<58) variable[list[1]-48]=data_integer;	// Variable.
	  else if (match_string(list, (const FAR_ROM char*) "HSK")) putrsUSART((const far rom char *)"$HRC*\n\r");  // Handshake.
          else if (list[0]=='M' && (list[1]=='1' || list[1]=='2')) {					// Motor.
              switch (list[1]){
                  case '1': setSpeedMotor1(data_integer);
		    break;
                  case '2': setSpeedMotor2(data_integer);
		    break;
              }
          }
	  else if (match_string(list, (const FAR_ROM char*) "LED")) LEDS = data_integer;		    // Leds.
	  else if (match_string(list, (const FAR_ROM char*) "LCDI")) appendIntToLCD(data_integer);	    // Variable on LCD.
	  else if (match_string(list, (const FAR_ROM char*) "LCDT")) appendVarStringToLCD(data_string);	    // Text on LCD.
	  else if (match_string(list, (const FAR_ROM char*) "LCDC")) clearLCD();			    // Clear the LCD.
	  else if (match_string(list, (const FAR_ROM char*) "SCR")) setCursorLCD(data_p[0],data_p[1]);	    // Move the cursor on the LCD.
          //else if (match_string(list,"RST")); // Reset the board.
	  else {				    // It can happen that the string is not recognized correctly.
              printStringToLCD("SNR: ",1,0);	    // String no recognised.
              appendVarStringToLCD(list);	    // So show us that string.
	      //delay_s(2);			    // And wait a while so we can see it.
	      //clearLCD();
          }
	  // And, to end, send something back to acknowledge receiving.
	  // An handshake request will get two messages back: $HRC* and $SRC*.
	  // These messages are a bit formatted (\n,\r) so they are well readable in terminal.
	  putrsUSART((const far rom char *)"$SRC*\n\r");
      }
      quotes=FALSE;
      j=-1;
    }
    j++;
    //LED0=0;
///////////////////////
  }
}

#pragma code high_vector=0x08
void high_vector() {
  _asm
    goto ISR
  _endasm
}
#pragma code
// Einde van de interrupt.

void main (void){
  initBoard();
  initMotor();
  //initADC();
  backlightOn();
  // Open de connectie.
  initUsart();
  // Configureer de interrupt.
  configISR();

  putrsUSART((const far rom char *)"$HSK*\n\r");
  putrsUSART((const far rom char *)"Hello, here I am!\n\r");

  printStringToLCD("Dwengo",0,0);
  printStringToLCD("Dataconnect",1,3);
  delay_s(1);
  printStringToLCD("                ",1,0);
  printStringToLCD("v1.2 beta",1,3);
  delay_s(1);
  clearLCD();

  while(TRUE){
      delay_ms(100);
      /*setCursorLCD(0,0);
      appendStringToLCD("OERR: ");
      appendIntToLCD(RCSTAbits.OERR);*/
      if(RCSTAbits.OERR == 1){
	RCSTAbits.CREN = 0; // enable receiver
	RCSTAbits.CREN = 1; // enable receiver
      }
  }
}

void initUsart(void){
    // USART configuration
    TXSTAbits.TX9 = 0; // 8-bit transmission
    TXSTAbits.TXEN = 1; // transmit enabled
    TXSTAbits.SYNC = 0; // asynchronous mode
    TXSTAbits.BRGH = 0; // high speed
    RCSTAbits.SPEN = 1; // enable serial port - configures RX/DT and TX/CK pins as serial port pins
    RCSTAbits.RX9 = 0; // 8-bit reception
    RCSTAbits.CREN = 1; // enable receiver
    BAUDCONbits.BRG16 = 1; // 16-bit baud rate generator off
    SPBRG = 25; // write the decimal value 25 to the baud rate generator - value calculated using formula from table 20-1 in the PIC18F4550 datasheet
}

void configISR(void){
    TRISCbits.RC6 = 1;
    TRISCbits.RC7 = 1;

    // interrupts / USART interrupts configuration
    RCONbits.IPEN = 0;		// disable interrupt priority
    INTCONbits.GIE = 1;		// enable interrupts
    INTCONbits.PEIE = 1;	// enable peripheral interrupts.
    PIE1bits.RCIE = 1;		// enable USART receive interrupt
}

void appendBinaryToLCD( int number, BYTE bits ){
  signed char b;
  appendStringToLCD("0b");
  for(b=bits-1; b>=0; b--){
    if( testbit_on( number, b ) ) appendCharToLCD('1');
    else appendCharToLCD('0');
  }
  //testbit_on(data,bitno) ((data>>bitno)&0x01)
}

char len( char* in){
    char b;
    for( b=0; in[b]; b++);
    return b;
}

char match_string( char* string, const FAR_ROM char* part){
    char match=1;
    char b;
    //if(len(string)!=len(part)) match=0;
    for( b=len(string); (b>0) && (match==1); b--) if(string[b-1]!=part[b-1]) match=0;
    return match;
}

void appendVarStringToLCD(char* message){
    while (*message) {
        appendCharToLCD(*message++);
    }
}

void change_quotes( char* array ){
    if(*array=='"'){
	if(quotes==FALSE) quotes=TRUE;
	else quotes=FALSE;
    }
}
