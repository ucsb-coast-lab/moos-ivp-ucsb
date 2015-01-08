// Recon.cpp
//
// 17oct2013: added additional parser for CTD messages (njn)
// 2dec2013: Added flags for RECON_ENGAGED and REMUS_DEPLOYED, 
//           with appropriate keep-alive heartbeat as needed.

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <string>
#include <math.h>
#include <sys/time.h>
#include "Recon.h"
using namespace std;

// Max rate to receive MOOS updates (0 indicates no rate, get all)
#define DEFAULT_REGISTER_RATE 0.0

// Enable/disable debug code
#define DEBUG 0

// Time stamp tracking tolerance
#define REMUS_TIME_TOLERANCE 1

// Allow pthread_create() to call class member
void *ReconRecvHandoff(void *arg)
{
  return ((Recon*)arg)->ReconRecvLoop();
}

Recon::Recon()
{
  // initialize internal state
  reconActive = false;
  wantReconActive = false;
  vCourse = 0.0;
  vSpeed = 0.0;
  vDepth = 0.0;
  recvNewCourse = false;
  recvNewSpeed = false;
  recvNewDepth = false;
  tDepth = 3.0;
  tAltitude = 3.0;
  tRate = 10.0;
  tMaxD = 30.0;
  triangleMode = false;
  
  allowKeepAlive = true;
  activateKeepAlive = true;
  rpmSpeedMode = false;
  
  // default ready signalling, true/false to RECON_ENGAGED
  readyVar = "RECON_ENGAGED";
  readyVarIsNumeric = false;
  
  // time stamp tracking
  remusTimeValid = false;
  
  // NTP Reference Init
  ntpRefMaxSkew = -1;

  // Initialize timer for sending NODE_REPORTS
  time(&node_timer);
  node_interval = 10.0;

}

//----------------------------------------------------
// Procedure: angle360
//   Purpose: Convert angle to be strictly in the rang [0, 360).

double Recon::angle360(double degval)
{
  while(degval >= 360)
    degval -= 360.0;
  while(degval < 0)
    degval += 360.0;
  return(degval);
}


void Recon::DoRegistrations(double updateRate)
{
  // MOOSDB variables we're interested in
  
  // Commanded vehicle state
  m_Comms.Register("DESIRED_HEADING", updateRate);
  m_Comms.Register("DESIRED_SPEED", updateRate);
  m_Comms.Register("DESIRED_DEPTH", updateRate);
  m_Comms.Register("TRIANGLE_MODE", updateRate);  //NJN:20141118
  
  // Tells us whether IvP should be active
  m_Comms.Register("MOOS_MANUAL_OVERIDE", updateRate);
  
  // Provide interface to allow other folks to control RECON
  m_Comms.Register("RECON_ENGAGE", 0.0);
  m_Comms.Register("RECON_XMIT", 0.0);
  m_Comms.Register("RECON_XMITM", 0.0);

  m_Comms.Register("REMUS_TIME", 0.0); // NJN:20141210

  // Get node reports (maybe via PLATFORM_REPORT?)
  m_Comms.Register("NODE_REPORT_LOCAL", updateRate);
}

bool Recon::OnConnectToServer()
{
  // Reregister in case this is a new MOOS
  DoRegistrations(DEFAULT_REGISTER_RATE);
  return true;
}

