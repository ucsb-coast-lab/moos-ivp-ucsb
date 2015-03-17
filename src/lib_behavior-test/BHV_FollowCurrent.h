/*====================================================================
File: BHV_FollowCurrent.h
Authors: Nick Nidzieko & Sean Gillen
Date: Jan/22/15
Origin: Horn Point Laboratory
Description: An IvP behavior meant to follow a current until it reaches
             the original waypoint specified in the bhv file. 

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

====================================================================*/

#ifndef BHV_FOLLOW_CURRENT_HEADER
#define BHV_FOLLOW_CURRENT_HEADER

#include <string>
#include "IvPBehavior.h"
#include "XYPoint.h"

class BHV_FollowCurrent : public IvPBehavior {
public:
  BHV_FollowCurrent(IvPDomain);
  ~BHV_FollowCurrent() {};
  
  bool         setParam(std::string, std::string);
  void         onIdleState();
  IvPFunction* onRunState();

protected:
  void         postViewPoint(bool viewable=true);
  IvPFunction* buildFunctionWithZAIC();
  bool parseCurrentVector(std::string, std::string&, std::string&);
protected: // Configuration parameters
  double       m_arrival_radius;
  double       m_desired_speed;
  XYPoint      m_nextpt;
  XYPoint      m_origpt;
  int          m_dir;
  bool         m_dir_set;
  
  std::string  m_invar;
protected: // State variables
  double   m_osx;
  double   m_osy;

};



#ifdef WIN32
	// Windows needs to explicitly specify functions to export from a dll
   #define IVP_EXPORT_FUNCTION __declspec(dllexport) 
#else
   #define IVP_EXPORT_FUNCTION
#endif

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_FollowCurrent(domain);}
}
#endif









