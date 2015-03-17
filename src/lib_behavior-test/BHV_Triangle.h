/*==========================================================================
File: BHV_Triangle.h  
Authors: Nick Nidzieko & Sean Gillen
Date: Jan/22/15
Origin: Horn Point Laboratory
Description: 

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

==========================================================================*/

 
#ifndef BHV_CONSTANT_ALTITUDE_HEADER
#define BHV_CONSTANT_ALTITUDE_HEADER

#include "IvPBehavior.h"
#include "ConditionalParam.h"

class BHV_ConstantAltitude : public IvPBehavior {
public:
  BHV_ConstantAltitude(IvPDomain);
  ~BHV_ConstantAltitude() {};
  
  void         onIdleState() {updateInfoIn();};
  IvPFunction* onRunState();
  bool         setParam(std::string, std::string);

 protected:
  bool         updateInfoIn();

 protected: // Configuration variables
  double      m_desired_altitude;
  double      m_peakwidth;
  double      m_basewidth;
  double      m_summitdelta;
  std::string m_depth_mismatch_var;
  double      m_floordepth;

 protected: // State variables
  double      m_osd;
};
#endif


