#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <serialGLCD.h>
//#include <SD.h>

//RTC_1307:
RTC_DS1307 RTC; //<- eller her?!

//Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //physical mac address
byte ip[] = { 192, 168, 0, 120 };			// ip in lan
byte gateway[] = { 192, 168, 0, 120 };			// internet access via router
byte subnet[] = { 255, 255, 255, 252 };			 //subnet mask
EthernetServer server(84);						  //server port
byte sampledata=50;		//some sample data - outputs 2 (ascii = 50 DEC)
String readString = String(30); //string for fetching data from address
int Light = 0;


// LED and fan
int const bla = 6;
int const hvid = 7;
int const blas = 9;

// Float switch
int const fSwitch = 8;
boolean switchState = false;
boolean switchKonst = true;
int waterMinute = 0;
int waterHour = 0;
int waterDay = 0;
int waterMonth = 0;

//OneWire temp sensor:
// Data wire is plugged into port 5 on the Arduino
#define ONE_WIRE_BUS 5
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
//Adress: 0x28, 0xF5, 0xF6, 0x4F, 0x04, 0x00, 0x00, 0x59

//SD setup
//const int chipSelect = 4;

void setup(){

  pinMode(bla, OUTPUT);  
  pinMode(hvid, OUTPUT); 
  pinMode(blas, OUTPUT);
  pinMode(fSwitch, INPUT);
  pinMode(10, OUTPUT); //Reserved for SD-card!

  Serial.begin(115200); //default for LCD
  
  Wire.begin();
  RTC.begin();
  //RTC.adjust(DateTime(__DATE__, __TIME__)); //Only use at first upload!
  Ethernet.begin(mac, ip, subnet);
  
  
}

serialGLCD lcd; // initialisation

void loop(){
  DateTime now = RTC.now();
  lcd.clearLCD();

   // Create a client connection
EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
   if (client.available()) {
    char c = client.read();
    //read char by char HTTP request
    if (readString.length() < 30)
	{
	  //store characters to string
	  //-----readString.append(c);
	    readString.concat(c);
	}
	  //output chars to serial port
	  //Serial.print(c); NOT IN USE WITH GLCD
	  //if HTTP request has ended
	  if (c == '\n') {
  
            if (readString.substring(6,13) == "Light=1")
            {
              Light = 1;
            }
            if (readString.substring(6,13) == "Light=2")
            {
              Light = 2;
            }
            if (readString.substring(6,13) == "Light=3")
            {
              Light = 3;
            }
            if (readString.substring(6,13) == "Light=0")
            {
              Light = 0;
            }

	    // now output HTML data starting with standart header
	    client.println("HTTP/1.1 200 OK");
	    client.println("Content-Type: text/html");
	    client.println();
	    //set background to color
	    client.print("<body style=background-color:white>");
            //if(readString.indexOf("Akvariuminfo") >0){//checks for on SLEETT
  	      client.println("<h1>Arduino controller</h1>");
	      client.println("<font color='black'>");
	      client.println("<br />");

              client.println("<font color='black' <br />");
            
            //Web_Time
            client.print(now.hour(), DEC);
            client.print(':');
            client.print(now.minute(), DEC);
            client.print(':');
            client.print(now.second(), DEC);
            client.print(' ');
            client.print(now.day(), DEC);
            client.print('/');
            client.print(now.month(), DEC);
            client.print('/');
            client.print(now.year(), DEC);
            client.println("<br />");
            client.println("<br />");
            client.println("<size='5'> Light controller"); 
            client.println("<br />");
            
            client.println("<form method=get name=ON><input type=submit name=Light value=1>ON<form>");
	    client.println("<form method=get name=50pct><input type=submit name=Light value=2>50 pct.<form>");
            client.println("<form method=get name=OFF><input type=submit name=Light value=3>OFF<form>");
            client.println("<form method=get name=Aut><input type=submit name=Light value=0>Aut.<form>");
            client.println("<br />");
            if (Light==0){
              client.println("Running automatically");
            }
            if (Light==1){
              client.println("<font color='blue'>MANUAL: Lights on");
            }
            if (Light==2){
              client.println("<font color='blue'>MANUAL: Lights dimmed");
            }
            if (Light==3){
              client.println("<font color='blue'>MANUAL: Lights off");
            }
            client.println("<font color='black'>");
            client.println("<br />");
            client.println("<br />");
            
            //Light_Temp
            client.print("Light temperature is ");
            client.print(lightTemp());
            client.print("*C");
            client.println("<br />");
            client.println("<br />");
            
            //Water_Temp
            client.print("Aquarium temperature is ");
            sensors.requestTemperatures(); // Send the command to get temperatures
            client.print(sensors.getTempCByIndex(0));
            client.print(" *C");
            client.println("<br />");
            client.println("<br />");
            
             //Water_Level
            switchState = digitalRead(fSwitch);
  
            client.print("Water level: ");
            if (switchState){
              client.print("<font color='green'>");
              client.print("OK");
              switchKonst = true;
            }
             else{
              client.print("<font color='red'>");
              client.print("LOW! ");
              client.print("<font color='black'>");
              client.print("and has been since ");
              while (switchKonst){
                waterMinute = now.minute();
                waterHour = now.hour();
                waterDay = now.day();
                waterMonth = now.month();
                switchKonst = false;
              }
              client.print(waterHour, DEC);
              client.print(":");
              client.print(waterMinute, DEC);
              client.print("H");
              client.print(" ");
              client.print(waterDay, DEC);
              client.print("/");
              client.print(waterMonth, DEC);
             }
             client.print("<font color='black'>");
             client.println("<br />");
             client.println("<br />");
           
           
            client.println("<meta http-equiv=\"refresh\" content=\"10\">"); //refresh content every 5 sec.
	    client.println("</body></html>");
               
	    //clearing string for next read
	    readString="";
	    //stopping client
	    client.stop();
		}
	    }
        }
    }

  if (Light==0){
    int h = now.hour();
     if (h == 14){
      Sunrise();
     }
     if (h == 15 || h == 16 || h == 17 || h == 18 || h == 19 || h == 20){
      Light_On();
      }
     if (h==21){
      Sunset();
     }
     if (now.hour() >= 22 || now.hour() <= 11){
      Light_Off();  
     }
  }
  if (Light==1){
    Light_On();
  }
  if (Light==2){
    Light_Dim();
  }
  if (Light==3){
    Light_Off();
  }
     

//Noget med logging her! Læs om strings. 
  
}
float lightTemp(){
  int a=analogRead(0);
  int b;
  int B=3975; 
  float resistance=(float)(1023-a)*10000/a; 
  return (float) (1/(log(resistance/10000)/B+1/298.15)-273.15);
}
void Light_On(){
  analogWrite (hvid, 200);
  analogWrite (bla, 200);
  analogWrite (blas, 255);
}
void Light_Off(){
  analogWrite (hvid, 0);
  analogWrite (bla, 0);
  analogWrite (blas, 0);
}
void Sunset(){
  DateTime now = RTC.now();
  analogWrite (hvid, map(now.minute(), 0,59,200,0) );
  analogWrite (bla, map(now.minute(), 0,59,200,15) );
  analogWrite (blas, 255) ;
}
void Sunrise(){
  DateTime now = RTC.now();
  analogWrite (hvid, map(now.minute(), 0,59,0,200) );
  analogWrite (bla, map(now.minute(), 0,59,25,200) );
  analogWrite (blas, 255) ;
}
void Light_Dim(){
  analogWrite (hvid, 30);
  analogWrite (bla, 80);
}
