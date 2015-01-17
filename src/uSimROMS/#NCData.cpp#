//new uSimROMS3 reads in data from a specified NCFile that contains ROMS output. right now the program can only read
//in scalar ROMS variables but this may be updated in future versions. the program first finds the 4 closest 
//points to the current lat/lon position, and using depth and altitude finds the closest 2 depth levels. it then 
// does an inverse weighted average of the values at all these points based on the distance from the current
// location. if there are time values in the future from the current time, the program will also take an inverse 
// weighted average of what the value would be at the two nearest time steps. if there is only one time step, or 
// if the last time step has been passed, the program simply uses the most recent time step as representing 
// the most accurate values
//
// NJN:2014-12-09: Added check for land values based on mask_rho
// NJN:2014-12-11: Fixd indexing to (x,y,z,t) = (i,j,k,n) and corrected distance calculations 
//                 (x pos was diffing against northing rather than easting)

#include <iostream>
#include <cmath>
#include <fstream>
#include "NCData.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include <string>
#include <ctime>
using namespace std;

//------------------------------------------------------------------------
// Constructor

NCData::NCData() 
{
  // Initialize state vars
  m_posx     = 0;
  m_posy     = 0;
  m_depth    = 0;
  m_head     = 0;
  m_rTime = "2010-10-30 12:34:56";
  time_step = 0;

  //defualt values for things
  maskRhoVarName = "mask_rho";        
  latVarName = "lat_rho";        
  lonVarName = "lon_rho";
  sVarName = "s_rho";
  timeVarName = "ocean_time";
  bathyVarName = "h";
  scalarOutputVar = "SCALAR_VALUE";
  safeDepthVar = "SAFE_DEPTH";
  look_fwd = 50;

  time_message_posted = false;      

  bad_val = -1;

 
}


//---------------------------------------------------------------------
// Procedure: LatLongtoIndex
// notes: stores the 4 closest index pairs well as their distance from the given x , y coordinate. if no lat lon pairs are within 
//        1 (may be lowered) then we assume we are outside the grid, and return false. this is very primitive
//        (read: slow) right  now, but may be sped up later using a more sophisticated data structure if this
//        way is not tennable.

bool NCData::LatLontoIndex(int eta[4], int xi[4], double dist[4], double x , double y)
{

  int chk_dist = 100000; //distance to check for grid points, if nothing pops up we assume we're outside the grid(hardcoded for now)
  //intialize the arrays we'll be storing things in
  for(int i = 0; i < 4; i++){
    eta[i] = 0;
    xi[i] = 0;
    dist[i] = chk_dist;
  }  

  //these nested fors go through the entire ROMS grid searching for the 4 closest index pairs to the current lat
  //lon coordinate.
  for(int j = 0; j < eta_rho; j++)
    {
      for(int i = 0; i < xi_rho; i++){
       if(pow(meters_n[j][i] - y,2) + pow(meters_e[j][i] - x, 2) < pow(dist[0],2)){
	dist[3] = dist[2];
	   eta[3] = eta[2];
	   xi[3] = xi[2];
	dist[2] = dist[1];
	   eta[2] = eta[1];
	   xi[2] = xi[1];
	dist[1] = dist[0];
	   eta[1] = eta[0];
	   xi[1] = xi[0];
	   dist[0] = sqrt((pow(meters_n[j][i] - y, 2) + pow(meters_e[j][i] - x, 2)));
	   eta[0] = j;
	   xi[0] = i;
      }
     else if(pow(meters_n[j][i] - y,2) + pow(meters_e[j][i] - x, 2) < pow(dist[1],2)){
	dist[3] = dist[2];
	   eta[3] = eta[2];
	   xi[3] = xi[2];
	dist[2] = dist[1];
	   eta[2] = eta[1];
	   xi[2] = xi[1];
	   dist[1] = sqrt((pow(meters_n[j][i] - y,2) + pow(meters_e[j][i] - x, 2)));
	   eta[1] = j;
	   xi[1] = i;
      }
     else if(pow(meters_n[j][i] - x,2) + pow(meters_e[j][i] - y, 2) < pow(dist[2],2)){
	dist[3] = dist[2];
	   eta[3] = eta[2];
	   xi[3] = xi[2];
	   dist[2] = sqrt((pow(meters_n[j][i] - y,2) + pow(meters_e[j][i] - x, 2)));
	   eta[2] = j;
	   xi[2] = i;
      }
     else if(pow(meters_n[j][i] - y,2) + pow(meters_e[j][i] - x, 2) < pow(dist[3],2)){
       dist[3] = sqrt((pow(meters_n[j][i] - y,2) + pow(meters_e[j][i] - x, 2)));
	  eta[3] = j;
	  xi[3] = i;
      }
	 
    }     
} 

  // printf("distances in latlon to index\n: , %f %f %f %f" , dist[0], dist[1], dist[2], dist[3]);
  //when the loop exits the index with the closest lat/lon pair to the current position will be in i and j 
  //if none of the values were close return false
  if(dist[0] ==  chk_dist || dist[1] == chk_dist || dist[2] == chk_dist || dist[1] == chk_dist)
    {
     cout <<"NCData: error : current lat lon pair not found in nc file " << endl;
     for(int i = 0; i < 4; i ++)
       {
       eta[i]  = 0; //these zeroes aren't used for anything, but having junk values is bad
       xi[i] = 0;
       }
    return false;
    }else return true;
 
}


