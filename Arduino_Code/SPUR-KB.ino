//Copyright 2016 CodeMonkey1100001001 2016.05.06
//Released under MIT License https://opensource.org/licenses/MIT
#include <EEPROM.h>
#include <IRLib.h>

byte EEPROMWIDTH=8;//how wide are the control + sequences
byte availableSpot=0;

char incomingData[256];//incoming serial commands
int  incomingDataPointer=0x00;
int  incomingDataSize=256;
byte ATCommandReady=0x00;
byte incomingByte=0x00;

//config
byte dumpRAW=0;
byte showEcho=1;
char newLineChar='\n';

//these must always stay in order 
//they are used in a swtich case below
#define ATCommandCount 8
char *ATCommands[ATCommandCount] = {
"+HELO", //0
"?", //1
"+LEARN", //2
"+DUMP", //3 note if two similar commands match the longer one must come first
"+RAW",   //4
"-RAW", //5
"+ERASE", //6
"+LIST"//7
};

// IR Remote
int RECV_PIN = 11;
IRrecv IRReceiver(RECV_PIN);
IRdecode IRDecoder;
unsigned int Buffer[RAWBUF];

//Intervals
unsigned long previousMillis = 0; 
unsigned long repeatInterval = 10; //373;
unsigned long currentMillis  = 0;
byte okToRepeat=1;
int lastCommand=0;
int currentCommand=0;

void setup() 
{
  Serial.begin(9600);
  Keyboard.begin();
  delay(5000);//while(!Serial);//delay for Leonardo can take quite a while for sketch to actually start.
  Serial.println(F("AT Command Receiver"));
  Serial.println(F("Version 2016/05/06 0.000"));
  Serial.println(F("OK"));


  IRReceiver.enableIRIn(); // Start the receiver
  IRDecoder.UseExtnBuf(Buffer);
  
//  for (int i=0; i<512; i++) fakeEEPROM[i]=0x00;
  
}

void loop() 
{
  currentMillis=millis();

  if (Serial.available() >0)
  {
    incomingByte=Serial.read();
    if (showEcho==1) Serial.write(incomingByte);
    incomingByte=toupper(incomingByte); // case insenstive always upper case
    if (incomingByte==newLineChar) ATCommandReady=1;
    incomingData[incomingDataPointer]=incomingByte;
    incomingDataPointer++;
    if (incomingDataPointer>=incomingDataSize) incomingDataPointer=incomingDataSize-1;
  }
  if (ATCommandReady==1)
  {
    int success=parseATCommand();
    incomingDataPointer=0;
    ATCommandReady=0;
    if (success==1) Serial.println(F("OK"));
    else Serial.println(F("ERROR"));
  }


  if (IRReceiver.GetResults(&IRDecoder)) 
  {
    //Restart the receiver so it can be capturing another code
    //while we are working on decoding this one.
    IRDecoder.decode();
    IRReceiver.resume(); 

    //if this is the same as last one
    //and the previous timeout has not been met
    //then dont act on it.
    
    if (dumpRAW==1) IRDecoder.DumpResults();
    if (currentMillis - previousMillis >= repeatInterval) 
    {
      okToRepeat=1;
    }
    
    currentCommand=IRDecoder.value;
    
    if (lastCommand != currentCommand)
    {
      okToRepeat=1;
      lastCommand=currentCommand;
    }
    
    //Go through eeprom and see if this is a match
    for (int x=0; x<512; x+=EEPROMWIDTH)
    {
      if (EEPROM.read(x)!=0xFF)
      {
        if (dumpRAW==1)
        {
          Serial.print(F("Spot "));
          Serial.print(x/8,DEC);
          Serial.print(F("="));
          for (int i=0; i<8; i++)
          {
            printHexFlat(EEPROM.read(x+i),2);
          }
        }
        if ( (EEPROM.read(x+0)==IRDecoder.decode_type) && (EEPROM.read(x+1)==(byte)(IRDecoder.value>>8)) && (EEPROM.read(x+2)==(byte)(IRDecoder.value)) )
        {
          if (okToRepeat==1)
          {
            if (dumpRAW==1) Serial.print(F(" PLAY COMMAND "));
            play_command(x);//give pointer to eeprom location
            previousMillis=currentMillis;
            okToRepeat=0;
          }
          else
          {
            if (dumpRAW==1) Serial.print(F(" SKIPPING COMMAND "));
          }
        }
        if (dumpRAW==1) Serial.println("");


      }//if there is a record
      
      if (EEPROM.read(x)==0xFF && availableSpot==0) availableSpot=x/EEPROMWIDTH;
    }//for x to eepromsize
    
    
    Serial.print(F("Remote Type ["));
    Serial.print(IRDecoder.decode_type,HEX);
    Serial.print(F("] "));
    Serial.print(F("Remote Value ["));
    printHex(IRDecoder.value,4);
    Serial.println(F("]"));
    
    Serial.print(F("+BUTTON:"));
    //Serial.print(IRDecoder.decode_type,HEX);
    printHexFlat(IRDecoder.decode_type,2);
    Serial.print(F(" "));
    //Serial.print(IRDecoder.value,HEX);
    printHexFlat(IRDecoder.value>>8,2);
    Serial.print(F(" "));
    printHexFlat(IRDecoder.value & 0x00FF,2);
    
    Serial.println();
    
    Serial.print(F("Example: at+learn="));
    printHexFlat(availableSpot,2);
    Serial.print(F(" "));
    printHexFlat(IRDecoder.decode_type,2);
    Serial.print(F(" "));
    printHexFlat(IRDecoder.value>>8,2);
    Serial.print(F(" "));
    printHexFlat((byte)(IRDecoder.value),2);
    Serial.println(F(" FE 40 === slot type val-val repeat=FE*10ms command=@"));
    availableSpot=0;
    
    
  }//  if (My_Receiver.GetResults(&IRDecoder)) 




}//loop
void play_command(int startPos)
{
  Serial.print("Playing -----=======[");
  for (int i=startPos+4; i<startPos+8; i++)
  {
    printHex(EEPROM.read(i),2);
    Serial.print(F(" "));
    if (EEPROM.read(i)>0x00) Keyboard.press(EEPROM.read(i));
  }
  Serial.println("]=======-------");
  delay(10);
  Keyboard.releaseAll();
}

