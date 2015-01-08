/* -----------------------------------------------------------------------
   Process AIS messages read from stdin and output readable text fields
   Copyright 2006 by Brian C. Lane
   All Rights Reserved
   ----------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portable.h"
#include "nmea.h"
#include "sixbit.h"
#include "vdm_parse.h"



int main( int argc, char *argv[] )
{
    ais_state     ais;
    char          buf[256];

    /* AIS message structures, only parse ones with positions */
    aismsg_1  msg_1;
    aismsg_2  msg_2;
    aismsg_3  msg_3;
    aismsg_4  msg_4;
    aismsg_9  msg_9;
    aismsg_15 msg_15;
    aismsg_18 msg_18;
    aismsg_19 msg_19;
    
    /* Position in DD.DDDDDD */
    double lat_dd = 0;
    double long_ddd = 0;
    long   userid = 0;
    double cog = 0;
    double sog = 0;

      int s;
      int nread;

    /* Clear out the structures */
    memset( &ais, 0, sizeof( ais_state ) );

    /* Try connecting to the host. */
      s = tcpconnect("76.103.90.196",9009);
    if (s < 0)
      exit(1);

    /* Process incoming packets from port */
      while( (nread = read (s, buf, sizeof (buf))) > 0 )
    {
      
        if (assemble_vdm( &ais, buf ) == 0)
        {
            /* Get the 6 bit message id */
            ais.msgid = (unsigned char) get_6bit( &ais.six_state, 6 );
            
            /* process message with appropriate parser */
            switch( ais.msgid ) {
                case 1:
                    if( parse_ais_1( &ais, &msg_1 ) == 0 )
                    {
                        userid = msg_1.userid;
                        pos2ddd( msg_1.latitude, msg_1.longitude, &lat_dd, &long_ddd );
		        cog = msg_1.cog;
		        sog = msg_1.sog;
                    }
                    break;
                            
                case 2:
                    if( parse_ais_2( &ais, &msg_2 ) == 0 )
                    {
                        userid = msg_2.userid;
                        pos2ddd( msg_2.latitude, msg_2.longitude, &lat_dd, &long_ddd );
		        cog = msg_2.cog;
		        sog = msg_2.sog;
                    }
                    break;
                            
                case 3:
                    if( parse_ais_3( &ais, &msg_3 ) == 0 )
                    {
                        userid = msg_3.userid;
                        pos2ddd( msg_3.latitude, msg_3.longitude, &lat_dd, &long_ddd );
		        cog = msg_3.cog;
		        sog = msg_3.sog;
                    }
                    break;
                            
                case 4:
                    if( parse_ais_4( &ais, &msg_4 ) == 0 )
                    {
                        userid = msg_4.userid;
                        pos2ddd( msg_4.latitude, msg_4.longitude, &lat_dd, &long_ddd );
		        cog = 0;
		        sog = 0;
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
                
                case 15:
                    if( parse_ais_15( &ais, &msg_15 ) == 0 )
                    {
                        userid = msg_15.userid;
                        if( msg_15.num_reqs > 0 )
                        {
                            printf("dest #1   : %ld\n", msg_15.destid1 );
                            printf("msgid #1  : %d\n", msg_15.msgid1_1 );
                            printf("offset #1 : %d\n", msg_15.offset1_1 );
                        }
                        if( msg_15.num_reqs > 1 )
                        {
                            printf("msgid #2  : %d\n", msg_15.msgid1_2 );
                            printf("offset #2 : %d\n", msg_15.offset1_2 );
                        }
                        if( msg_15.num_reqs > 2 )
                        {
                            printf("dest #2     : %ld\n", msg_15.destid2 );
                            printf("msgid #2.1  : %d\n", msg_15.msgid2_1 );
                            printf("offset #2.1 : %d\n", msg_15.offset2_1 );
                        }
                    }
                    break;
                    
                    
                case 18:
                    if( parse_ais_18( &ais, &msg_18 ) == 0 )
                    {
                        userid = msg_18.userid;
                        pos2ddd( msg_18.latitude, msg_18.longitude, &lat_dd, &long_ddd );
		        cog = msg_18.cog;
		        sog = msg_18.sog;
                    }
                    break;
                    

                case 19:
                    if( parse_ais_19( &ais, &msg_19 ) == 0 )
                    {
                        userid = msg_19.userid;
                        pos2ddd( msg_19.latitude, msg_19.longitude, &lat_dd, &long_ddd );
		        cog = msg_19.cog;
		        sog = msg_19.sog;
                    }
                    break;                    
            }  /* switch msgid */
            
            printf( "MESSAGE ID: %d\n", ais.msgid );
            printf( "USER ID   : %ld\n", userid );
            printf( "POSITION  : %0.6f %0.6f\n", lat_dd, long_ddd );
            printf( "VELOCITY  : %0.6f %0.6f\n", sog, cog );
	    sog = 0;
	    cog = 0;
        }  /* if */
    }  /* while */
    
    return 0;
}
