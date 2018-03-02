#include <SPI.h>
#include <mcp_can.h>

const int fdistance = 3;
const int lanewarn = 4;
const int relayH = 5;
const int relayL = 6;

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
int fdistanceState = 1;
int lanewarnState = 1;
MCP_CAN CAN(SPI_CS_PIN); 

int relaystate = 1; // 0 = DSU off


////SERIAL INPUT (UNUSED)
//char receivedChar;
 
void setup()
{ 
  pinMode(fdistance, INPUT_PULLUP);
  pinMode(lanewarn, INPUT_PULLUP);
  pinMode(relayH, OUTPUT);
  pinMode(relayL, OUTPUT);

  digitalWrite(relayH, relaystate); //Remember Relay is Backward - HIGH means off
  digitalWrite(relayL, relaystate);
  
  Serial.begin(115200);
 //SPI (ADA & CAN)
  SPI.begin();  
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  
 //CAN
    while (CAN_OK != CAN.begin(CAN_1000KBPS))              // init can bus : baudrate = 1000                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            00k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");

//  attachInterrupt(2, MCP2515_ISR, FALLING); // start interrupt for can
//  CAN.init_Mask(0, 0, 0x3ff);                         // there are 2 mask in mcp2515, you need to set both of them
//  CAN.init_Mask(1, 0, 0x3ff);
//  CAN.init_Filt(0, 0, 0x200); //0x200
}


////CAN INTERRUPT FUNCTION
//void MCP2515_ISR()
//{
//      flagRecv = 1;
//}
//
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
  lanewarnState = digitalRead(lanewarn);

  

  //CAN ----------------------------------------------------------------
//  if (relaystate == 0) {
  unsigned long currentMillis = millis(); // grab current time
  //check if "interval" time has passed (1000 milliseconds)
  if ((unsigned long)(currentMillis - previousMillis) >= interval) {

    if (fdistanceState == LOW) {
      JOEL_ID[0] = 0b10000000;
    } else {
      JOEL_ID[0] = 0b00000000;
    }


    if (lanewarnState == LOW) {
      JOEL_ID[1] = 0b10000000;
    } else {
      JOEL_ID[1] = 0b00000000;
    }
    CAN.sendMsgBuf(0x203, 0, 2, JOEL_ID);
    previousMillis = millis();
  }
//  }
  if(flagRecv)                   // check if get data
      {

        flagRecv = 0;                // clear flag
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        Serial.println("\r\n------------------------------------------------------------------");
        Serial.print("Get Data From id: ");
        Serial.println(CAN.getCanId());
        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print("0x");
            Serial.print(buf[i], HEX);
            Serial.print("\t");
        }
        Serial.println();

      }
}
