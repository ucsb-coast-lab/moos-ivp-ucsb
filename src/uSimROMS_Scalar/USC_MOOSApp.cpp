/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: USC_MOOSApp.cpp                                      */
/*    DATE: Jan 4th, 2011                                        */
/*                                                               */
/* This program is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation; either version  */
/* 2 of the License, or (at your option) any later version.      */
/*                                                               */
/* This program is distributed in the hope that it will be       */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE. See the GNU General Public License for more details. */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with this program; if not, write to the Free    */
/* Software Foundation, Inc., 59 Temple Place - Suite 330,       */
/* Boston, MA 02111-1307, USA.                                   */
/*****************************************************************/

#include <iostream>
#include <cmath>
#include <fstream>
#include "USC_MOOSApp.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "CurrentField.h"
#include <string>

using namespace std;

//------------------------------------------------------------------------
// Constructor

USC_MOOSApp::USC_MOOSApp() 
{
  m_pending_cfield = false;
  m_post_var = "USM_FORCE_VECTOR";
  m_posx     = 0;
  m_posy     = 0;
  
  m_posx_init = false;
  m_posy_init = false;
}

//------------------------------------------------------------------------
// Procedure: OnNewMail
//      Note: 

bool USC_MOOSApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
  CMOOSMsg Msg;
  
  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &Msg = *p;
    string key = Msg.m_sKey;
    double dval = Msg.m_dfVal;
    string sval = Msg.m_sVal;

    if(key == "NAV_X") {
      m_posx = dval;
      m_posx_init = true;
    }
    else if(key == "NAV_Y") {
      m_posy = dval;
      m_posy_init = true;
    }
  }
  
  return(true);
}

//------------------------------------------------------------------------
// Procedure: OnStartUp
//      Note: 

bool USC_MOOSApp::OnStartUp()
{
  cout << "SimCurrent Starting" << endl;
  
  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);
    
  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine  = *p;
    string param  = stripBlankEnds(MOOSChomp(sLine, "="));
    string value  = stripBlankEnds(sLine);
    //double dval   = atof(value.c_str());

    param = toupper(param);

    if(param == "SCALAR_FIELD_ACTIVE"){
      if (value == "true"){
	scalar_field_active = true;
	}
      else scalar_field_active = false;
    }
    if(param == "SCALAR_FILE_NAME"){
      scalar_file_name = value;
    }

    if(param == "CURRENT_FIELD") {
      bool ok = setCurrentField(value);
      if(!ok)
	cout << "Bad Current Field" << endl;
      m_pending_cfield = true;
    }
    if(param == "CURRENT_FIELD_ACTIVE"){
      m_cfield_active = (tolower(value) == "true");
    }
    if(param == "TOLERANCE"){
      tol = atoi(value.c_str());
      tol = RoundUp(tol, 10); 
   }
  }
  // look for latitude, longitude global variables
  double latOrigin, longOrigin;
  if(!m_MissionReader.GetValue("LatOrigin", latOrigin))
    cout << "pSimCurrent: LatOrigin not set in *.moos file." << endl;
  else if(!m_MissionReader.GetValue("LongOrigin", longOrigin))
    cout << "pSimCurrent: LongOrigin not set in *.moos file" << endl;
  else
    m_current_field.initGeodesy(latOrigin, longOrigin);
  
  
  registerVariables();

  setScalarField(scalar_file_name);
 
  cout << "SimCurrent started" << endl;
  return(true);
}


//------------------------------------------------------------------------
// Procedure: OnConnectToServer
//      Note: 

bool USC_MOOSApp::OnConnectToServer()
{
 
  registerVariables();
  cout << "SimCurrent connected" << endl;
  
  
  return(true);
}

//------------------------------------------------------------------------
// Procedure: registerVariables()
//      Note: 

void USC_MOOSApp::registerVariables()
{
  m_Comms.Register("NAV_X", 0);
  m_Comms.Register("NAV_Y", 0);
}

//------------------------------------------------------------------------
// Procedure: OnDisconnectFromServer
//      Note: 

bool USC_MOOSApp::OnDisconnectFromServer()
{
  return(true);
}

//------------------------------------------------------------------------
// Procedure: Iterate
//      Note: This is where it all happens.

bool USC_MOOSApp::Iterate()
{
  if(m_pending_cfield)
    postCurrentField();

   postScalarField();  //may have to add a timer/counter later if this posts way too fast

  if(!m_posx_init || !m_posy_init)
    return(false);

  double force_x, force_y;
  m_current_field.getLocalForce(m_posx, m_posy, force_x, force_y);

  double ang = relAng(0, 0, force_x, force_y);
  double mag = hypot(force_x, force_y);

  string ang_str = doubleToStringX(ang, 2);
  string mag_str = doubleToStringX(mag, 2);

  string vector_str = ang_str + "," + mag_str;

  if(vector_str != m_prev_posting) {
    Notify(m_post_var, vector_str);
    m_prev_posting = vector_str;
  }
  return(true);
}

//------------------------------------------------------------------------
// Procedure: postCurrentField
//      Note: Publishes the following two variables:
//            USC_CFIELD_SUMMARY - one posting for the whole field.
//            VIEW_VECTOR - one for each element in the field.
//  Examples:
//  USC_CFIELD_SUMMARY = field_name=bert, radius=12, elements=19
//         VIEW_VECTOR = x=12, y=-98, mag=3.4, ang=78, label=02


