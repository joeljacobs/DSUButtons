#include <SPI.h>
#include <mcp_can.h>

//IO Pins
const int fdistance = 3;
const int lanewarn = 4;
const int relay1 = 5;
const int relay2 = 6;

//CAN
const int SPI_CS_PIN = 9;
unsigned char flagRecv = 0;
unsigned char len = 3;
unsigned char buf[8];
unsigned long interval=100; // the time we need to wait
unsigned long previousMillis=0; // millis() returns an unsigned long.
unsigned char stmp[5] = {0, 0, 0, 0, 0};
unsigned char JOEL_ID[2] = {0, 0};
unsigned char NEW_MSG_1[8] = {0};
MCP_CAN CAN(SPI_CS_PIN);
int canSendId = 0x203;

//Logic variables
int fdistanceState = 1;
int lanewarnState = 1;
int lane100mscount = 0;
int dist100mscount = 0;
int relaystate = 1; // 0 = DSU off

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
    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     00k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");

//  attachInterrupt(2, MCP2515_ISR, FALLING); // start interrupt for can
  CAN.init_Mask(0, 0, 0x3ff); // there are 2 mask in mcp2515, you need to set both of them
  CAN.init_Mask(1, 0, 0x3ff);
  CAN.init_Filt(0, 0, 0x2c1);
  CAN.init_Filt(1, 0, 0x2c1);
  CAN.init_Filt(2, 0, 0x2c1);
  CAN.init_Filt(3, 0, 0x2c1);
  CAN.init_Filt(4, 0, 0x2c1);
  CAN.init_Filt(5, 0, 0x2c1);
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
    if ((fdistanceState == LOW) && (dist100mscount == 50)) {
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


  if(flagRecv)                   // check if CAN receive is enabled
      {
        int CANID;
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        CANID=CAN.getCanId();
        if (CANID != canSendId) {
          Serial.println("\r\n------------------------------------------------------------------");
          Serial.print("Get Data From id: ");
          Serial.println(CANID);
         for(int i = 0; i<len; i++)    // print the data
          {
              Serial.print("0x");
             Serial.print(buf[i], HEX);
             Serial.print("\t");
          }
          Serial.println();
        }
      }
}
