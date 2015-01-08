//this program should allow us to send simple messages out through the modem. it requires iWhoiMicroModem to be running alongside it. it should also be launched in its own window
#include <iostream>
#include "PSM_MOOSApp.h"
using namespace std;


PSM_MOOSApp::PSM_MOOSApp(){
  //default values, if not specified
  Dest = "8";
  Cmd = "WriteBinaryData";
  PacketType = "0";
  AckTimeoutSec = "0";
  
}


bool PSM_MOOSApp::OnConnectToServer(){
  cout << "uSendModemMessage found the server" <<endl;
  return true;
}

bool PSM_MOOSApp::OnStartUp(){
  cout << "uSendModemMessage started up ok" << endl;

  STRING_LIST sParams;
    m_MissionReader.GetConfiguration(GetAppName(), sParams);
    
  STRING_LIST::iterator p;
    for(p = sParams.begin();p!=sParams.end();p++) {
      string sLine  = *p;
      string param  = stripBlankEnds(MOOSChomp(sLine, "="));
      string value  = stripBlankEnds(sLine);
    //double dval   = atof(value.c_str());

    param = toupper(param);

    if(param == "DEST"){
      Dest = value;
    }
    // if(param == "Cmd"){ //different commands may be implemented later if need be
    //  Cmd = value;
    // }
    if(param == "PACKETTYPE"){
      PacketType = value;
    }
    if(param == "ACKTIMEOUTSEC"){
      AckTimeoutSec = value;
    }
    if(param == "VAR1"){
      var1 = value;
    }
    
  }

    cout << "using " << Dest << " as the destination" << endl;
    cout << "using " << PacketType << " as the packet type" << endl;
    cout << "using " << AckTimeoutSec << " as the number of seconds to wait for acknowledgement" << endl; 
    cout << "will be sending the variable published under : " << var1 << " over the modem" << endl;

    registerVariables();
  return true;
}


//------------------------------------------------------------------------
// Procedure: registerVariables()
//      Note: 

void PSM_MOOSApp::registerVariables()
{
  m_Comms.Register(var1, 0);

}  

//------------------------------------------------------------------------
// Procedure: OnNewMail
//      Note: reads messages from MOOSDB to update values

bool PSM_MOOSApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
  CMOOSMsg Msg;
  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &Msg = *p;
    string key = Msg.m_sKey;
    double dval = Msg.m_dfVal;
    string sval = Msg.m_sVal;

    if(key == var1) {
      msg1 = "set " + var1 + "=" + sval;
      cout << "msg1 : " << msg1 << endl;
      cout << "var1 : " << var1 << endl;
      cout << "sval : " << sval << endl;
    }
  }
  return(true);
}


//-----------------------------------------------------------------------

bool PSM_MOOSApp::OnDisconnectFromServer(){
  cout << "pSendModemMessage Disconnecting" <<endl;
  return true;

}


//-----------------------------------------------------------------------
bool PSM_MOOSApp::Iterate(){
  //string msg = var1;

  if(PacketType == "2"){
    if (msg1.length() > 63){
      cout << "the message exceeds 63 bytes, cannot send!" <<endl;
    }
    else if(msg1.length() < 63){
      cout << "message less than 63 bytes, appending filler characters" << endl;
      int blank_spaces = 63 - msg1.length();
      //cout << "blank_spaces =" << blank_spaces << endl; 
      string append_chars(blank_spaces, '-');
     
      msg1 = msg1 + append_chars;
    }
   
  }

  else if(PacketType == "0" || PacketType == "6"){
    if (msg1.length() > 31){
      cout << "pSendModemMsg: the message exceeds 31 bytes, cannot send!" <<endl;
    }
    else if(msg1.length() < 31){
      cout << "pSendModemMsg: message less than 31 bytes, appending filler characters" << endl;
      int blank_spaces = 31 - msg1.length();
      //cout << "blank_spaces =" << blank_spaces << endl; 
      string append_chars(blank_spaces, '-');
     
      msg1 = msg1 + append_chars;
    }
  }

    cout << "uSendModemmsg: msg = "<< msg1 << endl;
    string payload = string_to_hex(msg1);
    payload = "1E" + payload;
    cout << "pSendModemMsg: payload = " << payload << endl;
  
  Notify("iWhoiMicroModem_CMD", "Cmd=" + Cmd +",Dest=" + Dest + ",PacketType=" + PacketType + ",AckTimeoutSec=" + AckTimeoutSec + ",Payload=" + payload); 
  return true;
}


std::string string_to_hex(const std::string& input)
{
  static const char* const lut = "0123456789ABCDEF";
  size_t len = input.length();

  std::string output;
  output.reserve(2 * len);
  for (size_t i = 0; i < len; ++i)
    {
      const unsigned char c = input[i];
      output.push_back(lut[c >> 4]);
      output.push_back(lut[c & 15]);
    }
  return output;
}