void USC_MOOSApp::postCurrentField()
{
  unsigned int fld_size = m_current_field.size();
  if(fld_size == 0)
    return;

  m_pending_cfield = false;

  string cfield_name   = m_current_field.getName();
  string cfield_radius = doubleToString(m_current_field.getRadius());
  cfield_radius = dstringCompact(cfield_radius);
  string cfield_size = doubleToString(fld_size);
  cfield_size = dstringCompact(cfield_size);

  string summary = "field_name=" + cfield_name;
  summary += ", radius=" + cfield_radius;
  summary += ", elements=" + cfield_size;
  Notify("USC_CFIELD_SUMMARY", summary);

  unsigned int i;
  for(i=0; i<fld_size; i++) {
    double xval = m_current_field.getXPos(i);
    double yval = m_current_field.getYPos(i);
    double fval = m_current_field.getForce(i);
    double dval = m_current_field.getDirection(i);
    
    string xstr = doubleToStringX(xval,2);
    string ystr = doubleToStringX(yval,2);
    string fstr = doubleToStringX(fval,2);
    string dstr = doubleToStringX(dval,2);
    string id   = uintToString(i);
    
    string msg = "xpos=" + xstr + ",ypos=" + ystr + ",mag=" + fstr;
    msg += ",ang=" + dstr + ",label=" + cfield_name + "_" + id;
    Notify("VIEW_VECTOR", msg);
  }
}


//--------------------------------------------------------------------- 
// Procedure: setCurretField                                      

bool USC_MOOSApp::setCurrentField(string filename)
{
  m_current_field = CurrentField();
  m_current_field.populate(filename);
  m_current_field.print();
  return(true);
}

//--------------------------------------------------------------------
// Procedure: setScalarField
// purpose: reads scalar values from a given file format should look like the following
// [number of entries (number of x y pairs)] [max xval] [max yval]
// [xcoord]  [ycoord]  [scalar value]
// [xcoord2] [ycoord2] [scalar value2]
//  .
//  .
//  .
// [xcoordn] [ycoordn] [scalar valuen]
// all entries should be nothing but numbers

void USC_MOOSApp::setScalarField(string filename){
  ifstream fin;
  fin.open(filename.c_str(),ios::in);
  if (!fin.is_open()){
    cout << "error opening file!" << endl;
    return;
  }
  int entries;
  int xtemp, ytemp;

  fin >> entries;
  fin >> m_xmax;
  fin >> m_ymax;


  //initialize all entries within range to zero
  //I used twice the max values to account for negative x and y coordinates. so in the setup I have 
  //the index of the m_scalar matrix coresponding to the x and y max values cooresponds to the point 0,0.
  m_scalar = new double*[(2* m_xmax/10)];
  for (int i = 0; i < (2*m_xmax)/10 ;i++){
	   m_scalar[i] = new double[2*m_ymax/10];
    }
  for(int i = 0 ; i < 2*m_xmax/10; i++){
    for(int j = 0; j < 2* m_ymax/10; j++){
      m_scalar[i][j] = 0;
    }
}
  //set specified coordinate values, round down to nearest tenth if the input was screwed up
  for(int i = 0; i < entries; i++){
    fin >> xtemp;
    fin >> ytemp; 
    xtemp = RoundDown(10, xtemp);
    ytemp = RoundDown(10,ytemp);
 

     xtemp = (xtemp + m_xmax) / 10;
     ytemp = (ytemp + m_ymax) / 10;

    fin >> m_scalar[xtemp][ytemp];
    cout << "setting scalar value at " << xtemp-m_xmax/10 << " " << ytemp-m_ymax/10 << " to: " << m_scalar[xtemp][ytemp] << endl;
  }

}


//------------------------------------------------------------------
//Procedure: postScalarField
//Purpose:post the scalar value at the current position in a variable called "SCALAR_VALUE", checks within a
//tolerance set by a paramter called (fittingly enough) "TOLERANCE" set in the moos file. this is so the x y 
//scalar values don't act like points, but specify a grid. as of right now there's no averaging if there's
// more than one value in the tolerance range, the program will post two reports, but this can be changed easily 
//if need be

void USC_MOOSApp::postScalarField(){
   bool posted = false;
   string scalarstr;
  

   //as I've said above, to account for negative coordinates the value of m_scalar[m_xmax][m_ymax] cooresponds
   //to the point  0,0. simulary the value at m_scalar[0][0] coressponds to the minimum x and y values allowed
   //and vice versa.
   
   for(int i = ((m_xmax + m_posx) - tol)/10 ; i < ((m_xmax + m_posx) + tol)/10 ; i++){
     for(int j = ((m_ymax + m_posy) - tol)/10 ; j < ((m_ymax + m_posy) + tol)/10; j++){
      if (m_scalar[i][j] != 0){

	//xstr = to_string(i);   the to_string method isn't being recognized, which i've determined to be a 
	//ystr = to_string(j);   complier error, i've used a work around, but it's something that should be 
	//                       fixed eventually. here's a link I found to someone who found and resolved the
	//                       same problem https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52015
	
	        posted = true;
		scalarstr = doubleToStringX(m_scalar[i][j],2);
		string msg = scalarstr;
		Notify("SCALAR_VALUE", msg);
      }
     }
    }

     if(!posted){
       string msg = "0";
       Notify("SCALAR_VALUE",msg);   //if no value in the range was nonzero, post that the scalar value at the 
                                     //current position as zero
     }
}


//---------------------------------------------------------------------
//rounds number up to the nearest whole multiple of the base number
int RoundUp(int base, int number)
{
 if (number % base != 0)
    {
      for(int i = 0; i < base ; i++)
	{
	if ((number + i) % base == 0)
	  {
	  number += i;
	  }
        }
     }
 return number; 
}

//--------------------------------------------------------------------
//rounds number down to the nearest whole multiple of the base number
int RoundDown(int base, int number)
{
 if (number % base != 0)
    {
      for(int i = 0; i < base ; i++)
	{
	if ((number - i) % base == 0)
	  {
	  number -= i;
	  }
        }
     }
 return number; 
}
