//uSimROMS2 reads in data from a specified NCFile that contains ROMS output. right now the program can only read
//in scalar ROMS variables but this may be updated in future versions. the program first finds the 4 closest 
//points to the current lat/lon position, and using depth and altitude finds the closest 2 depth levels. it then 
// does an inverse weighted average of the values at all these points based on the distance from the current
// location. if there are time values in the future from the current time, the program will also take an inverse 
// weighted average of what the value would be at the two nearest time steps. if there is only one time step, or 
// if the last time step has been passed, the program simply uses the most recent time step as representing 
// the most accurate values


//I've started on adding the ability to ready bathy data, as of right now it is all commented out, so the program
//should retain all it's old functionality

#include <iostream>
#include <cmath>
#include <fstream>
#include "USC_MOOSApp2.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "CurrentField.h"
#include <string>
using namespace std;

//------------------------------------------------------------------------
// Constructor

USC_MOOSApp::USC_MOOSApp() 
{
  m_posx     = 0;
  m_posy     = 0;

  //defualt values for things
  latVarName = "lat_rho";        
  lonVarName = "lon_rho";
  sVarName = "s_rho";
  timeVarName = "ocean_time";
  bathyVarName = "h";
  scalarOutputVar = "SCALAR_VALUE";
  //  start_time = MOOSTime();   //for some reason MOOSTime, when called here, returns the 
                                 //the correct unix time, but when called anywhere else it doesn't. when called 
                                 //elsewhere it still counts so i can use it. but it's kind of weird and should
                                 //probably be looked into
  time_message_posted = false;      

  time_override = false;
  s_override = false;
  eta_override = false;
  xi_override = false;

  bottom_lock = false;
  //bathy_only = false;

  bad_val = 0;

 
}

//------------------------------------------------------------------------
// Procedure: OnNewMail
//      Note: reads messages from MOOSDB to update values

bool USC_MOOSApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
  CMOOSMsg Msg;
  bottom_lock = false;
  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &Msg = *p;
    string key = Msg.m_sKey;
    double dval = Msg.m_dfVal;
    string sval = Msg.m_sVal;

    if(key == "NAV_X") {
      m_posx = dval;
     
    }
    else if(key == "NAV_Y") {
      m_posy = dval;
      
    }
    else if(key == "NAV_DEPTH") {
      m_depth = dval;
    }
    else if(key == "NAV_ALTITUDE") {
      m_altitude = dval;
      bottom_lock = true;
  }
  }
  return(true);
}

//------------------------------------------------------------------------
// Procedure: OnStartUp
//      Note: initializes paramters based on what it finds in the moos file