int parseATCommand()
{
  int retV=-1;
  int ATCommandMatched=0;
  int parsePointer=0x00;

    //The first 2 chars of the incoming data must be AT
    if ( strncmp(incomingData,"AT",2)==0 ) 
    {
       //is it just an AT\r\n
      if (incomingData[2]=='\r' || incomingData[2]=='\n') ATCommandMatched=1;
      retV=1;//just return OK
      parsePointer=2; // skip the AT
      
      for (int lop=0; lop< ATCommandCount; lop++)//loop over all the known commands
      {
        int returnVal=compareCommand(ATCommands[lop],incomingData,parsePointer,strlen(ATCommands[lop]));
        if (returnVal==1) 
        { 
          ATCommandMatched=1;
          parsePointer=parsePointer+strlen(ATCommands[lop]);
          int getSet=0;// 0=get 1=set
          if (incomingData[parsePointer]=='=') getSet=1;//this will effectively ignore ? if present
          parsePointer++;
          
          switch (lop)
          {
            case 0: //+HELO
              //retV=atCommand_CCLK(getSet,parsePointer,incomingDataPointer);
              Serial.println("Hello AT Reader");
              break;
            case 1: //?
              retV=ATCommand_KNOWN(getSet,parsePointer,incomingDataPointer);
              break;
            case 2://+NULL
              //do nothing
              retV=ATCommand_LEARN(getSet,parsePointer,incomingDataPointer);
              break;
            case 3: //+DUMP
              retV=ATCommand_DUMP (getSet,parsePointer,incomingDataPointer);
              break;
            case 4: //+RAW
              Serial.println(F("+RAW=TRUE"));
              dumpRAW=1;
              break;
            case 5: //-RAW
              Serial.println(F("+RAW=FALSE"));
              dumpRAW=0;
              break;
            case 6: //+ERASE
              Serial.println(F("+ERASE=0xFF"));
              retV=ATCommand_ERASE(getSet,parsePointer,incomingDataPointer);
              break;
            case 7 : //+LIST
              retV=ATCommand_LIST(getSet,parsePointer,incomingDataPointer);
              break;            
              
          }//switch
          lop=ATCommandCount;
          //retV=1;// OK or ERROR
        }//if returnVal==1
        
       }//match a command;
       if (ATCommandMatched==0) { Serial.println(F("No command match")); retV=-1;}
    }//was at matched
  return retV;
}//parseATCommand()



