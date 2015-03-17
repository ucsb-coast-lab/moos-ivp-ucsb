/*==========================================================================
File: USM_MOOSApp.h
Authors: Nick Nidzieko & Sean Gillen
Date: Jan/22/15
Origin: Horn Point Laboratory
Description: This program allows us to send simple messages out through 
             the modem. it requires iWhoiMicroModem to be running alongside
             it. It should also be launched in its own window.

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

#ifndef PSM_MOOSAPP_HEADER
#define PSM_MOOSAPP_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <string>
#include "MBUtils.h"
#include "AngleUtils.h"
class PSM_MOOSApp : public CMOOSApp  
{
public:
  PSM_MOOSApp();
  virtual ~PSM_MOOSApp() {};

  bool Iterate();
  bool OnConnectToServer();
  bool OnDisconnectFromServer();
  bool OnStartUp();
  bool OnNewMail(MOOSMSG_LIST&);

  void registerVariables();
  

  std::string Dest;
  std::string Cmd;
  std::string PacketType;
  std::string AckTimeoutSec;
  std::string var1;
  std::string msg1;

};

std::string string_to_hex(const std::string&);


#endif
