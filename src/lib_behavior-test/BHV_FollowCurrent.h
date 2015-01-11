#ifndef BHV_SIMPLE_WAYPOINT_HEADER
#define BHV_SIMPLE_WAYPOINT_HEADER

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
  std::string  m_ipf_type;

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









