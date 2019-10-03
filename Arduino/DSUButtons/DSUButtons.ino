#include <mcp_can_dfs.h>
#include <mcp_can.h>

#include <SPI.h>


//IO Pins
const int fdistance = 3;
const int lanewarn = 4;
const int relay1 = 5;
const int relay2 = 6;

//CAN
const int SPI_CS_PIN = 9;
unsigned char len = 3;
unsigned char buf[8];
unsigned long interval=100; // the time we need to wait
unsigned long previousMillis=0; // millis() returns an unsigned long.
unsigned char stmp[5] = {0, 0, 0, 0, 0};
unsigned char JOEL_ID[2] = {0, 0};
unsigned char NEW_MSG_1[8] = {0};
long unsigned int rxId;
unsigned char rxBuf[8];
MCP_CAN CAN(SPI_CS_PIN);
int canSendId = 0x203;

//Logic variables
int fdistanceState = 1;
int lanewarnState = 1;
int lane100mscount = 0;
int dist100mscount = 0;
int relaystate = 0; // 0 = DSU off?

////SERIAL INPUT (UNUSED)
//char receivedChar;
 
void setup()
{ 
  //GPIO
  pinMode(fdistance, INPUT);
  pinMode(lanewarn, INPUT_PULLUP);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, relaystate); //Remember Relay is Backward - HIGH means off
  digitalWrite(relay2, relaystate);

  //Serial 
  Serial.begin(115200);
  
 //SPI (CAN)
  SPI.begin();  
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
 //CAN
  if(CAN.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.print("MCP2515 Init Okay!!\r\n");
  else Serial.print("MCP2515 Init Failed!!\r\n");
  pinMode(2, INPUT);                       // Setting pin 2 for /INT input




  CAN.init_Mask(0,0,0x03FFFFFF);                // Init first mask...
  CAN.init_Mask(1,0,0x03FFFFFF);                // Init second mask... 
  CAN.init_Filt(0,0,0x04140000);                // Init first filter...
//  CAN.init_Filt(1,0,0x02010000);                // Init second filter...

  CAN.setMode(MCP_NORMAL);                // Change to normal mode to allow messages to be transmitted
}

////SERIAL INPUT (unused)
//void recvOneChar() {
//    if (Serial.available() > 0) {
//        receivedChar = Serial.read();
//    }
//}

//MAIN LOOP
void loop()
{

  fdistanceState = digitalRead(fdistance);
  //Serial.println(fdistanceState);
  lanewarnState = digitalRead(lanewarn);  

  //CAN Write SUB-LOOP
  // Only want to send messages every 100 ms
  unsigned long currentMillis = millis(); // grab current time
  //check if "interval" time has passed (100 milliseconds)
  if ((unsigned long)(currentMillis - previousMillis) >= interval) {

    if (fdistanceState == LOW) {
      JOEL_ID[0] = 0b10000000;
      dist100mscount++;
    } else {
      JOEL_ID[0] = 0b00000000;
      dist100mscount = 0;
    }


    if (lanewarnState == LOW) {
      JOEL_ID[1] = 0b10000000;
      lane100mscount++;
    } else {
      JOEL_ID[1] = 0b00000000;
      lane100mscount = 0;
    }
    CAN.sendMsgBuf(canSendId, 0, 2, JOEL_ID);
    //Serial.println(dist100mscount);
    if ((fdistanceState == LOW) && (dist100mscount == 10)) {
      dist100mscount = 0;
      if (relaystate == 0){
        relaystate = 1;
        Serial.println("relay=1");
        digitalWrite(relay1, relaystate);
        digitalWrite(relay2, relaystate);
      }
      else if (relaystate == 1) {
        relaystate = 0;
        Serial.println("relay=0");
        digitalWrite(relay1, relaystate);
        digitalWrite(relay2, relaystate);
      }
    }
    previousMillis = millis();
  }

  if(!digitalRead(2))                   // check if CAN receive is enabled
      {
      CAN.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
//        Serial.print("ID: ");
//        Serial.print(rxId, HEX);
//        Serial.print(" Data: ");
//        for(int i = 0; i<len; i++)           // Print each byte of the data
//        {
//          if(rxBuf[i] < 0x10)                // If data byte is less than 0x10, add a leading zero
//          {
//            Serial.print("0");
//          }
//          Serial.print(rxBuf[i], HEX);
//          Serial.print(" ");
//        }
//        Serial.println();
        if(rxBuf[6] == 0x16) {
          Serial.println("EON NOT ON - Stock!");
        }
      }

 // pseudo code:
 // Start with DSU off.
 // if we don't see an EON packet in x seconds, turn DSU on. (x is the maximum the DSU can stay off before a force-restart error occurs)
 // Make the change permanent until next restart of car. Can stop looking for ID at that point.
 // future: send a can packet to switch the DIPs as well
 // and EON packet is defined as an id and data that only the EON puts out. So it can be both an ID and data -since most, if not all
 // id's are common with the EON and the DSU. So we'd look at some of the "values.py" values that the EON always sends as an example packet.
}