bool USC_MOOSApp::OnStartUp()
{
  cout << "uSimROMS2: SimROMS2 Starting" << endl;
  
  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);
    
  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine  = *p;
    string param  = stripBlankEnds(MOOSChomp(sLine, "="));
    string value  = stripBlankEnds(sLine);
    //double dval   = atof(value.c_str());

    param = toupper(param);

    if(param == "NC_FILE_NAME"){  //required
       ncFileName = value;
     }
    if(param == "OUTPUT_VARIABLE"){  //defaults to SCALAR_VALUE
       scalarOutputVar = value;
       cout << "uSimROMS2: publishing under name: " << scalarOutputVar;
     }
    if(param == "SCALAR_VARIABLE"){  //e.g. salt or temprature
       varName = value;
     }
    if(param == "LAT_VARIABLE"){   //name of variable holding latitude data
       latVarName = value;
     }
    if(param == "LON_VARIABLE"){ //ditto LAT_VARIABLE
       lonVarName = value;
     }
    if(param == "TIME_VARIABLE"){ //ditto LAT_VARIABLE
       timeVarName = value;
     }
    if(param == "DEPTH_VARIABLE"){ //ditto LAT_VARIABLE
       sVarName = value;
     }
    if(param == "BATHY_VARIABLE"){ //ditto LAT_VARIABLE
      bathyVarName = value;
    }
    //for all the [something]_VALUES the program can find the number of variables automatically, so I wouldn't
    //recommend messing with them unless you have a good reason.
    if(param == "TIME_VALUES"){  
       time_vals = atoi(value.c_str());
       cout << "uSimROMS2: manually overriding auto allocation : using " << time_vals << " time values " << endl;
       time_override = true;
     }
     if(param == "DEPTH_VALUES"){
       s_rho = atoi(value.c_str());
       cout << "uSimROMS2: manually overriding auto allocation : using " << s_rho << "depth values " << endl;
       s_override = true;
     }
     if(param == "ETA_VALUES"){
       cout << "uSimROMS2: manaully overriding auto allocation : using " << value.c_str() << " eta values" << endl;
       eta_rho = atoi(value.c_str());
       eta_override = true;
     }
     if(param == "XI_VALUES"){
       cout << "uSimROMS2: manually overriding auto allocation : using "   << value.c_str() << " xi values" << endl;
       xi_rho = atoi(value.c_str());
       xi_override = true;
     }
     //this specifies the value that the ROMS file uses as a "bad" value, defaults to zero
     if(param == "BAD_VALUE"){
       bad_val = atof(value.c_str());
       cout << "uSimROMS2: using " << bad_val << " as the bad value" << endl;
     }
     /*     if(param == "BATHY_ONLY"){
       char* lower = value.c_str();    //this block ensure the users "true" is not case sensitive
       for(int i = 0; i < value.length(); i++){ 
	 lower[i] = tolower(lower[i]);
       }
       if (strcmp(lower , "true") ==  1){
	 bathy_only = true;
       }else batthy_only = false;
     }
     */
  }
  // look for latitude, longitude global variables
  double latOrigin, longOrigin;
  if(!m_MissionReader.GetValue("LatOrigin", latOrigin))
    cout << "uSimROMS2: LatOrigin not set in *.moos file." << endl;
  else if(!m_MissionReader.GetValue("LongOrigin", longOrigin))
    cout << "uSimROMS2: LongOrigin not set in *.moos file" << endl;
  else
  geodesy.Initialise(latOrigin, longOrigin);  //initializes the geodesy class  

  start_time = MOOSTime();  //starts the clock 
  
  if(!ReadNcFile()){    //loads all the data into local memory that we can actually use
    std::exit(0);        //if we can't read the file, exit the program so it's clear something went wrong and
  }                     //so we don't publish misleading or dangerous values, using exit() probably isn't the
                        //best way, but it works

  registerVariables();    

  cout << "uSimROMS2: SimROMS2 started" << endl;
  return(true);
}


//------------------------------------------------------------------------
// Procedure: OnConnectToServer
//      Note: 

bool USC_MOOSApp::OnConnectToServer()
{
 
  
  cout << "uSimROMS2: SimROMS2 connected" << endl;
  
  
  return(true);
}

//------------------------------------------------------------------------
// Procedure: registerVariables()
//      Note: 

void USC_MOOSApp::registerVariables()
{
  m_Comms.Register("NAV_X", 0);
  m_Comms.Register("NAV_Y", 0);
  m_Comms.Register("NAV_DEPTH", 0);
  m_Comms.Register("NAV_ALTITUDE",0);
}  

//------------------------------------------------------------------------
// Procedure: OnDisconnectFromServer
//      Note: 

bool USC_MOOSApp::OnDisconnectFromServer()
{
  cout << "uSimROMS2: uSimROMS2 is leaving" << endl;
  return(true);
}

//------------------------------------------------------------------------
// Procedure: Iterate
//      notes: this bad boy calls everything else

bool USC_MOOSApp::Iterate()
{
 
  double  value;
 
  if(GetS_rho()){ //GetS_rho only fails if there is no bottom lock
 
  GetTimeInfo();

  //there's no good reason for me to use LocalGrid2latLon on NAV_X and NAV_Y rather than just use NAV_LAT 
  // and NAV_LONG other than that I didn't know they existed at the time. but this should work fine
  geodesy.LocalGrid2LatLong( m_posx,  m_posy, current_lat , current_lon );  //returns lat and lon

  if(LatLontoIndex())   //returns eta and xi, returns false if we're outside the ROMS grid, in which 
    {                   //case we won't publish                                                        
    value = GetValue();
    if(value != bad_val)  //if the value is good, go ahead and publish it
      {
	//if(!bathy_only)
	//  {
	    Notify(scalarOutputVar.c_str(), value);
	    cout << "uSimROMS2: uSimROMS is publishing value :" << value << endl;
	    //  }
      } else cout << "uSimROMS2: all local values are bad, refusing to publish new values" << endl;//if values were bad, let the user know
    } else cout << "uSimROMS2: no value found at current location" << endl;//if we couldn't find a value let the user know

  }else cout << "uSimROMS2: error: no bottom lock, refusing to publish new values" << endl;


  // GetBathy()

  return(true);
}



 
//---------------------------------------------------------------------
// Procedure: ReadNcFile
// notes : should read scalar data from the specified ncfile into a four dimensional array called val
//         will also read in lat and lon coordinates coresponding to specific indexes as well as get all 
//         time steps and their values