int ATCommand_LEARN(int getSet,int startPtr, int endPtr)
{
  int inval=0x00;
  //AT+LEARN=016100  // spot 01 = A
  //AT+LEARN=00 80 61 41   /// store 8061 in pos 0 to play 41='A'
  //AT+LEARN=01  80 61         00 01 02 03  // spot 01 = KEY_LEFT_CTRL	, A // hex values
  //         pos remote val    sequence
  int spot=get2ByteValHex(startPtr); startPtr+=3;
  int remoteType=get2ByteValHex(startPtr); startPtr+=3;
  int remoteVal=get2ByteValHex(startPtr)<<8; startPtr+=3;
  remoteVal=remoteVal+get2ByteValHex(startPtr); startPtr+=3;
  int repeatValue=get2ByteValHex(startPtr); startPtr+=3;
  
  if (remoteVal==0x00) return -1;
  
  Serial.print(F("spot="));
  Serial.println(spot,HEX);
  Serial.print(F("remoteType="));
  Serial.print(remoteType,HEX);
  Serial.print(F(" remoteVal="));
  Serial.print(remoteVal,HEX);
  Serial.print(F(" repeatValue="));
  Serial.println(repeatValue,HEX);  
  
  
  Serial.print(F("Sequence="));


  
  if (true)
  {
    Serial.println(F("---=== WRITING TO EEPROM ===---"));
    int eepromStart=spot*EEPROMWIDTH;
    EEPROM.write(eepromStart+0,(byte)(remoteType));
    EEPROM.write(eepromStart+1,(byte)(remoteVal>>8));
    EEPROM.write(eepromStart+2,(byte)(remoteVal));
    EEPROM.write(eepromStart+3,(byte)(repeatValue));

    int commandCount=0;
    for (int x=startPtr; x<endPtr-2; x+=3)
    {
        inval=get2ByteValHex(x);
        //printHex(incomingData[x],2);
        printHex(inval,2);
        Serial.print(" ");
        if (4+commandCount < EEPROMWIDTH) 
        {
          EEPROM.write(eepromStart+4+commandCount,(byte)(inval));
        }
        commandCount++;
    }
    while (commandCount+4 <= EEPROMWIDTH-1) // fill it out with 0x00
    {
      EEPROM.write(eepromStart+4+commandCount,(byte)(0x00));
      commandCount++;
    }

    Serial.println("");  
 
    
  }
  
  //keyCode[0] will equal the previous remote code
  //keyValue[] will = 'A'
  
  int retV=1;
  return retV;
}//ATCommand_RAND

int ATCommand_LIST(int getSet,int startPtr, int endPtr)
{
  int retV=1;

  for (int x=0; x<512; x+=EEPROMWIDTH)
  {
    Serial.print(F("AT+LEARN="));
    //Serial.print(x/8,DEC);
    printHexFlat(x/8,2);
    Serial.print(F(" "));
    for (int i=0; i<8; i++)
    {
      printHexFlat(EEPROM.read(x+i),2);
      Serial.print(F(" "));
    }
    Serial.println("");

  }//for x to eepromsize
  return retV;
}//ATCommand_LIST


int ATCommand_DUMP(int getSet,int startPtr, int endPtr)
{
  int retV=1;
  byte value;
  int address = 0;
  
  //Serial.print("EEPROM Size:");//only in arduino 1.6.2 >
  //Serial.println(EEPROM.length,DEC);
  
  for (int w=0; w<16; w++)
  {
    printHexFlat(w,2);
    Serial.print(F(" "));
  }
  Serial.println("");
  Serial.println(F("-----------------------------------------------"));
  for (int w=0; w<512/16; w++)
  {  
    for (int i=0; i<16; i++)
    {
  
      value = EEPROM.read(address);
      printHexFlat(value, 2);
      Serial.print(",");
      address = address + 1;
    }
    Serial.println("");
  }
  return retV;
}//ATCommand_DUMP

int ATCommand_ERASE(int getSet,int startPtr, int endPtr)
{
  int retV=1;
  byte value;
  int address = 0;
  for (int i=0; i<512; i++)
  {
    EEPROM.write(i,0xFF);
  }  
  return retV;
}//ATCommand_ERASE




int ATCommand_KNOWN(int getSet,int startPtr, int endPtr)
{
  int retV=1;
  
  Serial.println(F("Known Commands:"));
  for (int lop=0; lop< ATCommandCount; lop++)//loop over all the known commands
  {
    Serial.println(ATCommands[lop]);
  }
  
  return retV;
}//ATCommand_RAND

int compareCommand(char *needle, char *haystack,int startPos, int compareLen)
{
  int retV=0;
  
  for (int i=0; i< compareLen; i++)
  {
    //Serial.print("needle="); Serial.print(needle[i]);
    //Serial.print(" hay="); Serial.print(haystack[i+startPos]);
    //Serial.println("");
    if (needle[i]==haystack[i+startPos]) retV++;
  }
  
  //Serial.println("-=--");
  
  if (compareLen==retV) retV=1;
    else retV=0;
  
  return retV;
}

void printHex(int num, int precision) 
{
      char tmp[16];
      char format[128];

      sprintf(format, "0x%%.%dX", precision);

      sprintf(tmp, format, num);
      Serial.print(tmp);
}
void printHexFlat(int num, int precision) 
{
      char tmp[16];
      char format[128];

      sprintf(format, "%%.%dX", precision);

      sprintf(tmp, format, num);
      Serial.print(tmp);
}

int get2ByteValHex(int startPtr)
{//hex values should be 00 FF 00 FF 
  int retV=0x00;
  byte digit1=incomingData[startPtr+0];
  byte digit2=incomingData[startPtr+1];
  
  if (!( (digit1>='0' && digit1 <='9') || (digit1>='A' && digit1<='F') ) ) return 0x00;
  if (!( (digit2>='0' && digit2 <='9') || (digit2>='A' && digit2<='F') ) ) return 0x00;
  
  if (digit1>='A') digit1=digit1-'A'+10;
  else { digit1=digit1-'0';}
  
  if (digit2>='A') digit2=digit2-'A'+10;
  else { digit2=digit2-'0';}
  
  
  retV= (digit1*16)+digit2;
  return retV;
}