bool Recon::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;
  
  // Loop through new mail
  for (p = NewMail.begin(); p != NewMail.end(); p++)
  {
    // Pull message contents
    CMOOSMsg &rMsg = *p;
    if (strcmp(rMsg.GetKey().c_str(), "DESIRED_HEADING") == 0)
    {
      if (!rMsg.IsDouble())
      {
        MOOSTrace("ERROR - DESIRED_HEADING is not a double\n");
        continue;
      }
      else
      {
	double new_heading;
	if (random_hdg)
	  {
	    new_heading = angle360(rMsg.m_dfVal + hdg_offset);
	    //MOOSTrace(">>>>> New heading %f \n",new_heading);
	  }
	else
	  new_heading = rMsg.m_dfVal;

        vCourse = new_heading;
	//        vCourse = rMsg.GetDouble();
        recvNewCourse = true;
      }
    }
    else if (strcmp(rMsg.GetKey().c_str(), "DESIRED_SPEED") == 0)
    {
      if (!rMsg.IsDouble())
      {
        MOOSTrace("ERROR - DESIRED_SPEED is not a double\n");
        continue;
      }
      else
      {
        vSpeed = rMsg.GetDouble();
        recvNewSpeed = true;
      }
    }
    else if (strcmp(rMsg.GetKey().c_str(), "DESIRED_DEPTH") == 0)
    {
      if (!rMsg.IsDouble())
      {
        MOOSTrace("ERROR - DESIRED_DEPTH is not a double\n");
        continue;
      }
      else
      {
        vDepth = rMsg.GetDouble();
        recvNewDepth = true;
      }
    }
    ///// Removed any dependence on MOOS_MANUAL_OVERIDE
    //else if (strcmp(rMsg.GetKey().c_str(), "MOOS_MANUAL_OVERIDE") == 0)
    //{
    //  // true  - Manual is engaged, send keep-alive packets as necessary
    //  // false - Autonomous system is engaged, should not need keep-alive
    //  activateKeepAlive = ParseBoolString(rMsg.GetAsString().c_str());
    //}
    else if (strcmp(rMsg.GetKey().c_str(), "TRIANGLE_MODE") == 0)
    {
      triangleMode = ParseBoolString(rMsg.GetAsString().c_str());
    }
    else if (strcmp(rMsg.GetKey().c_str(), "RECON_ENGAGE") == 0)
    {
      // Signal on/off to RECON
      wantReconActive = ParseBoolString(rMsg.GetAsString().c_str());
      activateKeepAlive = !wantReconActive;
    }
    else if (strcmp(rMsg.GetKey().c_str(), "RECON_XMIT") == 0)
    {
      // Allow other MOOS processes to send RECON commands
      Transmit("%s", rMsg.GetAsString().c_str());
    }
    else if (strcmp(rMsg.GetKey().c_str(), "RECON_XMITM") == 0)
    {
      // Allow other MOOS processes to send RECON commands   
      // sg: dded the #m,1,1 to the first argument of transmit!!!!
      Transmit("#m 1,1, %s, *", rMsg.GetAsString().c_str());
    }

  
    //  else if (strcmp(rMsg.GetKey().c_str(), "NODE_REPORT_LOCAL") == 0)
    // {
      // How many ASCII chars do we need for the useful info in a node report? e.g.
      // NODE_REPORT_LOCAL = "NAME=alpha,TYPE=UUV,TIME=1252348077.59,X=51.71,Y=-35.50,
      //                   LAT=43.824981,LON=-70.329755,SPD=2.00,HDG=118.85,YAW=118.84754,
      //                   DEPTH=4.63,LENGTH=3.8,MODE=MODE@ACTIVE:LOITERING"
      //
      // NAME = fixed 
      // TYPE = fixed 
      // TIME = 1252348077.59  = 10 (without fractional seconds--could be further reduced)
      // X = omitable
      // Y = omitable
      // LAT = -43.824981       = 9 (if you multiply by 10^6)
      // LON = -121.123456      = 10 (ditto)
      // SPD = 2.00            = 3 (x 100)
      // HDG = 118.85          = 3 (ignore decimal)
      // YAW = 118.84754       = 3 (ditto)
      // DEPTH = 4.63          = 4 (up to 600.0 x 10)
      // LENGTH = fixed
      // MODE = MODE@ACTIVE:LOITERING = 21 (in this case--should use whatever is leftover in msg)
      //
      //                       There's 32 chars above, which leaves 30 for the mode message. 
      //                       The "MODE@" part might be constant too?
      //
      //

      // Send NODE_REPORT at specified interval
    // if (difftime(time(NULL),node_timer) > node_interval)
    // {
    //	Transmit("#m,1,1,%s,*", rMsg.GetAsString().c_str());
    //	time(&node_timer); // Reset timer after sending message
    // }  
    // }
    
  }
  return true;
}

