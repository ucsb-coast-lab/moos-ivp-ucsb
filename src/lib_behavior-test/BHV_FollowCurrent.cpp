/*===================================================================
File: BHV_FollowCurrent.cpp
Authors: Nick Nidzieko & Sean Gillen
Date: Jan/22/15
Origin: Horn Point Laboratory
Description: An IvP behavior meant to follow a current until it 
             reaches original waypoint specified in the bhv the
	     file. 

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

===================================================================*/

#include <cstdlib>
#include <iostream>
#include <math.h>
#include "BHV_FollowCurrent.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "BuildUtils.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"
#include <stdio.h>

using namespace std;

//-----------------------------------------------------------
// Procedure: Constructor

BHV_FollowCurrent::BHV_FollowCurrent(IvPDomain gdomain) : 
  IvPBehavior(gdomain)
{
  IvPBehavior::setParam("name", "follow_current");
  m_domain = subDomain(m_domain, "course,speed");

  // All distances are in meters, all speed in meters per second
  // Default values for configuration parameters 
  m_desired_speed  = 0; 
  m_arrival_radius = 10;
  // Default values for behavior state variables
  m_osx  = 0;
  m_osy  = 0;
  m_dir_set = 0;

  addInfoVars("NAV_X, NAV_Y");
  cout << "constructed" << endl;
}

//---------------------------------------------------------------
// Procedure: setParam - handle behavior configuration parameters

bool BHV_FollowCurrent::setParam(string param, string val) 
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);
  double double_val = atof(val.c_str());
  if((param == "ptx")  && (isNumber(val))) {
    m_nextpt.set_vx(double_val);
    m_origpt.set_vx(double_val);
    return(true);
  }
  else if((param == "pty") && (isNumber(val))) {
    m_nextpt.set_vy(double_val);
    m_origpt.set_vy(double_val);
    return(true);
  }
  else if((param == "speed") && (double_val > 0) && (isNumber(val))) {
    m_desired_speed = double_val;
    return(true);
  }
  else if((param == "radius") && (double_val > 0) && (isNumber(val))) {
    m_arrival_radius = double_val;
    cout << "BHV_FollowCurrent : using " << m_arrival_radius << " as the radius" << endl; 
    return(true);
  }
  else if(param == "current_var"){
    m_invar = val;
    addInfoVars(m_invar);
    return(true);
  }
  else if(param == "direction"){
    if(val == "upstream"){
      m_dir = 1;
      m_dir_set = 1;
      return(true);
    }
    else if(val == "downstream"){
      m_dir = -1;
      m_dir_set = 1;
      return(true);
    }
    else {
      cout << "direction value not recognized, please use \"upstream\" or \"downstream\"" << endl;
      return(false);
    }
  }
  return(false);
}

//-----------------------------------------------------------
// Procedure: onIdleState

void BHV_FollowCurrent::onIdleState() 
{
  postViewPoint(false);
}

//-----------------------------------------------------------
// Procedure: postViewPoint

void BHV_FollowCurrent::postViewPoint(bool viewable) 
{
  m_nextpt.set_label(m_us_name + "'s next waypoint");
  
  string point_spec;
  if(viewable)
    point_spec = m_nextpt.get_spec("active=true");
  else
    point_spec = m_nextpt.get_spec("active=false");
  postMessage("VIEW_POINT", point_spec);
}

//-----------------------------------------------------------
// Procedure: onRunState

