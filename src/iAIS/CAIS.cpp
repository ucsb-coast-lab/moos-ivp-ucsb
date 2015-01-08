#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>

#include <map>
#include <algorithm>
#include <iostream>

#include "MOOSLib.h"
#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"
#include "CAIS.h"
#include "portable.h"
#include "nmea.h"
#include "sixbit.h"
#include "vdm_parse.h"
#include "tcpsocket.h"


// From BCL parser
ais_state     ais;
char          buf[256];

/* AIS message structures, only parse those with positions */
aismsg_1  msg_1;
aismsg_2  msg_2;
aismsg_3  msg_3;
aismsg_4  msg_4;
aismsg_5 msg_5;
aismsg_9  msg_9;
aismsg_15 msg_15;
aismsg_18 msg_18;
aismsg_19 msg_19;

int nread;

std::map<unsigned long, contact> watch_list;
std::map<unsigned long, contact>::iterator it;

CAIS::CAIS(string ais_host, int port, double lat_origin, double lon_origin)
{
  const char *host;
  host = ais_host.c_str();
  pt = tcpconnect(host, port);
  if (pt < 0)
      exit(1);

  m_geodesy.Initialise(lat_origin, lon_origin);


  lat_dd = 0;
  long_ddd = 0;
  nav_x = 0;
  nav_y = 0;
  userid = 0;
  sog = 0;
  cog = 0;
  hdg = 0;
  postable = false;

  running = false;
  pthread_create(&thr, NULL, &tramp, this);
}

CAIS::~CAIS()
{
  running = false;
  pthread_join(thr, NULL);
  // need to close the socket here?
  close(pt);
}