bool Recon::OnStartUp()
{
  string param;
  double dfLatOrigin, dfLongOrigin;
  
  // UDP Network defaults
  string localPort = "23456";
  string remusHost = "remus";
  string remusPort = "23456";
  
  ///////////////////////////////
  // Initialize RECON Settings //
  ///////////////////////////////
  
  // Initialize RECON Connection
  m_MissionReader.GetConfigurationParam("local_port", localPort);
  m_MissionReader.GetConfigurationParam("remus_host", remusHost);
  m_MissionReader.GetConfigurationParam("remus_port", remusPort);
  if (recon.ConnectUDP(remusHost.c_str(), remusPort.c_str(),
                       localPort.c_str()))
  {
    printf("FATAL - Could not connect to REMUS\n");
     exit(1);
  }
  
  // Parse mission configuration
  if (m_MissionReader.GetConfigurationParam("speed_mode", param))
  {
    if (strcmp(param.c_str(), "rpm") == 0)
    {
      rpmSpeedMode = true;
    }
    else
    {
      rpmSpeedMode = false;
    }
  }
  if (m_MissionReader.GetConfigurationParam("ready_var", param))
  {
    readyVar = param;
  }
  if (m_MissionReader.GetConfigurationParam("ready_numeric", param))
  {
    readyVarIsNumeric = ParseBoolString(param.c_str());
  }
  if (m_MissionReader.GetConfigurationParam("keep_alive", param))
  {
    allowKeepAlive = ParseBoolString(param.c_str());
  }
  
  ///////////////////////////////////////////////////
  // Initialize Lat/Long <-> X/Y Coordinate Mapper //
  ///////////////////////////////////////////////////
  
  // Initial Latitude
  if(!m_MissionReader.GetValue("LatOrigin",param))
  {
    MOOSTrace("FATAL ERROR - LatOrigin not set\n");
    return false;
  }
  dfLatOrigin = atof(param.c_str());
  
  // Initial Longitude
  if(!m_MissionReader.GetValue("LongOrigin",param))
  {
    MOOSTrace("FATAL ERROR - LongOrigin not set\n");
    return false;
  }
  dfLongOrigin = atof(param.c_str());
  
  // Initialize Converter
  if(!m_Geodesy.Initialise(dfLatOrigin,dfLongOrigin))
  {
    MOOSTrace("FATAL ERROR - Geodesy Init failed\n");
    return false;
  }
  
  // Register for appropriate MOOS variables
  DoRegistrations(DEFAULT_REGISTER_RATE);
  
  // Spawn child thread to receive RECON packets
  if (pthread_create(&reconRecvThread, NULL, ReconRecvHandoff, this) != 0)
  {
    MOOSFail("Could not spawn RECON receive thread\n");
    return false;
  }
  
  //////////////////////////////////
  // NTP SHM Driver Intialization //
  //////////////////////////////////
  
  // Which NTP SHM driver to provide (127.127.28.x)
  if(m_MissionReader.GetValue("ntpref_unit", param))
  {
    ntpRef.select(atoi(param.c_str()));
  }
  
  // Snap clock if off by more than this amount
  if(m_MissionReader.GetValue("ntpref_maxskew", param))
  {
    ntpRefMaxSkew = atoi(param.c_str());
  }

  return true;
}

bool Recon::Iterate()
{
  // Sync timestamps as needed
  if (remusTimeValid == false)
  {
    Transmit("#s,Time,query,*");
  }
  
  // Ready RECON
  CheckReady();
  
  // Send new RECON control packet if we've received a new course/speed/depth,
  // or if keep alive has been both allowed and engaged
  if (allowKeepAlive && activateKeepAlive)
    {
      Transmit("#V,%f,*", 0);
    }
  else if (recvNewCourse && recvNewSpeed && recvNewDepth)
  {
    // Send RECON course/speed/depth
    Transmit("#C,Heading,Goal,%f,*", vCourse);
    Transmit("#C,Speed,%f %s,*", vSpeed,
             (rpmSpeedMode ? "rpm" : "meters/sec"));
    if (triangleMode)
      {
	Transmit("#C,Depth,Triangle,%f,%f,%f,%f,*",
		 tDepth,tAltitude,tRate,tMaxD);
      }
    else
      {
	Transmit("#C,Depth,Depth,%f,*", vDepth);
      }
  }
  
  
  // Mark course/speed/depth as not new
  recvNewCourse = false;
  recvNewSpeed = false;
  recvNewDepth = false;
  
  
  return true;
}

