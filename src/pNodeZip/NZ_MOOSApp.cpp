//NodeZip will take a standard node report and compress to less than 31 bytes (the maximum that can be sent to 
//Callie acoustically. it can be recovered in full by using this same process with the MODE param set to unzip

#include "NZ_MOOSApp.h" 
using namespace std;
//-------------------------------------------------------------------
//constructor
//initializes variables
NZ_MOOSApp::NZ_MOOSApp()
{
  nodeReportVar = "NODE_REPORT";
  zipReportVar = "ZIP_REPORT";
  count = 0;
  index = 0;

}

//--------------------------------------------------------------------
//OnNewMail
//listens for a new node report to zip
bool NZ_MOOSApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
  CMOOSMsg Msg;
  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &Msg = *p;
    string key = Msg.m_sKey;
    double dval = Msg.m_dfVal;
    string sval = Msg.m_sVal;

    if(key == nodeReportVar) {
      node_report = sval;
    }
    if(key == zipReportVar){
      zip_report = sval;
    }

  return(true);
  }
}


//--------------------------------------------------------------------
//OnStartUp
//initializes the name of the node report to subscribe to, and what name to publish under.


bool NZ_MOOSApp::OnStartUp()
{
  cout << "pNodeZip: pNodeZip Starting" << endl;
  
  bool MODE_Set = false;
  
  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);
    
  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine  = *p;
    string param  = stripBlankEnds(MOOSChomp(sLine, "="));
    string value  = stripBlankEnds(sLine);
    //double dval   = atof(value.c_str());

    param = toupper(param); 


    if(param == "UNZIPPED_REPORT"){  //defaults to "NODE_REPORT"
       nodeReportVar = value;
       cout << "pNodeZip: using " << nodeReportVar << "as the name for the unzipped node report" << endl;
     }
    if(param == "ZIPPED_REPORT"){ //defaults to "ZIP_REPORT
      zipReportVar = value;
      cout << "pNodeZip: using " << zipReportVar << "as the name for the zipped node report" << endl;
    }
    if(param == "MODE"){
      MODE_Set = true;
      if (value == "ZIP"){
	zipping = true;
	cout << "pNodeZip: will be ZIPPING a full node report" << endl;
      }
      else if (value == "UNZIP"){
	zipping = false;
	cout << "pNodeZip: will be UNZIPPING a zipped node report" << endl;
      }
      else{
	cout << "pNodeZip: ERROR!: MODE paramter does not have a recongnized value" << endl;
        cout << "pNodeZip: please set MODE to ZIP or UNZIP, now exiting" << endl;  
	std::exit(0);
      }
    }
  }
  if(!MODE_Set){
    cout << "pNodezip: ERROR: No MODE paramter found, please set MODE to ZIP or UNZIP, now exiting" << endl;
    std::exit(0);
  }

  // look for latitude, longitude global variables
  
  if(!m_MissionReader.GetValue("LatOrigin", latOrigin))
    cout << "pNodeZip: LatOrigin not set in *.moos file." << endl;
  else if(!m_MissionReader.GetValue("LongOrigin", lonOrigin))
    cout << "pNodeZip: LongOrigin not set in *.moos file" << endl;
  else
  geodesy.Initialise(latOrigin, lonOrigin);  //initializes the geodesy class 

   RegisterVariables();

   return true;

}



//------------------------------------------------------------------------
// OnConnectToServer
// here to make CMOOSAppp happy 

bool NZ_MOOSApp::OnConnectToServer()
{
 
  
  cout << "pNodeZip: pNodeZip2 connected" << endl;
  
  
  return(true);
}


//------------------------------------------------------------------------
// registerVariables()
// tells the DB what variables we would like to subscribe to

void NZ_MOOSApp::RegisterVariables()
{
  if(zipping)
    m_Comms.Register(nodeReportVar, 0); // may change that zero later, or maybe change the APP/COMMS tick
  
  if(!zipping)
    m_Comms.Register(zipReportVar, 0);
}