bool USC_MOOSApp::ReadNcFile()
{
  cout << "uSimROMS2: opening file: " << ncFileName << endl;
  // these  dont actually do anything we need to concern ourselves with, but  the constructor requires them
  size_t* buffer = NULL;
  size_t size = 0;

  //open file
  NcFile File((const char*)ncFileName.c_str(), NcFile::ReadOnly , buffer , size , NcFile::Netcdf4); 
  if(!(File.is_valid())) //check if file is open
    {
      cout << "uSimROMS2: error reading file!" << endl;
      return false;
    } else cout <<"uSimROMS2: file opened successfully" << endl;

  //find specified variable
  NcVar* scalar_var = File.get_var((const char*) varName.c_str());
  if(!(scalar_var->is_valid())) //check if variable was valid
    {
      cout << "uSimROMS2: error reading scalar variable" << endl;
      return false;
    }else cout << "uSimROMS2: variable \"" << varName << "\" found" << endl; 

  
  //get the size of the array our variable is stored in, edge lenths apply to lat/lon and time variables too
   long* edge;
   edge =  scalar_var->edges();
   if(time_override == false){ 
     time_vals = edge[0];
     cout << "uSimROMS2: using "  << time_vals << " time values"  << endl;
   }
   if(s_override == false){
     s_rho = edge[1];
     cout << "uSimROMS2: using " << s_rho << " s values" << endl; 
   }
   if(eta_override == false){
     eta_rho = edge[2];
     cout << "uSimROMS2: using " << eta_rho << " eta values" << endl;
   }
   if(xi_override == false){
     xi_rho = edge[3];
     cout << "uSimROMS2: using " << xi_rho << " xi_values" << endl;
   }
  
  

  //find lat variable
  NcVar* lat_var = File.get_var((const char*) latVarName.c_str()); 
  if(!(lat_var->is_valid())) //check if variable was valid
    {
      cout << "uSimROMS2: error reading latitude variable, exiting" << endl;
      return false;
    }else cout << "uSimROMS2: latitude variable found" << endl;

  //find lon variable    
  NcVar* lon_var = File.get_var((const char* ) lonVarName.c_str()); 
  if(!(lat_var->is_valid())) //check if variable was valid
    {
      cout << "uSimROMS2: error reading longitude variable, exiting" << endl;
      return false;
    }else cout << "uSimROMS2: longitude variable found" << endl;

  //find time variable
  NcVar* time_var = File.get_var((const char*) timeVarName.c_str());
    if(!(time_var->is_valid())) //check if variable was valid
    {
      cout << "uSimROMS2: error reading time variable, exiting" << endl;
      return false;
    }else cout << "uSimROMS2: time variable found" << endl;

  //find depth variable 
  NcVar* s_var = File.get_var((const char*) sVarName.c_str());
  if(!(time_var->is_valid()))
    {
      cout << "uSimROMS2: error reading depth variable, exiting" << endl;
      return false;
    }else cout << "uSimROMS2: depth variable found" << endl;

  /* NcVar* bathy_var = File.get_var((const char*) bathyVarName.c_str());
  if(!bathy_var->is_valid())
    {
      cout << "uSimROMS2: error reading bathymetry variable, exiting" << endl;
      return false;
    }else cout << "uSimROMS2: bathymetry variable found" << endl;
  */
  
  //create a value array in local memory, read in scalar values
  vals = new double***[time_vals];
  for(int i = 0; i < time_vals; i ++)  //dynamic memory must be initialized row by row
    {
       vals[i] = new double**[s_rho];
       for(int j = 0; j < s_rho; j++)
          {
	    vals[i][j] = new double*[eta_rho];
	    for(int k = 0; k < eta_rho; k++)
	       {
	 	 vals[i][j][k] = new double[xi_rho];
	       }
	  }
      }


  //the netCDF get method can only read in values to contigous blocks of memory, meaning we have to get values
  //row by row.
  for(int i = 0; i < time_vals; i ++)
    {
       for(int j = 0; j < s_rho; j++)
	 {
	    for(int k = 0; k < eta_rho; k++)
	       {
		 scalar_var->set_cur(i,j,k,0);
	 	 scalar_var->get(&vals[i][j][k][0], 1, 1, 1, xi_rho);
		 
	       }
	  }
      }    
  cout << "uSimROMS2: field for \"" << varName << "\" populated" << endl;

  //create lat array in local memory, read in lat values
  lat = new double* [eta_rho];
    for(int i = 0; i < eta_rho; i++)
	lat[i] = new double[xi_rho];
    
    //just like the main variable, lon/lat need to be read in row by row
    for(int i = 0; i < eta_rho; i++) 
      {
	lat_var->set_cur(i,0);
	lat_var->get(&lat[i][0], 1, xi_rho);
      }

  cout << "uSimROMS2: latitude field populated" << endl;
 
   

  //create lon array in local memory, read in lon values
  lon = new double* [eta_rho];
    for(int i = 0; i < eta_rho; i++)
        lon[i] = new double[xi_rho];

    //also needs to be read in row by row
    for(int i = 0; i < eta_rho; i++)
      {
	lon_var->set_cur(i,0);
       lon_var->get(&lon[i][0], 1 , xi_rho);
      }       
    cout << "uSimROMS2: longitude field populated" << endl;

    //read in time values 
    time = new double [time_vals]; 
    time_var->get(&time[0], time_vals);
    cout << "time field populated" << endl;
    
    //makes time entries corespond to seconds since the simulator started
    double first_time = time[0];
    for(int i = 0; i < time_vals; i++){
      time[i] -= first_time;
    }   

    //read in s_rho values
    s_values = new double[s_rho];
    s_var->get(&s_values[0], s_rho);
    cout << "uSimROMS2: depth field populated" << endl;
    
 //create bathy array in local memory, read in lat values
  bathy  = new double* [eta_rho];
    for(int i = 0; i < eta_rho; i++)
	bathy[i] = new double[xi_rho];
    
    //needs to be read in row by row
    /*    for(int i = 0; i < eta_rho; i++) 
      {
	bathy_var->set_cur(i,0);
	bathy_var->get(&bathy[i][0], 1, xi_rho);
      }
    */
  cout << "uSimROMS2: bathymetry field populated" << endl;   
  
  return true;

 }




