// Need G4P library
import g4p_controls.*;
import processing.serial.*;
String[] rawPortNames = Serial.list();
String[] portNames;
String[] inCommand;
StringDict hexDict;
String[] hexDictString;

Serial irPort;
boolean irPortOpen=false;
int lf = 10;      // ASCII linefeed

//extra gui elements
GDropList[] dropListX= new GDropList[4]; 
GOption optionX[] = new GOption[16]; 
GToggleGroup togGroupX; 
GLabel labelRemoteType[]= new GLabel[16];
GLabel labelButtonCode[]= new GLabel[16];
GLabel labelRepeat[]=new GLabel[16];
GLabel labelButtonCommand[]= new GLabel[16];
GLabel labelColumns[]= new GLabel[5];

public void setup()
{
  size(960, 700, JAVA2D);
  
  
  // Sort the port names giving prcedence to ttyACM? ttyUSB?
  portNames=new String[rawPortNames.length];
  int portNamesPointer=0;
  for (int i=0; i<rawPortNames.length; i++)
  {
    if( rawPortNames[i].indexOf("ACM") != -1 )
    {
      portNames[portNamesPointer]=rawPortNames[i];
      portNamesPointer++;
    }
  }
  for (int i=0; i<rawPortNames.length; i++)
  {
    if( rawPortNames[i].indexOf("USB") != -1 )
    {
      portNames[portNamesPointer]=rawPortNames[i];
      portNamesPointer++;
    }
  }
  for (int i=0; i<rawPortNames.length; i++)
  {
    if( rawPortNames[i].indexOf("ACM") == -1 && rawPortNames[i].indexOf("USB") == -1)
    {
      portNames[portNamesPointer]=rawPortNames[i];
      portNamesPointer++;
    }
  }
  //end of sort
  hexDictString=new String[0];
  
  setupDictionary();
  
  createGUI();
  customGUI();
  // Place your setup code here
 
  
  
  
}

public void draw()
{
  background(230);
  if (irPortOpen==true)
  {
    if (irPort.available() > 0) 
    {
      String myString = irPort.readStringUntil(lf);
      if (myString != null) 
      {
        //textarea1.appendText(myString+"\r\n");
        myString=myString.trim();
        println(myString);
        if (myString.indexOf("BUTTON:")!= -1)
        {
          println("Incoming Remote Command ["+myString+"]");
          //String[] 
          inCommand=split(myString.substring(myString.indexOf(":")+1),' ');
          //these are in hex values 
          for (int i=0; i<inCommand.length; i++)
          {
            println("C["+i+"]"+inCommand[i]);
          }
          remoteType.setText(inCommand[0]);
          buttonPressed.setText(inCommand[1]+" "+inCommand[2]);
          
        }
        if (myString.indexOf("AT+LEARN=")!=-1)
        {
          println("Incoming config["+myString+"]");
          String[] inConfig=split(myString.substring(myString.indexOf("=")+1),' ');
          int unHex=0;
          for (int i=0; i<inConfig.length-1; i++)
          {
            unHex=unhex(inConfig[i]);
            print("["+inConfig[i]+"]="+unHex+"=");
            print("["+hexDict.get(inConfig[i])+"] ");
          }
          println("=====================");
          print("pos="+inConfig[0]+" ");
          print("remoteType="+inConfig[1]+" ");
          print("remoteButton="+inConfig[2]+" "+inConfig[3]+" ");
          print("repeat="+inConfig[4]+" ");
          print("keys=");
          print(hexDict.get(inConfig[5])+" ");
          print(hexDict.get(inConfig[6])+" ");
          print(hexDict.get(inConfig[7])+" ");
          print(hexDict.get(inConfig[8])+" ");
          println(" ");
          
          int valuePosition=unhex(inConfig[0]);
          String valueString="";
          
          //valueString+=inConfig[1]+" ";
          //valueString+=inConfig[2]+inConfig[3]+" ";
          //valueString+="rep["+inConfig[4]+"] ";
          valueString+=hexDict.get(inConfig[5])+" ";
          valueString+=hexDict.get(inConfig[6])+" ";
          valueString+=hexDict.get(inConfig[7])+" ";
          valueString+=hexDict.get(inConfig[8])+" ";
          if (valuePosition<16)
          {
            optionX[valuePosition].setText(inConfig[0]);
            labelRemoteType[valuePosition].setText(inConfig[1]);
            labelRepeat[valuePosition].setText(inConfig[4]);
            labelButtonCode[valuePosition].setText(inConfig[2]+inConfig[3]);
            labelButtonCommand[valuePosition].setText(valueString);
          }
          
          println(valueString);
        }
        
      }
    }
  }
  label1.setText(str(frameCount));
}