//------------------------------------------------------------------------
// OnDisconnectFromServer
// again, here mostly to make CMOOSApp happy

bool NZ_MOOSApp::OnDisconnectFromServer()
{
  cout << "pNodeZip: pNodeZip is leaving" << endl;
  return(true);
}

//------------------------------------------------------------------------
// Iterate
// this calls everything else

bool NZ_MOOSApp::Iterate()
{
    if(zipping)
      ReportZip();

    if(!zipping)
      ReportUnZip(); 
}

//------------------------------------------------------------------------
//ReportZip
//compresses a node report
bool NZ_MOOSApp::ReportZip(){
  if(node_report.length() == 0){
    cout << "couldn't find node report with name : " << nodeReportVar << endl;
  }
 
  int namepos = node_report.find("NAME=", 0);
  int spdpos = node_report.find("SPD=", 0);
  int hdgpos = node_report.find("HDG=" , 0);
  int latpos = node_report.find("LAT=" , 0);
  int lonpos = node_report.find("LON=" , 0);

  int name_endpos = node_report.find_first_of("," , namepos);
  int spd_endpos = node_report.find_first_of("," , spdpos);
  int hdg_endpos = node_report.find_first_of("," , hdgpos);
  int lat_endpos = node_report.find_first_of("," , latpos);
  int lon_endpos = node_report.find_first_of("," , lonpos);
  
  string namefield = node_report.substr(namepos , name_endpos - namepos);
  string spdfield = node_report.substr(spdpos, spd_endpos - spdpos);
  string hdgfield = node_report.substr(hdgpos, hdg_endpos - hdgpos);
  string latfield = node_report.substr(latpos, lat_endpos - latpos);
  string lonfield = node_report.substr(lonpos, lon_endpos - lonpos);

  
  int eqpos = namefield.find_first_of("=" , 0);
  namefield = namefield.substr(eqpos + 1, 9);

  eqpos = spdfield.find_first_of("=" , 0);
  spdfield = spdfield.substr(eqpos + 1, spdfield.length());
     
     float spdf = atof(spdfield.c_str());
     spdf*=10;

     stringstream ss;  //to_string is still not working
     ss << (int) spdf; // ought to cut off the decimal if there is one
     spdfield = ss.str();
   
  
  
  eqpos = hdgfield.find_first_of("=" , 0);
  hdgfield = hdgfield.substr(eqpos + 1, 3);
     
     if(hdgfield.find_first_of(".") != -1){
       int decpos = hdgfield.find_first_of(".");
       hdgfield = hdgfield.substr(0, decpos - 1);
     }

  int decpos = latfield.find_first_of(".", 0);
  eqpos = latfield.find_first_of("=" , 0);


  /*basically I'm assuming here the AUV won't be going on missions where it travels more than (worst case)50ish miles from the lat/long origin,
  this allows us to chop off a the part of latitude/longitude before the decimal. however "crossing over" a degree of latitude or longitude can
  occur at any distance, and will result in an inaccurate node report. the first number in the zipped lat/lon field encodes whether to add or
  subtract from the lat/lon origin upon unzipping. of course moving 2 or more degrees away from the origin will still make problems, so if we
  start running extremely long mission this section will need to be updated
  */

  int biglat = atoi(latfield.substr(eqpos + 1, decpos - eqpos - 1).c_str());
     string firstdig;
     if (biglat > (int) latOrigin)
       firstdig = "2";
     else if(biglat < (int) latOrigin)
       firstdig = "1";
     else firstdig = "0";
		    

      latfield = firstdig + latfield.substr(decpos + 1 , 5);

  decpos = lonfield.find_first_of(".", 0);
  eqpos = lonfield.find_first_of("=" , 0);
  int biglon = atoi(lonfield.substr(eqpos + 1, decpos - eqpos - 1).c_str());
     if (biglon > (int) lonOrigin)
       firstdig = "2";
     else if(biglon < (int) lonOrigin)
       firstdig = "1";
     else firstdig = "0";
		    
 
      lonfield = firstdig + lonfield.substr(decpos + 1 , 5);

  

  zip_report = namefield + "," + spdfield + "," + hdgfield + "," + latfield + "," + lonfield;


  Notify(zipReportVar, zip_report);

  return true;
 }


 //------------------------------------------------------------------------
 //ReportUnZip
 //recovers a zipped node report, assuming of course it was zipped by this same process 