//---------------------------------------------------------------------
// Procedure: LatLongtoIndex
// notes: gives the index corresponding to the closest lat lon points. if no lat lon pairs are within 
//        1 (may be lowered) then we assume we are outside the grid, and return false. this is very primitive
//        (read: slow) right  now, but may be sped up later using a more sophisticated data structure if this
//        way is not tennable.

bool USC_MOOSApp::LatLontoIndex()
{
  //intialize the arrays we'll be storing things in
  for(int i =0; i < 4; i++){
    closest_eta[i] = 0;
    closest_xi[i] = 0;
    closest_distance[i] = 1;
  }


  //these nested fors go through the entire ROMS grid searching for the 4 closest index pairs to the current lat
  //lon coordinate.
  for(int i = 0; i < eta_rho; i++)
    {
      for(int j = 0; j < xi_rho; j++)
	{
       if(pow(lat[i][j] - current_lat,2) + pow(lon[i][j] - current_lon, 2) < pow(closest_distance[0],2)){
	closest_distance[3] = closest_distance[2];
	   closest_eta[3] = closest_eta[2];
	   closest_xi[3] = closest_xi[2];
	closest_distance[2] = closest_distance[1];
	   closest_eta[2] = closest_eta[1];
	   closest_xi[2] = closest_xi[1];
	closest_distance[1] = closest_distance[0];
	   closest_eta[1] = closest_eta[0];
	   closest_xi[1] = closest_xi[0];
	   closest_distance[0] = sqrt((pow(lat[i][j] - current_lat, 2) + pow(lon[i][j] - current_lon, 2)));
	   closest_eta[0] = i;
	   closest_xi[0] = j;
      }
     else if(pow(lat[i][j] - current_lat,2) + pow(lon[i][j] - current_lon, 2) < pow(closest_distance[1],2)){
	closest_distance[3] = closest_distance[2];
	   closest_eta[3] = closest_eta[2];
	   closest_xi[3] = closest_xi[2];
	closest_distance[2] = closest_distance[1];
	   closest_eta[2] = closest_eta[1];
	   closest_xi[2] = closest_xi[1];
	   closest_distance[1] = sqrt((pow(lat[i][j] - current_lat,2) + pow(lon[i][j] - current_lon, 2)));
	   closest_eta[1] = i;
	   closest_xi[1] =j;
      }
     else if(pow(lat[i][j] - current_lat,2) + pow(lon[i][j] - current_lon, 2) < pow(closest_distance[2],2)){
	closest_distance[3] = closest_distance[2];
	   closest_eta[3] = closest_eta[2];
	   closest_xi[3] = closest_xi[2];
	   closest_distance[2] = sqrt((pow(lat[i][j] - current_lat,2) + pow(lon[i][j] - current_lon, 2)));
	   closest_eta[2] = i;
	   closest_xi[2] = j;
      }
     else if(pow(lat[i][j] - current_lat,2) + pow(lon[i][j] - current_lon, 2) < pow(closest_distance[3],2)){
       closest_distance[3] = sqrt((pow(lat[i][j] - current_lat,2) + pow(lon[i][j] - current_lon, 2)));
	  closest_eta[3] = i;
	  closest_xi[3] = j;
      }

	 
    }
      
} 
  //when the loop exits the index with the closest lat/lon pair to the current position will be in i and j 
  //if none of the values were close return false
  if(closest_distance[0] ==  1 || closest_distance[1] == 1 || closest_distance[2] == 1 || closest_distance[1] == 1)
    {
     cout <<"uSimROMS2: error : current lat lon pair not found in nc file " << endl;
     for(int i = 0; i < 4; i ++)
       {
       closest_eta[i]  = 0; //these zeroes aren't used for anything, but we having junk values is always bad
       closest_xi[i] = 0;
       }
    return false;
    }else return true;
 
  

}