// Use this method to add additional statements
// to customise the GUI controls
public void customGUI()
{
  dropList1.setItems(portNames,0);
  
  dropListX[0] = new GDropList(this, 20+(160*0), 160, 140, 360, 8);
  dropListX[0].setItems(hexDictString, 0);
  
  dropListX[1] = new GDropList(this, 20+(160*1), 160, 140, 360, 8);
  dropListX[1].setItems(hexDictString, 0);
  
  dropListX[2] = new GDropList(this, 20+(160*2), 160, 140, 360, 8);
  dropListX[2].setItems(hexDictString, 0);
  
  dropListX[3] = new GDropList(this, 20+(160*3), 160, 140, 360, 8);
  dropListX[3].setItems(hexDictString, 0);
  
  togGroupX = new GToggleGroup();

  
  //dropList1.addEventHandler(this, "dropList1_click1");
  
  int optionWidth=400;
  int optionHeight=20;
  
  labelColumns[0]= new GLabel(this, 35, 260, 30, 20);
  labelColumns[0].setText("Slot");
  labelColumns[0].setOpaque(false);

  labelColumns[1]= new GLabel(this, 60, 260, 60, 20);
  labelColumns[1].setText("Remote");
  labelColumns[1].setOpaque(false);

  labelColumns[2]= new GLabel(this, 105, 260, 60, 20);
  labelColumns[2].setText("Button");
  labelColumns[2].setOpaque(false);

  labelColumns[3]= new GLabel(this, 150, 260, 60, 20);
  labelColumns[3].setText("Repeat");
  labelColumns[3].setOpaque(false);

  labelColumns[4]= new GLabel(this, 210, 260, 60, 20);
  labelColumns[4].setText("Command");
  labelColumns[4].setOpaque(false);


  
  for (int i=0; i<16; i++)
  {
    optionX[i] = new GOption(this, 20, 280+(i*optionHeight), 60, 20);
    optionX[i].setTextAlign(GAlign.LEFT, GAlign.MIDDLE);
    optionX[i].setText("(null)");
    optionX[i].setOpaque(false);
    //option1.addEventHandler(this, "option1_clicked1");
    
    togGroupX.addControl(optionX[i]);
    //option1.setSelected(true);
    
    labelRemoteType[i] = new GLabel(this, 80, 280+(i*20), 30, 20);
    labelRemoteType[i].setText("00");
    labelRemoteType[i].setOpaque(false);

    labelButtonCode[i] = new GLabel(this, 110, 280+(i*20), 50, 20);
    labelButtonCode[i].setText("0000");
    labelButtonCode[i].setOpaque(false);


    labelRepeat[i] = new GLabel(this, 140, 280+(i*20), 80, 20);
    labelRepeat[i].setText("00");
    labelRepeat[i].setOpaque(false);


    labelButtonCommand[i] = new GLabel(this, 200, 280+(i*20), 240, 20);
    labelButtonCommand[i].setText("(null)");
    labelButtonCommand[i].setTextAlign(GAlign.LEFT, GAlign.MIDDLE);

    labelButtonCommand[i].setOpaque(false);

    
  }


  optionX[0].setSelected(true);//

}


public void sendProgrammingCommand()
{
  int programmingSlot=0;
  for (int i=0; i<16; i++)
  {
    if (optionX[i].isSelected()==true)
    {
      programmingSlot=i;
      println("Selected"+i);
    }
    
  }
  String todo="AT+LEARN=";
  todo+=optionX[programmingSlot].getText()+" ";
  todo+=remoteType.getText()+" ";
  todo+=buttonPressed.getText()+" ";
  todo+=keyRepeat.getText()+" ";
  
  String temps="";
  for (int i=0; i<4; i++)
  {
    temps=dropListX[i].getSelectedText();
    temps=temps.substring(0,2);
    todo+=temps+" ";
  }  
  todo+="\n";
  println("todo["+todo+"]");
  irPort.write(todo);
}

