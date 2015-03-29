/*==========================================================================
File: USM_MOOSApp.cpp
Authors: Nick Nidzieko & Sean Gillen
Date: Jan/22/15
Origin: Horn Point Laboratory
Description: Allows users to send modem messages over a WHOI micromodem
             needs to be running alongside iWhoiMicroModem, it should be
             run in it's own window

 Copyright 2015 Nick Nidzieko, Sean Gillen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

==========================================================================*/
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

//SG: got this from stack overflow, it does does some binary magic to convert to a string to hex
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