bool Recon::Transmit(const char *fmt, ...)
{
  int rc, length, i, chksum = 0, maxlen;
  va_list args;
  char buf[MAX_RECON_MSG_SIZE];
  char *bufFields[MAX_RECON_FIELDS];
  int numFields;
  
  // Format buffer
  maxlen = MAX_RECON_MSG_SIZE - 4;
  va_start(args, fmt);
  length = vsnprintf(buf, maxlen, fmt, args);
  va_end(args);
  
  // vsnprintf may return -1 or maxlen(or more) on error
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
  
  // Transmit message
    printf("Sending: %s", buf);
  if (DEBUG)
  {
    printf("Sending: %s", buf);
  }
  rc = recon.Send(buf, length, false);
  if (rc != length)
  {
    MOOSTrace("WARNING - sendto() failed\n");
    return false;
  }
  
  return true;
}

void *Recon::ReconRecvLoop()
{
  int i;
  int numBytes;
  int numFields;
  char *nextPtr, *savPtr;
  char buf[MAX_RECON_MSG_SIZE];
  char *bufFields[MAX_RECON_FIELDS];
  
  MOOSTrace("Started RECON receiver thread\n");
  
  while ((numBytes = recon.Recv(buf, MAX_RECON_MSG_SIZE)) > 0)
  {
    // RECON talks text, so make sure buffer is properly terminated
    buf[numBytes] = '\0';
    
    // Debug input
    if (DEBUG)
    {
      printf("Receiving: \'%s\'\n", buf);
    }
    
    // Hack up inbound buffer into null-delimited fields, using strtok()
    numFields = 0;
    for (nextPtr = strtok_r(buf, ",", &savPtr);
         (nextPtr != NULL) && (numFields < MAX_RECON_FIELDS);
         nextPtr = strtok_r(NULL, ",", &savPtr))
    {
      bufFields[numFields++] = nextPtr;
    }
    
    // Pass control off to appropriate message handler
    switch (bufFields[0][1])
    {
    case 'S':
      // REMUS State message
      ParseMsgState(numFields, bufFields);
      break;
      
    case 's':
      // REMUS set message
      ParseMsgSet(numFields, bufFields);
      break;

    case 'c':
      // CTD data message
      ParseMsgCTD(numFields, bufFields);
      break;

    case 'm':
      // MODEM message
      ParseMsgModem(numFields, bufFields);
      break;

      
//    case 'C':
//      // Echo of last RECON command (not really interested)
//      break;
      
    default:
      if (DEBUG)
      {
        printf("Message type \'%c\' - \'#%c", bufFields[0][1], bufFields[0][1]);
        for (i = 1; i < numFields; i++)
        {
          printf(",%s", bufFields[i]);
        }
        printf("\'\n");
      }
      break;
    }
  }
  
  MOOSFail("FATAL - RECON receiver thread exiting\n");
  exit(1);
  
  return NULL;
}