//----------------------------------------------------------------------
//Procedure: GetS_rho
//notes: takes altitude and depth along with data on s_values pulled fro the NC file to get the current s_rho coordinate
// NJN: 2014/12/03: re-wrote routine to find nearest sigma levels
 bool NCData::GetS_rho(){
   double * s_depths = new double[s_rho];
   for(int i = 0; i < s_rho; i++){
     // sigma[0] = -1 = ocean bottom
     // sigma[s_rho] = 0 = free surface
     s_depths[i] = -s_values[i] * floor_depth;  
     //cout << s_depths[i] << endl;
   }

   // Find last sigma level deeper than current depth
   // e.g. vehicle depth of 1.5 m on a grid with
   // s_depths = [2.2 1.7 1.2 0.7 0.2] should find
   // s_level = 1 (that is: sigma[1] = 1.7 is the
   //                  last depth below.)
   int k = 0;
   while ((k < s_rho) && (s_depths[k] > m_depth)){
     k++;
   }
   s_level = k - 1;
   
   // Check for the special cases of being above the surface bin
   // or below the bottom bin.
   if (s_level > 0){
     distSigma = s_depths[s_level] - m_depth;
   } else {
     distSigma = -1;
   }
   if (s_level == s_rho - 1){
     distSp1 = -1;
   }else{
     distSp1 = m_depth - s_depths[s_level + 1];
   }

   //   cout << "uSR: Vehicle depth " << m_depth << endl;
   //cout << "uSR: s_level       " << s_level << endl;
   //cout << "uSR: distSigma     " << distSigma << endl;
   //cout << "uSR: distSp1       " << distSp1 << endl;
   return true;
 }


