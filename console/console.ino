//************************************************************
// This is console part**
//
//************************************************************

#include "painlessMesh.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <console.h>
#include <painlessMesh.h>
#include <painlessMeshSync.h>
#include <painlessScheduler.h>
#include <TimeLib.h>


#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555


#define   BTN             12
#define   LED             16

//*************************
#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);
const int type = 2;

int alarm = 16;
bool isFire = false;
bool isDoor = false;
//bool isDoor = true;

int reading;
float voltage;
float tempF;
int tempFint;

int upPin = 14;
int downPin = 12;
int idealTemp = 65;

int blrState = 0;

uint32_t door_id;
time_t door_start = now();

String device_name = "Heng's room";
uint32_t device_id = ESP.getChipId();

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//*************************

void sendMessage() ; // Prototype so PlatformIO doesn't complain

painlessMesh  mesh;
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

// Variables for my bells and whistles
bool myLight = false;
bool yourLight = false;


void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& myRequest = jsonBuffer.parseObject(msg);
  int type_recived = int(myRequest["type"]);
  if(type_recived == 1)
  {
    isFire = int(myRequest["data"]);
    return;
  }

  if(type_recived == 3)
  {
    blrState = int(myRequest["data"]);
    return;
  }

  if(type_recived == 4)
  {
      door_id = uint32_t(myRequest["data"]);
      if(door_id == device_id && isDoor == 0)
      {
        isDoor = 1;
        door_start = now();
      }
      
  }

  
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections %s\n",mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT);
  attachInterrupt(BTN, changeLight, RISING);

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.scheduler.addTask( taskSendMessage );
  taskSendMessage.enable() ;

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  pinMode(alarm,OUTPUT);
  digitalWrite(alarm,LOW);
  pinMode(upPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(upPin),upTemp,FALLING);
  attachInterrupt(digitalPinToInterrupt(downPin),downTemp,FALLING);
  //setTime(now);
}

void changeLight(){
  yourLight = !yourLight;
  return;
}

void doorBellRing()
{
  digitalWrite(alarm,HIGH);
  delay(3000);
  digitalWrite(alarm,LOW);
  delay(2000);
  
}

void fireAlarmOn()
{
  digitalWrite(alarm,HIGH);
  testfillrect();
}
void fireAlarmOff()
{
  digitalWrite(alarm,LOW);
}

void testfillrect() {
  Serial.println("Inside rect");
  uint8_t color = 1;
  for (int16_t i=0; i<display.height()/2; i+=3) {
    // alternate colors
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
    display.display();
    delay(1);
    color++;
  }
  delay(1000);
}

int door_counter = 0;

void loop() {
  mesh.update();

  int tempF = getTemp();
    
    if(isFire)
    {
      fireAlarmOn();
      return;
      
    }

    if(!isFire)
    {
      fireAlarmOff();
      
    }

    // doorbell ring for 10 seconds
    if(isDoor && (now() - door_start) < 10 )
    {
       //Serial.print("Now  ");
       //Serial.println(now()-door_start);
       digitalWrite(alarm,HIGH);
      
    }
    else
    {
      isDoor = 0;
      digitalWrite(alarm,LOW);
    }
    
    String blr_disState = "NA";
    if(blrState == 1)
    {
      blr_disState = "ON";
    }
    else
    {
      blr_disState = "OFF";
    }
    
    //Serial.println("Temp F");
    //Serial.println(tempF);
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    display.print(idealTemp);
    /*
    display.setTextSize(1);
    display.print("        BLR: ");
    display.print(blr_disState);
    display.setTextSize(2);
    display.println();
    */
    display.println("     "+blr_disState);
    
    display.setTextSize(6);
    //display.print(" ");
    display.print(tempF);
    display.display();

}

void upTemp()
{
  //Serial.println("UpTemp called");
  //isFire = true; 
  // *********** Testing
  
  idealTemp++;
}
void downTemp()
{
  //isFire = false;
  // *************Testing
  idealTemp--;
}

int getTemp()
{
    reading = analogRead(A0);
    /*
    Serial.print("***");
    Serial.println(reading);
    Serial.println("mv");
    Serial.println(idealTemp);
    Serial.println("ideal Temp");
    */
    float volt = reading * 3.0;
    //float volt = reading;
    volt /= 1024.0;
    float tempC = (volt - 0.5)*100;

    int tempF = tempC * 9.0/5.0 +32.0;
    //Serial.println("Temp F");
    return tempF;
}

void sendMessage() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& myRequest = jsonBuffer.createObject();
  myRequest["requestTemp"] = idealTemp;
  myRequest["data"] = getTemp();
  myRequest["device_id"] = ESP.getChipId();
  myRequest["type"] = 2;
  myRequest["device_name"] = device_name;
  String request;
  myRequest.printTo(request);
  Serial.print("Sending: ");
  Serial.println(request);
  if(!mesh.sendBroadcast( request ))
    Serial.println("Message Failed");
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}