bool Recon::ParseBoolString(const char *buf)
{
  if (strcasecmp(buf, "true") == 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void Recon::CheckReady()
{
  // Signal RECON ready
  if (readyVarIsNumeric == false)
  {
    // Post ready as alpha true/false
    if (reconActive == true)
    {
      m_Comms.Notify(readyVar, "true");
    }
    else
    {
      m_Comms.Notify(readyVar, "false");
    }
  }
  else
  {
    // Post ready as numeric 1/0
    // true/false are 1/0 resp.
    m_Comms.Notify(readyVar, (int)reconActive);
  }
  
  // Engage/Disengage RECON as necessary
  if (wantReconActive != reconActive)
  {
    Transmit("#E,%d,*", (int)wantReconActive);
  }
}

void Recon::HandleStateTime(int hour, int min, int sec, int subsec)
{
  struct tm newTime;
  struct timeval newTimeval;
  time_t newTimeUTC;
  
  // Don't try to operate if we don't know what day/year we're on
  if (remusTimeValid == false)
  {
    return;
  }
  
  // Build struct using fields from REMUS,
  // note this doesn't include year/day, so
  // on hour rollover (midnight), this will
  // look a day off, and trigger a resync
  newTime = remusTime;
  newTime.tm_hour = hour;
  newTime.tm_min  = min;
  newTime.tm_sec  = sec;
  newTimeUTC = mktime(&newTime);
  
  // NTPD doesn't want updates more frequently than 1Hz,
  // if we're on the same second as the last update, skip this one
  if (remusTimeval.tv_sec == newTimeUTC)
  {
    return;
  }
  
  // Check to see if the UTC timestamp makes sense
  // (or we've rolled over a day boundary)
  if (abs(remusTimeval.tv_sec - newTimeUTC) > REMUS_TIME_TOLERANCE)
  {
    // Doesn't make sense, next Iterate() will query a time sync
    printf("Invalidating REMUS timestamp (off by %d seconds)\n",
           (int) abs(remusTimeval.tv_sec - newTimeUTC));
    remusTimeValid = false;

    return;
  }
  
  // Otherwise, REMUS time seems good
  // (note - remusTimeValid already set to true)
  remusTime = newTime;
  remusTimeval.tv_sec = newTimeUTC;
  remusTimeval.tv_usec = subsec * 100000;
  char YMDhms[23];
  std::strftime(YMDhms, sizeof(YMDhms), "%Y-%m-%d %H:%M:%S", &remusTime);
  m_Comms.Notify("REMUS_TIME",YMDhms);

  // Big time deltas can keep ntpd from syncing,
  // so we can configure ourselves to set the
  // clock if off by a large amount
  if ((ntpRefMaxSkew != -1) &&
      (abs(time(NULL) - newTimeUTC) > ntpRefMaxSkew))
  {
    // Need to set clock
    if (settimeofday(&newTimeval, NULL) == 0)
    {
      // Clock set successfully
      return;
    }
    
    // In all likelyhood, this is a permission issue (not running as root)
    // we can either choke here (which will kill the mission),
    // print an error message (which may not be seen except on the bench),
    // or pass the time through to NTP and hope for the best
    
    // Whine to the console, in case someone is watching,
    // and fall through to NTP
    perror("Cannot set clock");
  }
  
  // Forward time to NTP
  //printf("REMUS Time is now: %s", asctime(&newTime));
  //printf("Time offset is now: %d seconds\n", 
  //       time(NULL) - newTimeUTC);
  ntpRef.update(remusTimeval.tv_sec, remusTimeval.tv_usec);
}

bool Recon::ParseMsgState(int numFields, char **fields)
{
  double field, field2;
  int hour, min, sec, subsec;
  double speed_rpm, speed_vel;
  double lat, lon, x, y;
  int intField;
  char charField;
  double pi = 4*atan(1);
  
  // Require 17 fields for data
  if (numFields < 17)
  {
    return false;
  }
  
  // Field 1 - Time of Day
  if (sscanf(fields[1], "T%d:%d:%d.%d", &hour, &min, &sec, &subsec) == 4)
  {
    // Handle complexity of this elsewhere
    HandleStateTime(hour, min, sec, subsec);
  }
  
  // Field 2 - Latitude
  if (sscanf(fields[2], "LAT%lf%c%lf", &field, &charField, &field2) == 3)
  {
    // Calculate resultant
    lat = field + (field2 / 60);
    if (charField == 'S')
    {
      lat = -1 * lat;
    }
    m_Comms.Notify("NAV_LAT", lat);
  }
  
  // Field 3 - Longitude
  if (sscanf(fields[3], "LON%lf%c%lf", &field, &charField, &field2) == 3)
  {
    // Calculate resultant
    lon = field + (field2 / 60);
    if (charField == 'W')
    {
      lon = -1 * lon;
    }
    m_Comms.Notify("NAV_LONG", lon);
  }
  
  // Field 4 - Depth (m)
  if (sscanf(fields[4], "D%lf", &field) == 1)
  {
    m_Comms.Notify("NAV_DEPTH", field);
    m_Comms.Notify("NAV_Z", -1 * field);
  }
  
  // Field 5 - Depth Goal (m)
  
  // Field 6 - Altitude (m)
  if (sscanf(fields[6], "A%lf", &field) == 1)
  {
    // 0 indicates no known altitude (don't post bad data)
    if (field != 0)
    {
      m_Comms.Notify("NAV_ALTITUDE", field);
    }
  }
  
  double pitch_angle;  
  // Field 7 - Pitch (deg)
  if (sscanf(fields[7], "P%lf", &field) == 1)
  {
    m_Comms.Notify("NAV_PITCH", (pi / 180.0) * field);
    pitch_angle = (pi/180.)*field;
  }
  
  // Field 8 - Roll (deg)
  if (sscanf(fields[8], "R%lf", &field) == 1)
  {
    m_Comms.Notify("NAV_ROLL", (pi / 180.0) * field);
  }
  
  // Field 9 - Thruster RPM (rpm)
  if (sscanf(fields[9], "TR%lf", &speed_rpm) == 1)
  {
    m_Comms.Notify("NAV_SPEED_RPM", speed_rpm);
  }
  
  // Field 10 - Thruster RPM Goal (rpm)
  
  // Field 11 - Velocity (m/s)
  if (sscanf(fields[11], "V%lf", &speed_vel) == 1)
  {
    m_Comms.Notify("NAV_SPEED_VEL", speed_vel);
  }
  
  // Field 12 - Heading (deg)
  double hdg_angle;
  if (sscanf(fields[12], "H%lf", &field) == 1)
  {
    m_Comms.Notify("NAV_HEADING", field);
    m_Comms.Notify("NAV_YAW", (pi / -180.0) * field);
    hdg_angle=(90-field)*pi/180.0; 
 }
  
  // Field 13 - Heading Rate (deg/s)
  if (sscanf(fields[13], "HR%lf", &field) == 1)
  {
    m_Comms.Notify("NAV_HEADING_RATE", field);
  }
  
  // Field 14 - Heading Goal (deg)
  
  // Field 15 - Mode (RECON bitmask)
  if (sscanf(fields[15], "M%x", &intField) == 1)
  {
    char tmp[16];
    snprintf(tmp, 16, "0x%02X", intField);
    m_Comms.Notify("RECON_MODE", tmp);
    // Check the Status Bit to see if we're deployed
    if ((intField & 0x10) != 0)
    {
      m_Comms.Notify("REMUS_DEPLOYED", "false");
    }
    else
    {
      m_Comms.Notify("REMUS_DEPLOYED", "true");
    }

    // Now do anything else that we need to, based on mode
    switch (intField)
      {
      case 0x01: // Launched, over-ride inactive
	reconActive = false;
	activateKeepAlive = true;
	break;

      case 0x0D: // Launched, over-ride active
	// This can either be a status update, confirming that the
	// vehicle has recieved the "enable" message. If this is the case
	// then we want to denote that reconActive=true. Conversely, if
	// we are just in heatbeat mode, then switching to this mode indicates
	// that the vehicle has requested an override mode, and we need to tell 
	// romulus to start driving. 
	if (wantReconActive != 0) // We want recon active,
	{  
	  reconActive = true;     // then report that it is turned on,
	  activateKeepAlive = false; // turn off the heatbeat
	}
	else  // MOOS has not requested recon to be on,
	{     // but the vehicle has asked for an override
	  wantReconActive = true;
	}  
	break;

      case 0x02: // Launched, ovre-ride inactive, ignored
      case 0x0E: // Launched, over-ride active, ignored
	break;

      case 0x03: // Mission over	
	wantReconActive = false;
	activateKeepAlive = false;
	break;

      default:
	break;
      }


  }
  
  // Field 16 - Leg
  
  // Field 17 - Checksum
  
  /////////////////////////////////////////////
  // Calculate effective values for MOOS-IvP //
  /////////////////////////////////////////////
  
  // Speed may be in RPM or Velocity(m/s)
  if (rpmSpeedMode)
  {  
    m_Comms.Notify("NAV_SPEED", speed_rpm);
  }
  else
  {
    m_Comms.Notify("NAV_SPEED", speed_vel);
  }
  
  // Calculate local X/Y coordinates
  m_Geodesy.LatLong2LocalGrid(lat, lon, y, x);
  m_Comms.Notify("NAV_X", x);
  m_Comms.Notify("NAV_Y", y);
  
  // Speed vector needed for towed array simulator 
  double eastSpeed = speed_vel*cos(hdg_angle);
  double northSpeed = speed_vel*sin(hdg_angle);
  m_Comms.Notify("VEL_EAST", eastSpeed);
  m_Comms.Notify("VEL_NORTH", northSpeed);
  double upSpeed = speed_vel*sin(pitch_angle);
  double dnSpeed = -upSpeed;
  m_Comms.Notify("VEL_DOWN", dnSpeed);
  m_Comms.Notify("VEL_UP", upSpeed);

  return true;
}

bool Recon::ParseMsgSet(int numFields, char **fields)
{
  time_t timestamp;
  
  // Require 1 field for data
  if (numFields < 1)
  {
    return false;
  }
  
  // Determine what was passed
  if (strcmp(fields[1], "SHUTDOWN") == 0)
  {
    // REMUS has sent a shutdown..
    printf("RECON Shutdown Received...\n");
    system("/sbin/shutdown -h now");
  }
  else if (strcmp(fields[1], "TIME") == 0)
  {
    // REMUS UTC Time Fix
    if (sscanf(fields[2], "%d", (int*) &timestamp) == 1)
    {
      printf("REMUS Time Fix Received: %d\n", (int) timestamp);
      gmtime_r(&timestamp, &remusTime);
      printf("\t%s\n", asctime(&remusTime));
      remusTimeval.tv_sec = timestamp;
      remusTimeval.tv_usec = 0;
      remusTimeValid = true;
    }
  }
  
  // Otherwise, don't really care
  
  return true;
}


////////////////////////////////////////////////
// Parsers added by njn 
////////////////////////////////////////////////

bool Recon::ParseMsgCTD(int numFields, char **fields)
{
  double field, field2;
  
  // Require 5 fields for data
  if (numFields < 5)
  {
    return false;
  }
  
  // Field 1 - Temperature
  if (sscanf(fields[1], "T%lf", &field) == 1)
  {
    m_Comms.Notify("CTD_T", field);
  }

  // Field 2 - Conductivity
  // Don't see a need to report this right now.
  if (sscanf(fields[2], "C%lf", &field) == 1)
  {
    //m_Comms.Notify("CTD_C", field);
  }

  // Field 3 - Salinity
  if (sscanf(fields[3], "S%lf", &field) == 1)
  {
    m_Comms.Notify("CTD_S", field);
  }

  // Field 4 - Depth 
  if (sscanf(fields[4], "D%lf", &field) == 1)
  {
    m_Comms.Notify("CTD_D", field);
  }

  // Field 5 - Speed of Sound
  // Don't see a need to report this right now.
  if (sscanf(fields[5], "V%lf", &field) == 1)
  {
    //m_Comms.Notify("CTD_V", field);
  }

  return true;
}



//////////// Modem messages

bool Recon::ParseMsgModem(int numFields, char **fields)
{
  double field, field2;
  
  // Incoming data messages have four fields
  if (numFields < 4)
  {
    return false;
  }
  
  // Field 1 - Modem information type (MSG_DATA or ACK)
  // Check if this is an incoming message with new data, 
  // so that we can parse it out appropriately. (Tho at
  // present we won't get here b/c ACKs have < 4 fields.)
  if (strcmp(fields[1], "MSG_DATA") == 1)
  {
    // Field 2 - Message source
    // Confirm that the source is the topside modem
    // otherwise, ignore for now.
    if (sscanf(fields[2], "%d", &field) == 1)
    {
      if (field != 0) 
      {
	return false;  
      }
    }

    // Field 3 - Message destination. 
    // Make sure it's for us. Assuming that we are DR1 = 8.
    if (sscanf(fields[3], "%d", &field) == 1)
    {
      if (field != 8) 
      {
	return false;  
      }
    }

    // Field 4 - Message. Must start with "1E"
    if (strncmp(fields[4], "1E", 2) == 1)
    {
      m_Comms.Notify("MSG_DATA",fields[4]);
      return true;
    }
  }
  return false;
}