//---------------------------------------------------------------------
//GetTimeInfo
// notes: should check if there are any more time values, determine the current time step, and the time difference 
//        between the current time and the two nearest time steps.
bool NCData::GetTimeInfo(){

  // NJN: 20141210: -This was getting the current time to index into
  //                the ROMS file, but it was getting the MOOStime, 
  //                not the time that's on REMUS. 
  //                -Also, without an argument, MOOSTime returns the 
  //                current time (in UNIX) times the warp.
  //                -I'll leave this here for debugging.
  //double current_time = MOOSTime(false);  
  //std::time_t t = static_cast<time_t>(current_time);
  //struct tm * moosTime;
  //moosTime = std::gmtime( &t);
  //cout << "MOOS Time:  " << asctime(moosTime);

  std::time_t rt;
  std::time_t et;
  struct tm * remusTime;
  struct tm epoch;
  double current_time;
  int year, month, day, hour, min, sec;
  
  //cout << "USR::GetTimeInfo: setting unix epoch" << endl;
  epoch = *gmtime(&et);
  epoch.tm_year = 1970 - 1900;
  epoch.tm_mon = 1 - 1;
  epoch.tm_mday = 1;
  epoch.tm_hour = 0;
  epoch.tm_min = 0;
  epoch.tm_sec = 0;

  //cout << "USR::GetTimeInfo: Adjusting REMUS time" << endl;
  remusTime = std::gmtime(&rt);
  sscanf(m_rTime.c_str(), "%d-%d-%d %d:%d:%d",&year,&month,&day,&hour,&min,&sec);
  remusTime->tm_hour = hour;
  remusTime->tm_min = min;
  remusTime->tm_sec = sec;
  remusTime->tm_mday = day;
  remusTime->tm_mon = month - 1;
  remusTime->tm_year = year - 1900;
  mktime(remusTime);
  //cout << "USR::GetTimeInfo: REMUS Time = " << asctime(remusTime);
  //cout << "USR::GetTimeInfo: UNIX epoch = " << asctime(&epoch);

  // mktime converts to time_t
  current_time = difftime(mktime(remusTime), mktime(&epoch));
  //cout << "USR::GetTimeInfo: current time = " << current_time << endl;

  for(int i = 0; i < time_vals; i++){
  //  cout << time[n] << endl;
    if(current_time > time[i]){
      time_step = i;
    }
  }
  if (current_time > time[time_vals - 1]){ //if the current time is larger than the last time step then there are no
    more_time = false;                     //more time steps
  }else more_time = true;
  
  if(more_time){   //if there are more time values we need to know how close we are to the closest two time steps
    time_since = current_time - time[time_step];
    time_until = time[time_step + 1] - current_time; 
  }
  return true;
}

//---------------------------------------------------------------------
//GetValue
//notes: gets values at both the two closest time steps and does an inverse weighted average on them
double NCData::GetValue(){
  double value;
  if(more_time){  //if theres more time we need to interpolate over time
    double val1 = GetValueAtTime(time_step);
    double val2  = GetValueAtTime(time_step + 1);
    
    if (val1 == bad_val || val2 == bad_val){ // if either of the values are bad, return bad so we don't publish bad data
      return bad_val;
    }
    double weights[2] = {time_since, time_until};
    double values[2] = {val1 , val2};
    int good[2] = {1, 1};
    value = WeightedAvg(values , weights , good, 2);
  }else{//if no future time values exist, just average the values around us at the most recent time step
    value = GetValueAtTime(time_step);
    if(!time_message_posted){
      cout << "NCData: warning: current time is past the last time step, now using data only data from last time step" << endl;
      time_message_posted = true; // we only want to give this warning once
    }
   }
  return value;

}
//---------------------------------------------------------------------
//GetValueAtTime
//notes: takes the closest position, takes an inverse weighted averege (using distance as weights) the 8 closest 
//      points, and spits out a value.
// NJN: 2014/11/17: Added limiter to the above/below level grab. 
// NJN: 2014/12/03: Modified for new sigma-level extraction, indexes good values
//
double NCData::GetValueAtTime(int t){

  double dz[2] = {distSigma, distSp1};
  double s_z[2] = {0, 0};
  int good_z[2] = {0, 0};
  double s_xy[4];
  int good_xy[4]; //keeps track of how many good values we have so they don't skew the returned value
  double value_t; // To be returned
  
  for(int k = 0; k < s_rho; k++){
    //    cout << "i,j,k: " << closest_xi[0] << ", " << closest_eta[0] << ", " << k << "; salt: " << vals[t][k][closest_eta[0]][closest_xi[0]] << endl; 
  }
  for(int k = 0; k < 2; k++){
    // Initialize
    for(int i = 0; i < 4; i++){
      s_xy[i] = -1;
      good_xy[i] = 0;
    }
    // Then make sure we want this level
    if (dz[k] != -1){
      // Find the four corners
      for(int i = 0; i < 4; i++){
	// Check for Water = 1
	if (maskRho[closest_eta[i]][closest_xi[i]]){
	  // Get the value
	  s_xy[i] = vals[t][s_level + k][closest_eta[i]][closest_xi[i]];
	  good_xy[i] = 1;
	}
      }
      // Average this level
      s_z[k] = WeightedAvg(s_xy, closest_distance, good_xy, 4); 
      good_z[k] = 1;
    }
  }

  //cout << "uSR: s_z  " << s_z[0] << ", " << s_z[1] << endl;
  //cout << "uSR: dz  " << dz[0] << ", " << dz[1] << endl;
  //cout << "uSR: good_z  " << good_z[0] << ", " << good_z[1] << endl;
  // Average the values at the two s_level
  value_t = WeightedAvg(s_z , dz , good_z, 2);
  if (value_t == bad_val){
    cout << "NCData: Bad value at time step " << time_step << endl;
  }
  
  return value_t;
}

