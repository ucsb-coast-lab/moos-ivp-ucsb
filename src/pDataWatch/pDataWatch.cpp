//pDataWatch looks constantly for data published under "SCALAR_VALUE" althought it can later be configured to look 
//for whatever data is neccessary. after it gets a specified number of values it takes a 
//running average and checking to aee if it is above or below a threshold determined by the "THRESHOLD" paramter.
//if it's above the threshold it publishes a variable s_plus as true and otherwise publishes it as false.
// NJN:2014-12-08: Changed SPLUS/SMINUS configuration to SGOAL, which is more flexible
// NJN:2014-12-10: Corrected some hi/lo threshold confusion... 
//

//SG 1/10/15: TODO: refactor to include function pointers to a user defined function? maybe that's doing too much? 


#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "pDataWatch.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "CurrentField.h"

using namespace std;


//------------------------------------------------------
// Constructor 
//
DW_MOOSApp::DW_MOOSApp()
{
 count = 0;
 PhaseShift = 0;
 vehicle_depth = 0;

 SGOAL = 0;
 ONPAPA = false;
}


//-----------------------------------------------------
// OnNewMail
// notes: if a new SCALAR_VALUE has been published since 
//        the last check, set salinity to that value and
//        increase the count by one
//
bool DW_MOOSApp::OnNewMail(MOOSMSG_LIST &NewMail)
{

 CMOOSMsg Msg;
  
  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &Msg = *p;
    string key = Msg.m_sKey;
    double dval = Msg.m_dfVal;
    string sval = Msg.m_sVal;
    
    if (key == invar) {
      //   cout << "if_entered" << endl;
      scalar = dval;                     //I have no idea why, but it appears that ,Msg.m_dfVal is not 
      //scalar = atof(sval.c_str());           // working, it may be SCALAR_VALUE is in a format that the
                                         // CMOOSMsg doesn't like, but atof works fin
      cout << "pDW is reading the scalar value: " << scalar << endl;
    }
    
    if (key == "NAV_DEPTH") {
      vehicle_depth = dval;    // atof(sval.c_str());
    }

    if (key == "ON_PAPA") {
      if (strcasecmp(sval.c_str(), "true") == 0)
	{
	  ONPAPA = true;
	}
      else
	{
	  ONPAPA = false;
	}
    }
    
  }   
  return true;
}


//----------------------------------------------------
// Pocedure: OnStartUp
//    notes: reads in the threshold variable

bool DW_MOOSApp::OnStartUp()
{
  cout << "DataWatch Starting" << endl;

  
  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);
    
  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine  = *p;
    string param  = stripBlankEnds(MOOSChomp(sLine, "="));
    string value  = stripBlankEnds(sLine);
   
    param = toupper(param);
 
    if(param == "PAPA_TARGET"){
      lo_thres = atof(value.c_str());
      cout << "pDW: PAPA_TARGET (low threshold) = " << lo_thres << endl;
    }
    if(param == "ECHO_TARGET"){
      hi_thres = atof(value.c_str());
      cout << "pDW: ECHO_TARGET (high threshold) =  " << hi_thres << endl;
    }
    if(param == "SCALAR_SOURCE"){
      invar = value;
      cout << "pDW: Monitoring the scalar values from: " << invar << endl;
    }
    if(param == "POINTS_AVERAGED"){
      points = atoi(value.c_str());
      cout << "pDW: averaging " << points << " points." << endl; 
    }
    if(param == "TIME_TO_EXCEED"){
      tte = atoi(value.c_str());
      cout << "pDW: Must exceed threshold for :  " << tte << " seconds." <<  endl;
    } 
  }
  
  // Initialize array for scalars
  sal = new double[points];
  for(int i = 0; i < points; i++){
    sal[i] = 0;
  }  
  
   
  RegisterVariables();

  // Create output file for debugging
  std::time_t t = std::time(NULL);
  char YMDhms[31];
  std::strftime(YMDhms, sizeof(YMDhms), "pDataWatchLog_%Y%m%d-%H%M%S", std::gmtime(&t));
  reportname = YMDhms;
  
  // Should be checking for file-write here.  
  fout.open(reportname.c_str(), ios::out|ios::app);

  return (true);
}


//------------------------------------------------------------------------
// Procedure: OnConnectToServer
//     notes: just registers variables as of right now
//
bool DW_MOOSApp::OnConnectToServer()
{
  
  cout << "DataWatch connected" << endl;
  
  return(true);
}


//------------------------------------------------------------------------
// Procedure: registerVariables()
//     notes: subscribes to SCALAR_VALUE (as invar)
//
void DW_MOOSApp::RegisterVariables()
{
  m_Comms.Register(invar, 0);
  m_Comms.Register("NAV_DEPTH", 0);
  m_Comms.Register("ON_PAPA", 0);
}

//------------------------------------------------------------------------
// Procedure: OnDisconnectFromServer
//     notes: doesn't do much, but everything gets upset if this isn't here I think
bool DW_MOOSApp::OnDisconnectFromServer()
{
  fout.close();
  cout <<"DataWatch is leaving"<< endl;
  return(true);
}


//------------------------------------------------------------------------
// Procedure: Iterate
//     notes: after user specified number of points were had, takes a running average, determines which way to
//            flip the output varibale flag
//
bool DW_MOOSApp::Iterate()
{

  if (count > points) {
	cout << "pDW: TRACKING" << endl;
    if (GradientTrackPlusMinus()){       // Returns true if outside threshold range
      if (SGOAL == 1) {
	Notify("ON_PAPA","true");
	cout << "pDW: Changing ON_PAPA to TRUE" << endl;
      }
      if (SGOAL == -1) {
	Notify("ON_PAPA","false");
	cout << "pDW: Changing ON_PAPA to FALSE" << endl;
      }
    }
    // Output to the log file
    std::time_t t = std::time(NULL);
    
    fout << t << ", ";
    fout << vehicle_depth << ", ";
    fout << sal[points - 1] << ", ";
    fout << avg << ", ";
    fout << PhaseShift << ", ";
    fout << SGOAL << ", ";  
    fout << ONPAPA << endl;  
  } else {
	cout << "pDW: Too few points..." << endl;
    count ++;
  }

  return true;

}


//------------------------------------------------------------------------
//GradientTrackPlusMinus with splus and sminus flip
//notes: Modified gradient tracking algorithm, purpose is to show when
//       we're inbetween to isohalines and when we're in papa or echo
//
bool DW_MOOSApp::GradientTrackPlusMinus(){

   avg = RunningAverage(scalar);

   if(avg > hi_thres){
     PhaseShift = PhaseShift++;

     if (PhaseShift > tte){
       SGOAL = 1;
       return true;
     } 
   } else if (avg < lo_thres) {
     PhaseShift = PhaseShift++;

     if (PhaseShift > tte){
       SGOAL = -1;
       return true;
     }
   } else {
     PhaseShift = 0;
     SGOAL = 0;
     return false;
   }
   
}


//------------------------------------------------------------------------
//RunningAverage
//notes:  computes a running average. RunningAverage should return values of zero
//        until it has at least the number of points the user wants to average.
//
double DW_MOOSApp::RunningAverage(double scalar){      

  double sstar = 0;
  double total = 0;

  for (int n = 0; n < points - 1; n++){ 
    sal[n] = sal[n+1];
    total += sal[n];
  }
  sal[points-1] = scalar;  // The new point
  total += scalar;
  
  sstar = total/points;

  return sstar;
}