//----------------------------------------------------------------------
//Procedure: GetS_rho
//notes: takes altitude and depth and returns the s_rho value. which i'm assuming is a fraction of how deep the
//        water is at any given point.

//right now s_rho takes the size of the s_rho index and divides the floor depth into even steps. however
//what it shoudl do is take the floor depth and then multiply by each entry of s_rho to obtain the depth
//represented by each respective entry. this will complicate things a litte, but shouldn't be too difficult to
//manage and shouldn't require any changes outside of this method. 

 bool USC_MOOSApp::GetS_rho(){

   if(!bottom_lock){ //if we don't have a bottom lock don't break things by using data that doesn't exist
     return false;
   }
   
   double floor_depth = m_depth + m_altitude;  //finds the depth of the sea floor
   double * s_depths = new double[s_rho];
   for(int i = 0; i < s_rho; i++){
     s_depths[i] = s_values[i] * floor_depth;
   }
   dist_to_s_level = floor_depth;
   dist_to_next_s_level = floor_depth; 

   for(int i = 0; i < s_rho; i++){
     if(abs(s_depths[i] - m_depth) < dist_to_s_level){
       s_level = i;
       dist_to_next_s_level = dist_to_s_level;
       dist_to_s_level = abs(s_depths[i] - m_depth);
     }else if((abs(s_depths[i] - m_depth) < dist_to_next_s_level)){
       dist_to_next_s_level = ((abs(s_depths[i] - m_depth) < dist_to_next_s_level));
     }
   }

   if(m_depth - abs(s_depths[s_level]) > 0){
     above_s_level = true;           //above s_level refers to having a greater index, rather than a greater depth
   }else above_s_level = false;
     
   return true;
 }


//---------------------------------------------------------------------
//GetTimeInfo
// notes: should check if there are any more time values, determine the current time step, and the time difference 
//        between the current time and the two nearest time steps.
bool USC_MOOSApp::GetTimeInfo(){
  double current_time = MOOSTime() - start_time;
  for(int i = 0; i < time_vals; i++)
  {
    if(current_time > time[i])
     {
       time_step = i;
     }
  }
  if (current_time > time[time_vals - 1]) //if the current time is larger than the last time step then there are no
    {                                     //no more time steps
      more_time = false;
    }else more_time = true;

   if(more_time)   //if there are more time values we need to know how close we are to the closest two time steps
    {
      time_since = current_time - time[time_step];
      time_until = time[time_step + 1] - current_time; 
    }

    return true;
}

