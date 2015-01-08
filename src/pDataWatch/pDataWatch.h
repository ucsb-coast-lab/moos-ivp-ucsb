//pDataWatch looks constantly for data publishes under "SCALAR_VALUE" althought it can later be configured to look 
//for whatever data is neccessary. it then takes the scalar data and after it gets six vlaues starts to take a
//running average and checking to aee if it is above or below a threshold determined by the "THRESHOLD" paramter.
//if it's above the threshold it publishes a variable s_plus as true and otherwise publishes it as false.


#ifndef USC_MOOSAPP_HEADER
#define USC_MOOSAPP_HEADER
#include <iostream>
#include <fstream>
#include <string>
#include "MOOS/libMOOS/MOOSLib.h"

using namespace std;


class DW_MOOSApp : public CMOOSApp
{
 public:
  DW_MOOSApp();
  // DW_MOOSApp(int);
  virtual ~DW_MOOSApp() {};


  bool Iterate();
  bool OnConnectToServer();
  bool OnDisconnectFromServer();
  bool OnStartUp();
  bool OnNewMail(MOOSMSG_LIST &NewMail);


 protected:


  double RunningAverage(double);
  bool GradientTrack();
  bool GradientTrackPlusMinus();
  void RegisterVariables(); 
 
 protected:

  double hi_thres;
  double lo_thres;
  double PhaseShift; 
  int points;
  std::string invar;
  std::string outvar1;
  std::string outvar2;
  int tte; // Time to Exceed
  std::string reportname;  // for collecting variables
  ofstream fout;
 

 protected:  //state variables
  double scalar;
  int count; 
  double* sal;
  double avg;
  double old_depth; 
  double vehicle_depth;
  int   SGOAL;
  bool   ONPAPA;
  


};
#endif