//------------------------------------------------------------------
//procedure : GetBathy
//notes: gets the bathymetry at the current location. uses an inverse weighted average
// NJN: 2014/12/03: Doesn't check for good values right now.
//
bool NCData::GetBathy(int eta[4], int xi[4], double dist[4], double &depth)
{
  double local_depths[4];
  int good[4];
  for(int i = 0; i < 4; i++){
    if (maskRho[eta[i]][xi[i]]){
      local_depths[i] = bathy[eta[i]] [xi[i]];
      good[i] = 1;
    }
  }
  depth = WeightedAvg(local_depths, closest_distance, good, 4);
}


//---------------------------------------------------------------------
//procedure: GetSafeDepth
//notes:interpolates the current best guess altitude with each of the 4 closest bathymetry points, it then returns the smallest depth found 
//
bool NCData::GetSafeDepth()
{ 
  int chk_x, chk_y;
  int chk_eta[4];
  int chk_xi[4];
  double chk_dist[4];
  double headRad = (90 - m_head)/180*3.1415; // convert from geo to math coords, then to radians
  chk_x = look_fwd*cos(headRad) + m_posx;
  chk_y = look_fwd*sin(headRad) + m_posy;
  LatLontoIndex(chk_eta, chk_xi, chk_dist, chk_x, chk_y);
  GetBathy(chk_eta, chk_xi, chk_dist, safe_depth);
  //cout << "Local bathy depth = " << floor_depth << endl;
  //cout << "Depth at " << look_fwd << " m ahead is " << safe_depth << endl;
      
}

//---------------------------------------------------------------------
//ConvertToMeters : converts the entire lat/lon grid in order to populate the northings and eastings grid
//
bool NCData::ConvertToMeters()
{
  meters_n = new double *[eta_rho];
  meters_e = new double *[eta_rho];

  for(int j = 0; j < eta_rho; j++){
    meters_e[j] = new double[xi_rho];
    meters_n[j] = new double[xi_rho];
  }
  
  for(int j = 0; j < eta_rho; j++){
    for(int i = 0; i < xi_rho; i++){
      geodesy.LatLong2LocalGrid(lat[j][i], lon[j][i], meters_n[j][i], meters_e[j][i]);
    }
  }
}








//---------------------------------------------------------------------
//procedure: WeightedAvg
//notes: this is actually a reverse weighted average. weights and averages the number of points given,
//        spits out the calculated value at the current lat/lon coordinate.
// NJN: 2014/12/03: Updated for good indexing
//
double WeightedAvg(double* values, double* weights, int* good, int num_vals)
{

  for(int i = 0; i < num_vals; i++){
    if(weights[i] == 0){
      weights[i] = .000000001;
    }
  }
  double numerator = 0;
  for(int i = 0; i < num_vals; i++){
    numerator += good[i]*values[i]/weights[i];
      }
  double denominator =0;
  for(int i = 0; i < num_vals; i++){
    denominator += good[i]/weights[i];
      }
  double  value =  numerator/denominator; 
    return value;
}