IvPFunction *BHV_FollowCurrent::onRunState() 
{
  if(!m_dir_set){      //I'd rather not check this every time, but I can't find a place
                       //to only call it once within a behavior. 
    cout << "direction parameter not set please set to \"upstream\" or \"downstream\"" << endl;
  }
  
  // Part 1: Get vehicle position from InfoBuffer and post a 
  // warning if problem is encountered
  bool ok1, ok2;
  m_osx = getBufferDoubleVal("NAV_X", ok1);
  m_osy = getBufferDoubleVal("NAV_Y", ok2);
  if(!ok1 || !ok2) {
    postWMessage("No ownship X/Y info in info_buffer.");
    return(0);
  }

  string current_vec = getBufferStringVal(m_invar, ok1); //re assigns ok1
  
  string x_str;
  string y_str;
  
  if(ok1){
    parseCurrentVector(current_vec, x_str, y_str); 
    m_nextpt.shift_x(m_dir * atof(x_str.c_str()));
    m_nextpt.shift_y(m_dir * atof(y_str.c_str()));
  }
  //cout << "after shift: x = " << m_nextpt.x() << "  y = " << m_nextpt.y() << endl;
  
  // Part 2: Determine if the vehicle has reached the destination 
  // point and if so, declare completion.
#ifdef WIN32
  double dist = _hypot((m_nextpt.x()-m_osx), (m_nextpt.y()-m_osy));
#else
  double dist = hypot((m_origpt.x()-m_osx), (m_origpt.y()-m_osy));
  cout << "dist = " << dist << endl;
  cout << "origpoint x , y = " << m_origpt.x() << " " << m_origpt.y() << endl;
  cout << "osx = " << m_osx << endl;
  cout << "osy = " << m_osy << endl;
#endif
  if(dist <= m_arrival_radius) {
    setComplete();
    cout << "all done!" << endl;
    postViewPoint(false);
    return(0);
  }
  // Part 3: Post the waypoint as a string for consumption by 
  // a viewer application.
  postViewPoint(true);

  // Part 4: Build the IvP function with either the ZAIC tool 
  // or the Reflector tool.
  IvPFunction *ipf = 0;
  
    ipf = buildFunctionWithZAIC();
    
  if(ipf == 0) 
    postWMessage("Problem Creating the IvP Function");
  if(ipf)
    ipf->setPWT(m_priority_wt);

  if(ok1){
    m_nextpt.shift_x(m_dir * -(atof(x_str.c_str())));
    m_nextpt.shift_y(m_dir * -(atof(y_str.c_str())));
  }
  cout << "after shiftback: x = " << m_nextpt.x() << "  y = " << m_nextpt.y() << endl;
  
  return(ipf);
}

//-----------------------------------------------------------
// Procedure: buildFunctionWithZAIC

IvPFunction *BHV_FollowCurrent::buildFunctionWithZAIC() 
{
  ZAIC_PEAK spd_zaic(m_domain, "speed");
  spd_zaic.setSummit(m_desired_speed);
  spd_zaic.setPeakWidth(0.5);
  spd_zaic.setBaseWidth(1.0);
  spd_zaic.setSummitDelta(0.8);  
  if(spd_zaic.stateOK() == false) {
    string warnings = "Speed ZAIC problems " + spd_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }
  
  double rel_ang_to_wpt = relAng(m_osx, m_osy, m_nextpt.x(), m_nextpt.y());
  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit(rel_ang_to_wpt);
  crs_zaic.setPeakWidth(0);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setSummitDelta(0);  
  crs_zaic.setValueWrap(true);
  if(crs_zaic.stateOK() == false) {
    string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }

  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();
  IvPFunction *crs_ipf = crs_zaic.extractIvPFunction();

  OF_Coupler coupler;
  IvPFunction *ivp_function = coupler.couple(crs_ipf, spd_ipf, 50, 50);

  return(ivp_function);
}

//-----------------------------------------------------------
// Procedure: parseCurrentString

bool BHV_FollowCurrent::parseCurrentVector(string vector, string& x_str, string& y_str){
  int i;
  for(i = 0; vector[i] != ',';i++);
  x_str = vector.substr(0 , i);
  
  int comma_pos = i;
  for(; i < vector.length(); i++);
  y_str = vector.substr((comma_pos + 1), (i - comma_pos));//a bit ugly, but it works

  return true;
}


 
