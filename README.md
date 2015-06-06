# Dwengo Connect

### Wat?
Dwengo connect is een interface die communicatie tussen een computer en het Dwengo pic bord makkelijk en eenvoudig maakt. 

### Wat moet ik installeren?
Het programma bestaan uit twee delen: een interface voor op de computer die de functies die je gebruikt in een net protocol giet en doorstuurt en het programma op het Dwengo bord dat deze ontvangt, decodeert en uitvoert.
Je kan ook met een seriële terminal zoals teraterm werken en de message-strings zelf typen. In dat geval is er geen programmeersoftware aan de computerzijde nodig.

Op de microcontroller dien je met behulp van MPLABX het bestand `Dwengo_connect_pic_board_vX-X.c` te flashen. Je kan deze code aanpassen om zo functionaliteit toe te voegen maar je kan ook de code plakken in je bestaande programma. Dat laatste is al wat moeilijker. Het ontvangen en verwerken van data is interrupt-gebaseerd waardoor de main-lus van het programma vrij is zodat er plaats is voor jouw programma.

Voor de computerzijde is er een demo programma in Python voorzien. Je kan het bestand `Dwengo_connect_demo_vX-X.py` downloaden, bewerken in je favoriete IDE en vervolgens met een Python interpreter runnen.

Voor de Rapsberry Pi is er ook een bestand in C++, `Dwengo_connect_Rpi_demo_vX-X.cpp` voorzien. Dit maakt gebruik van de wiringPi bibliotheek en werkt enkel op de Raspberry Pi. Voor het compileren zal je dus ook de wiringPi bibliotheek moeten installeren.

### Hoe gebruik ik de interface aan de computerzijde?
Om data door te sturen moet je steeds twee functies gebruiken:

1. een functie naar keuze, bv. `appendStringToLCD(“Hello World”)`
2. (optioneel) je kan nog functies toevoegen maar zorg ervoor dat de lijst niet te lang wordt.
3. `send_data()`. Deze stuurt al de data door. De functies staan samen in een command en worden sequentieel uitgevoerd.

### Welke functies zijn er tot nu toe beschikbaar?
Je kan de volgende functies gebruiken:
```c++
send_handshake();
appendStringToLCD( string text );
appendIntToLCD( int number );
setCursorLCD( char line, char pos );
printStringToLCD( string text, char line, char pos );
printIntToLCD( int number, char line, char pos );
clearLCD();
setLeds( char number );
setVariable( char var, int number );
setSpeedMotor1( int speed );
setSpeedMotor2( int speed );
stopMotors();
```

De functie 'setVariable' is een beetje een buitenbeentje. Deze zendt een waarde van een variabele door en wordt in de microcontroller vervolgens ingeladen. Je kan deze dan gebruiken in je eigen programma. Als je deze gebruikt met het demo programma wordt de variabele wel ingeladen maar er wordt vervolgens niets mee gedaan.

### Welk protocol wordt er gebruikt?
Een voorbeeld van een message-string:
`$SCR,00,LCDT,"Hello World!",SCR,10,LCDI,80,LED,AA,V2,15,M1,3FF*`

deze voert de volgende functies uit:
```python
printStringToLCD( "Hello World!",0,0 )
printIntToLCD( 128, 1,0)
setLeds( 0b10101010 )
setVariable( 2, 21 )    # Waarde voor variabele doorsturen.
SetSpeedMotor1( 1023 )
```

Een message string bestaat steeds uit de volgende componenten:

volledige string: `$<command_name>*`

onderdelen:

Karakter | Betekenis
--- | ---
`$` | markeert het begin van een nieuwe message
`<command_name>` | naam van de command
`*` | sluit de string af


Bij bepaalde commands hoort er ook data. Dan ziet het er zo uit:
`$<command_name>,<command_data>*`

Je kan zo veel commands achter elkaar zetten als je wil maar de message strings mogen maximum 120 tekens lang zijn. De Python code geeft in de terminal ook weer wat er verzonden wordt. Zo kan je zelf zien hoe de data van bepaalde functies wordt gestructureerd.

### Vragen, bugs, tips?
Op het Dwengo forum is er een forumtopic (link) voorzien waar je terecht kan met al je vragen en tips. Mocht iets van interface niet werken zoals voorzien, laat dat dan zeker weten in die forumthread. Als je zelf een mooi project hebt gemaakt met de interface, post dan zeker een berichtje, liefst met een beetje uitleg en een paar foto's.

### Bekende bugs?
Volgende bug is bekend:
- Wanneer er twee message strings vlak achter elkaar worden verstuurd, zullen de tweede string en diegene die daarop volgt niet of maar gedeeltelijk verwerkt worden. In de main-lus wordt in geval van een bufferprobleem de ontvanger gereset zodat het systeem zich herpakt.

Als je zelf denkt een bug gevonden te hebben post dan een berichtje in de forumthread. Je kan ook hier op GitHub een bug melden.

### TODO?
- bugs verhelpen.
- code aan de microcontrollerzijde opschonen.
- protocol ook implementeren in de Arduino language zodat er ook compatibiliteit is met het Dwenguino bord.
- code aan de computerzijde in een Python module gieten zodat niet altijd de gehele code moet gekopieerd worden.
- toevoegen van functies voor uitlezen analoge poorten en aansturen digitale poorten.
- voor iemand met wat meer ervaring met de pic 18f4550: de seriële interrupt combineren met een timer interrupt voor het aansturen van servos en een interrupt-on-change voor de drukknoppen.
- … ?