void CAIS::Thread()
{
  running = true;
  /* Read AIS messages from source */
  while(running && (nread = read (pt, buf, sizeof (buf))) > 0) 
    {
            cout << "Raw message: " << buf;
      /* Reassemble AIS message */
      if (assemble_vdm( &ais, buf ) == 0)
        {
	  /* Get the 6 bit message id */
	  ais.msgid = (unsigned char) get_6bit( &ais.six_state, 6 );
          if (!postable)
	    {
	      bool valid;
	      valid = false;
	      /* process message with appropriate parser */
	      switch( ais.msgid ) {
	      case 1: // position report
		if( parse_ais_1( &ais, &msg_1 ) == 0 )
		  {
		    userid = msg_1.userid;
		    pos2ddd( msg_1.latitude, msg_1.longitude, &lat_dd, &long_ddd );
		    cog = msg_1.cog / 10.0;
		    sog = msg_1.sog / 10.0;
		    hdg = msg_1.true_hdg;
		    nav_status_bit = msg_1.nav_status;
		    valid = true;
		  }
		break;
		
	      case 2: // position report
		if( parse_ais_2( &ais, &msg_2 ) == 0 )
		  {
		    userid = msg_2.userid;
		    pos2ddd( msg_2.latitude, msg_2.longitude, &lat_dd, &long_ddd );
		    cog = msg_2.cog / 10.0;
		    sog = msg_2.sog / 10.0;
		    hdg = msg_2.true_hdg;
		    nav_status_bit = msg_2.nav_status;
		    valid = true;
		  }
		break;
		
	      case 3:  // position report
		if( parse_ais_3( &ais, &msg_3 ) == 0 )
		  {
		    userid = msg_3.userid;
		    pos2ddd( msg_3.latitude, msg_3.longitude, &lat_dd, &long_ddd );
		    cog = msg_3.cog / 10.0;
		    sog = msg_3.sog / 10.0;
		    hdg = msg_3.true_hdg;
		    nav_status_bit = msg_3.nav_status;
		    valid = true;
		  }
		break;
		
	      case 4: // base station
		if( parse_ais_4( &ais, &msg_4 ) == 0 )
		  {
		    userid = msg_4.userid;
		    pos2ddd( msg_4.latitude, msg_4.longitude, &lat_dd, &long_ddd );
		    cog = 0;
		    sog = 0;
		    hdg = 0;;
		  }
		break;
		
	      case 5: // static information
		if( parse_ais_5( &ais, &msg_5 ) == 0 )
		  {
		    // Check for previous report in mmsi list
		    it = watch_list.find(msg_5.userid);
		    if ( it == watch_list.end() ) {
                      contact c = {msg_5.userid, msg_5.name, msg_5.ship_type, msg_5.draught};
		      watch_list[msg_5.userid] = c;
		      cout << " Added static info for: " << watch_list[msg_5.userid].name.c_str() << "\n";
		    }


		  }
		break;
		
	      case 9:
		if( parse_ais_9( &ais, &msg_9 ) == 0 )
		  {
		    userid = msg_9.userid;
		    pos2ddd( msg_9.latitude, msg_9.longitude, &lat_dd, &long_ddd );
		    cog = msg_9.cog;
		    sog = msg_9.sog;
		  }
		break;
				
		
	      case 18:
		if( parse_ais_18( &ais, &msg_18 ) == 0 )
		  {
		    userid = msg_18.userid;
		    pos2ddd( msg_18.latitude, msg_18.longitude, &lat_dd, &long_ddd );
		    cog = msg_18.cog/10;
		    sog = msg_18.sog/10;
		    hdg = msg_18.true_hdg;
		    nav_status_bit = 15;
		    valid = true;

		  }
		break;
		
		
	      case 19:  // extended class B
		if( parse_ais_19( &ais, &msg_19 ) == 0 )
		  {
		    userid = msg_19.userid;
		    pos2ddd( msg_19.latitude, msg_19.longitude, &lat_dd, &long_ddd );
		    cog = msg_19.cog/10;
		    sog = msg_19.sog/10;
		    hdg = msg_19.true_hdg;
		    nav_status_bit = 15;
		    valid = true;

		    // Check for previous report in mmsi list
		    it = watch_list.find(msg_19.userid);
		    if ( it == watch_list.end() ) {
                      contact c = {msg_19.userid, msg_19.name, msg_19.ship_type, 0};
		      watch_list[msg_19.userid] = c;
		      cout << " Added static info for: " << watch_list[msg_19.userid].name.c_str() << "\n";
		    }
		  }
		break;                   
	      }  /* switch msgid */
	      
	      // Check list of vessel names


	      if (valid) {
		switch (nav_status_bit) {
		case 0:
		  nav_status = "underPower";
		  break;

		case 1:
		  nav_status = "atAnchor";
		  break;		  

		case 2:
		  nav_status = "notUnderCommand";
		  break;		  
		  
		case 3:
		  nav_status = "restrictedManeuverability";
		  break;		  
		  
		case 4:
		  nav_status = "constrainedByHerDraft";
		  break;		  
		  
		case 5:
		  nav_status = "moored";
		  break;		  
		  
		case 6:
		  nav_status = "aground";
		  break;		  
		  
		case 7:
		  nav_status = "fishing";
		  break;		  
		  
		case 8:
		  nav_status = "underSail";
		  break;		  
		  
		case 10:
		  nav_status = "special";
		  break;		  
		  
		default:
		  nav_status = "unknown";
		}
		
		
		if (hdg == 511) {
		  hdg = cog;
		}
		m_geodesy.LatLong2LocalGrid(lat_dd, long_ddd, nav_y, nav_x);
		/*		printf("sending: NAME=%ld,TYPE=ship,TIME=%f,X=%f,Y=%f,LAT=%f,LON=%f,SPD=%2.1f,HDG=%d,YAW=%3.1f,"
		       "DEPTH=0,LENGTH=100,MODE=%s\n", userid, MOOSTime(),
		       nav_x, nav_y, 
		       lat_dd, long_ddd, 
		       sog, hdg, cog, nav_status.c_str());
		*/
		postable = true;
	      } //end IF valid
	    } // end IF !postable
        }  /* assemble IF */
      memset(buf,'\0',sizeof(buf));
    } /* read tcp buffer WHILE */
}