public void setupDictionary()
{
  hexDict = new StringDict();

  hexDict.set("00","(n)");
  hexDict.set("20","(space)");
  hexDict.set("21","!");
  hexDict.set("22","\"");
  hexDict.set("23","#");
  hexDict.set("24","$");
  hexDict.set("25","%");
  hexDict.set("26","&");
  hexDict.set("27","'");
  hexDict.set("28","(");
  hexDict.set("29",")");
  hexDict.set("2A","*");
  hexDict.set("2B","+");
  hexDict.set("2C",",");
  hexDict.set("2D","-");
  hexDict.set("2E",".");
  hexDict.set("2F","/");
  
  hexDict.set("30","0");
  hexDict.set("31","1");
  hexDict.set("32","2");
  hexDict.set("33","3");
  hexDict.set("34","4");
  hexDict.set("35","5");
  hexDict.set("36","6");
  hexDict.set("37","7");
  hexDict.set("38","8");
  hexDict.set("39","9");

  hexDict.set("3A",":");
  hexDict.set("3B",";");
  hexDict.set("3C","<");
  hexDict.set("3D","=");
  hexDict.set("3E",">");
  hexDict.set("3F","?");
  hexDict.set("40","@");
  
  hexDict.set("41","A");
  hexDict.set("42","B");
  hexDict.set("43","C");
  hexDict.set("44","D");
  hexDict.set("45","E");
  hexDict.set("46","F");
  hexDict.set("47","G");
  hexDict.set("48","H");
  hexDict.set("49","I");
  hexDict.set("4A","J");
  hexDict.set("4B","K");
  hexDict.set("4C","L");
  hexDict.set("4D","M");
  hexDict.set("4E","N");
  hexDict.set("4F","O");
  hexDict.set("50","P");
  hexDict.set("51","Q");
  hexDict.set("52","R");
  hexDict.set("53","S");
  hexDict.set("54","T");
  hexDict.set("55","U");
  hexDict.set("56","V");
  hexDict.set("57","W");
  hexDict.set("58","X");
  hexDict.set("59","Y");
  hexDict.set("5A","Z");
  
  hexDict.set("5B","[");
  hexDict.set("5C","\\");
  hexDict.set("5D","]");
  hexDict.set("5E","^");
  hexDict.set("5F","_");
  hexDict.set("60","`");

  hexDict.set("61","a");
  hexDict.set("62","b");
  hexDict.set("63","c");
  hexDict.set("64","d");
  hexDict.set("65","e");
  hexDict.set("66","f");
  hexDict.set("67","g");
  hexDict.set("68","h");
  hexDict.set("69","i");
  hexDict.set("6A","j");
  hexDict.set("6B","k");
  hexDict.set("6C","l");
  hexDict.set("6D","m");
  hexDict.set("6E","n");
  hexDict.set("6F","o");
  hexDict.set("70","p");
  hexDict.set("71","q");
  hexDict.set("72","r");
  hexDict.set("73","s");
  hexDict.set("74","t");
  hexDict.set("75","u");
  hexDict.set("76","v");
  hexDict.set("77","w");
  hexDict.set("78","x");
  hexDict.set("79","y");
  hexDict.set("7A","z");
  hexDict.set("7B","{");
  hexDict.set("7C","|");
  hexDict.set("7D","}");
  hexDict.set("7E","~");
  
  hexDict.set("80","KEY_LEFT_CTRL"); 
  hexDict.set("81","KEY_LEFT_SHIFT"); 
  hexDict.set("82","KEY_LEFT_ALT");   
  hexDict.set("83","KEY_LEFT_GUI");   
  hexDict.set("84","KEY_RIGHT_CTRL"); 
  hexDict.set("85","KEY_RIGHT_SHIFT");
  hexDict.set("86","KEY_RIGHT_ALT"); 
  hexDict.set("87","KEY_RIGHT_GUI");  
  hexDict.set("DA","KEY_UP_ARROW");   
  hexDict.set("D9","KEY_DOWN_ARROW"); 
  hexDict.set("D8","KEY_LEFT_ARROW"); 
  hexDict.set("D7","KEY_RIGHT_ARROW");
  hexDict.set("B2","KEY_BACKSPACE");
  hexDict.set("B3","KEY_TAB");   
  hexDict.set("B0","KEY_RETURN"); 
  hexDict.set("B1","KEY_ESC");      
  hexDict.set("D1","KEY_INSERT");   
  hexDict.set("D4","KEY_DELETE");   
  hexDict.set("D3","KEY_PAGE_UP");  
  hexDict.set("D6","KEY_PAGE_DOWN");
  hexDict.set("D2","KEY_HOME");
  hexDict.set("D5","KEY_END");
  hexDict.set("C1","KEY_CAPS_LOCK");
  hexDict.set("C2","KEY_F1");
  hexDict.set("C3","KEY_F2");
  hexDict.set("C4","KEY_F3");
  hexDict.set("C5","KEY_F4");
  hexDict.set("C6","KEY_F5");
  hexDict.set("C7","KEY_F6");
  hexDict.set("C8","KEY_F7");
  hexDict.set("C9","KEY_F8");
  hexDict.set("CA","KEY_F9");
  hexDict.set("CB","KEY_F10");
  hexDict.set("CC","KEY_F11");
  hexDict.set("CD","KEY_F12");
  hexDict.set("FF","(n)");

  println(hexDict);

  hexDictString=new String[hexDict.size()];

  for (int i=0; i<hexDict.size(); i++)
  {
    hexDictString[i]=hexDict.key(i)+"="+hexDict.value(i);
  }
  println(hexDictString);
}

//void serialEvent(Serial p) 
//{
//  String inString = p.readString();
//  textarea1.appendText("test" );
//  println("data received:["+inString+"]");
  
//}