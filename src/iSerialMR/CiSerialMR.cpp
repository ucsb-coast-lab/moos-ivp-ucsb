// This is a mashup/mod of the iMOOS2Serial class 
// written by Andrew Patikalakis.
// This code provides serial comms with an Arduino Uno, 
// for the purpose of toggling power to a Rockland MicroRider.
//
// nidzieko@umces.edu
// 18March2015
//
// CiSerialMR.cpp: implementation of the CiSerialMR class.
////////////////////////////////////////////////////////

#include <iterator>
#include "CiSerialMR.h"
#include "dtime.h"
#include <signal.h>

bool sigflag = false;

void sigh(int)
{
	sigflag = true;
}

CiSerialMR::CiSerialMR()
{
	// constructor
	signal(SIGINT, sigh);
	signal(SIGTERM, sigh);
}

CiSerialMR::~CiSerialMR()
{
  // destructor
}

bool CiSerialMR::OnConnectToServer()
{
	// register for variables here
	// possibly look at the mission file?
	// m_MissionReader.GetConfigurationParam("Name", <string>);
	// m_Comms.Register("VARNAME", <max frequency at which to get
	//                             updates, 0 = max>);
	// note, you cannot ask the server for anything in this function yet
     	string pt; int bd;
	
	m_MissionReader.GetConfigurationParam("Port", pt);
	m_MissionReader.GetConfigurationParam("Speed", bd);

        bbb = new CSerMR(pt, bd);
	bbb->SetCB(tramp, this);

	m_MissionReader.GetConfigurationParam("InputVariable", invar);

	m_Comms.Register(invar, 0);
	m_Comms.Register("DESIRED_THRUST", 0);
	m_Comms.Register("DESIRED_RUDDER", 0);

	return true;
}

bool CiSerialMR::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;
  
  for(p = NewMail.begin(); p != NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    // This is roughly the RECON Transmit function. 
    // It should be broken out into its onw call eventually.
    int rc, length, i, chksum = 0, maxlen;
    char buf[MAXBUFLEN];
    //    for (int i = 0; i < MAXBUFLEN; i++) {
    //  buf[i] = '\0';
    //}

//    printf("New Mail (getkey): %s\n", msg.GetKey().c_str());
//no	   printf("New Mail (m_skey.cstr): %s\n", msg.m_sKey().c_str());
//no	   printf("New Mail (m_skey): %s\n", msg.m_sKey());
//    printf("New Mail (getdouble): %f\n", msg.GetDouble());
//ok	   printf("New Mail (m_dfVal): %f\n", msg.m_dfVal);
    
    if (strcmp(msg.GetKey().c_str(), "DESIRED_THRUST") == 0) {
      length = sprintf(buf,"#C,Thrust,%f,*",msg.GetDouble());
    }
    else if (strcmp(msg.GetKey().c_str(), "DESIRED_RUDDER") == 0) {
      length = sprintf(buf,"#C,Rudder,%f,*",msg.GetDouble());
    }
    // Check for buffer overflow
    maxlen = MAXBUFLEN - 4;

    if ((length == -1) || (length >= maxlen))
      {
	// Not enough space left                                                                   
	return false;
      }

    // Do checksum                                                                               
    for (i = 0; i < length; i++)
      {
	chksum += buf[i];
      }

    // Finish message                                                                            
    rc = snprintf(&(buf[length]), 4, "%02X\n", 0xff & chksum);
    if (rc != 3)
      {
	MOOSTrace("WARNING - Appending checksum failed\n");
	return false;
      }
    length += rc;

    printf("%s",buf);
    bbb->AppendWriteQueue(buf,length);
    // bbb->AppendWriteQueue("\n",1);
    bbb->BlockingWrite();
  }
  NewMail.clear();
  
  return true;
}



bool CiSerialMR::handle(string s)
{
  // printf("S: %s\n", s.c_str());
  
  // s += "\r\n";
  
  string ss = s.substr(1);
  printf("raw: %s\n",ss.c_str());	
  int token = ss.find("=");
  string param = ss.substr(0, token);
  string value = ss.substr(token + 1);
  
  //printf("value: %s\n",value.c_str());
  //printf("param: %s\n",param.c_str());
  if (strcmp(param.c_str(),"IMU_HEADING") == 0) {
        m_Comms.Notify("IMU_HEADING", atof(value.c_str()));
  }
  if (strcmp(param.c_str(),"IMU_SPEED") == 0) {
        m_Comms.Notify("IMU_SPEED", atof(value.c_str()));
  }

}



bool CiSerialMR::Iterate()
{
	// happens AppTick times per second
	
  //	bbb->BlockingWrite();

	if(sigflag == true) { // time to exit
	  //		delete port;
		exit(1);
	}
	
	return true;
}

bool CiSerialMR::OnStartUp()
{
	// happens after connection is completely usable
	// ... not when it *should* happen. oh well...
	
	return true;
}