//---------------------------------------------------------------------
//GetValue
//notes: gets values at both the two closest time steps and does an inverse weighted average on them
double USC_MOOSApp::GetValue(){
  double value;
  if(more_time){  //if theres more time we need to interpolate over time
    double val1 = GetValueAtTime(time_step);
    double val2  = GetValueAtTime(time_step + 1);
    
    if (val1 == bad_val || val2 == bad_val){ // if either of the values are bad, return bad so we don't publish bad data
      return bad_val;
    }
    double weights[2] = {time_since, time_until};
    double values[2] = {val1 , val2};
    value = WeightedAvg(values , weights , 2);
  }else{//if no future time values exist, just average the values around us at the most recent time step
    value = GetValueAtTime(time_step);
    if(!time_message_posted){
      cout << "uSimROMS2: warning: current time is past the last time step, now using data only data from last time step" << endl;
      time_message_posted = true; // we only want to give this warning once
    }
   }
  return value;

}
//---------------------------------------------------------------------
//GetValueAtTime
//notes: takes the closest position, takes an inverse weighted averege(using distance as weights) the 8 closest 
//      points, and spits out a value.
double USC_MOOSApp::GetValueAtTime(int t){
  double local_vals[4];
  int    good_values = 0; //keeps track of how many good values we have so they don't skew the returned value
  for(int i = 0; i < 4; i++){
    if ( local_vals[i] = vals[t][s_level][closest_eta[i]][closest_xi[i]] != bad_val)
    {
       local_vals[i] = vals[t][s_level][closest_eta[i]][closest_xi[i]];
       good_values ++;
     }
    }
  
  //find average value at closest depth
  double value_s_1 = WeightedAvg(local_vals, closest_distance, good_values); //only average good points

  double value_s_2;
   good_values = 0;
   //using above_s_level we know if the next closest s_level is above or below us
	if(above_s_level)
	{
	    for(int i = 0; i < 4; i++)
	    {
	      if (local_vals[i] = vals[t][s_level + 1][closest_eta[i]][closest_xi[i]] != bad_val)
		{
		  local_vals[i] = vals[t][s_level + 1][closest_eta[i]][closest_xi[i]];
		  good_values ++;
		}
	      value_s_2 = WeightedAvg(local_vals, closest_distance, good_values);//only average good points
	    }

	}else
	{
	    for(int i = 0; i < 4; i++)
	    {
	      if ( local_vals[i] = vals[t][s_level - 1][closest_eta[i]][closest_xi[i]] != bad_val)
		{
		  local_vals[i] = vals[t][s_level - 1][closest_eta[i]][closest_xi[i]];
		  good_values ++;
		}
	    }
	    value_s_2 = WeightedAvg(local_vals, closest_distance, good_values);//only average good points
	}

	if(value_s_1 && value_s_2 == bad_val){
	  cout << "uSimROMS2: warning: all local values at time step " << time_step << " are bad, this probably means the vehicle has exited the range of valid values for the current nc file. or possibly something has gone horribly wrong." << endl;
	    return bad_val;
	}
        else if(value_s_1 == bad_val){
	  cout << "uSimROMS2: all bad_values found at current s_level" << endl;
	  return value_s_2;
	}
        else if(value_s_2 == bad_val){
	  cout << "uSimROMS2: all bad_values found at next closest s_level" << endl;
	  return value_s_1;
	}
	  
   double local_vals2[2] = {value_s_1, value_s_2};
   double level_distance[2] = {dist_to_s_level, dist_to_next_s_level};

   
   //average the values at the two s_level
   double value_t = WeightedAvg(local_vals2 , level_distance , 2);
  

   return value_t;
       
 
}

//------------------------------------------------------------------
//procedure : GetBathy
//notes: gets the bathymetry at the current location. uses an inverse weighted average
//bool USC_MOOSApp::GethBathy()
//{
// double 


//}

//---------------------------------------------------------------------
//procedure: WeightedAvg
//notes: this is actually a reverse weighted average. weights and averages the number of points given,
//        spits out the calculated value at the current lat/lon coordinate.
double USC_MOOSApp::WeightedAvg(double* values, double* weights, int num_vals)
{

  if(num_vals == 0){
    return bad_val;
  }

  for(int i = 0; i < num_vals; i++){
    if(weights[i] == 0){
    weights[i] = .000000001;
    }
  }
  double numerator = 0;
  for(int i = 0; i < num_vals; i++){
    numerator += values[i]/weights[i];
      }
  double denominator =0;
  for(int i = 0; i < num_vals; i++){
    denominator += 1/weights[i];
      }
  double  value =  numerator/denominator; 
    return value;
}
