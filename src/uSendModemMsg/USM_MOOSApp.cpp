//this program should allow us to send simple messages out through the modem. it requires iWhoiMicroModem to be running alongside it. it should also be launched in its own window
#include <iostream>
#include "USM_MOOSApp.h"
using namespace std;


USM_MOOSApp::USM_MOOSApp(){
  //default values, if not specified
  Dest = "8";
  Cmd = "WriteBinaryData";
  PacketType = "0";
  AckTimeoutSec = "0";
  
}


bool USM_MOOSApp::OnConnectToServer(){
  cout << "uSendModemMessage found the server" <<endl;
  return true;
}

bool USM_MOOSApp::OnStartUp(){
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
    
  }

    cout << "using " << Dest << " as the destination" << endl;
    cout << "using " << PacketType << " as the packet type" << endl;
    cout << "using " << AckTimeoutSec << " as the number of seconds to wait for acknowledgement" << endl; 
  return true;
}

bool USM_MOOSApp::OnDisconnectFromServer(){
  cout << "uSendModemMessage Disconnecting" <<endl;
  return true;

}

bool USM_MOOSApp::Iterate(){
  string msg;
  cout << endl << "enter your message, then press enter: ";
  getline(cin, msg);
  cout << "your message : " <<  msg << endl;

  if(PacketType == "2"){
    if (msg.length() > 63){
      cout << "the message exceeds 63 bytes, cannot send!" <<endl;
    }
    else if(msg.length() < 63){
      cout << "message less than 63 bytes, appending filler characters" << endl;
      int blank_spaces = 63 - msg.length();
      //cout << "blank_spaces =" << blank_spaces << endl; 
      string append_chars(blank_spaces, '-');
     
      msg = msg + append_chars;
    }
   
  }

  else if(PacketType == "0" || PacketType == "6"){
    if (msg.length() > 31){
      cout << "uSendModemMsg: the message exceeds 31 bytes, cannot send!" <<endl;
    }
    else if(msg.length() < 31){
      cout << "uSendModemMsg: message less than 31 bytes, appending filler characters" << endl;
      int blank_spaces = 31 - msg.length();
      //cout << "blank_spaces =" << blank_spaces << endl; 
      string append_chars(blank_spaces, '-');
     
      msg = msg + append_chars;
    }
  }

    cout << "uSendModemmsg: msg = "<< msg << endl;
    string payload = string_to_hex(msg);
    payload = "1E" + payload;
    cout << "uSendModemMsg: payload = " << payload << endl;
  
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