bool NZ_MOOSApp::ReportUnZip(){

  if(zip_report.length() == 0){  
    cout << "pNodeZip: didn't find the zipped report with name: " << zipReportVar << endl;
    return false; //returning false doesn't actually do anything right now, but it might make
                  // future improvements easier?
  }


 int namepos = zip_report.find_first_of(",", 0);
 int spdpos = zip_report.find_first_of("," , namepos + 1);
 int hdgpos = zip_report.find_first_of("," , spdpos + 1);
 int latpos = zip_report.find_first_of("," , hdgpos + 1);
 int lonpos = zip_report.length();
  
 
 string namefield = zip_report.substr(0, namepos);
 string spdfield = zip_report.substr(namepos + 1, spdpos - namepos - 1);
 string hdgfiled = zip_report.substr(spdpos + 1,  hdgpos - spdpos - 1);
 string latfield = zip_report.substr(hdgpos + 1, latpos - hdgpos - 1);
 string lonfield = zip_report.substr(latpos +1, lonpos - latpos - 1);


 //see comment in ReportZip method if the biglat / biglon stuff is confusing. 
 string biglat$;
 int biglat;
 if(latfield.compare(0 , 1 , "2" , 0 , 1) == 0)
   biglat = (int) latOrigin + 1;
 else if(latfield.compare(0 , 1 , "1" , 0 , 1) == 0)
   biglat = (int) latOrigin - 1;
 else biglat = (int) latOrigin;

 stringstream ss;
 ss << biglat;
 biglat$ = ss.str();

 latfield = biglat$ + "." + latfield.substr(1,latfield.length() - 1);
 	 
 string biglon$;
 int biglon;	 
 if(lonfield.compare(0 , 1 , "2" , 0 , 1) == 0)
   biglon = (int) lonOrigin + 1;
 else if(lonfield.compare(0, 1 , "1" , 0 , 1) == 0)
   biglon = (int) lonOrigin - 1;
 else biglon = (int) lonOrigin;

 ss.str("");
 ss << biglon;
 biglon$ = ss.str();

 lonfield = biglon$ + "." + lonfield.substr(1,latfield.length() - 1);


 float lat = atof(latfield.c_str());
 float lon = atof(lonfield.c_str());

 string xfield;
 string yfield;
 double xUTM = 0;
 double yUTM = 0;
 
 geodesy.LatLong2LocalGrid(lat, lon, xUTM, yUTM);   //returns X,Y

  ss.str("");  //to_string is still not working
  ss << xUTM;
  xfield = ss.str();
  
  ss.str("");
  ss << yUTM; 
  yfield = ss.str();

  float spd = atof(spdfield.c_str());
  spd/=10;
  ss.str("");
  ss << spd;
  spdfield = ss.str();

  ss.str("");
  ss << index;
  string indexfield = ss.str();
  
  ss.str("");
  ss << MOOSTime();
  string timefield = ss.str();

  //making some conservative assumptions about the vehicle. 
  node_report = ("NAME=" + namefield + ",X=" + xfield + ",Y=" + yfield + ",SPD=" + spdfield + ",DEP=0"
                 + ",LAT=" + latfield + ",LON=" + lonfield + "TYPE=CONTACT,MODE=DRIVE,ALLSTOP=clear,index="
                 + indexfield + ",YAW=180,TIME=" + timefield + ",LENGTH=100");

  Notify(nodeReportVar, node_report);

   return true;

 }
