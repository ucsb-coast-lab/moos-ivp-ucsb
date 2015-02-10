#include "NCData.h"
using namespace std;

//TODO : break this up into smaller functions (in a way that does NOT break everything horribly)

//---------------------------------------------------------------------
// Procedure: ReadNcFile
// notes : should read scalar data from the specified ncfile into a four dimensional array called val
//         will also read in lat and lon coordinates coresponding to specific indexes as well as get all 
//         time steps and their values
// NJN: 20141211: Changed indexing to typical numerical conventions: (x,y,z,t) = (i,j,k,n)

bool NCData::ReadNcFile(string ncFileName, string varName)
{
  cout << "NCData: opening file: " << ncFileName << endl;
  // these  dont actually do anything we need to concern ourselves with, but  the constructor requires them
  size_t* buffer = NULL;
  size_t size = 0;

  //open file
  NcFile File((const char*)ncFileName.c_str(), NcFile::ReadOnly , buffer , size , NcFile::Netcdf4); 
  if(!(File.is_valid())) //check if file is open
    {
      cout << "NCData: error reading file!" << endl;
      return false;
    } else cout <<"NCData: file opened successfully" << endl;

  //find specified variable
  NcVar* scalar_var = findNcVar(varName, &File);
  if(!scalar_var){
    cout << debugName << ":NCData: exiting!" << endl;
  }
  
  //get the size of the array our variable is stored in, edge lengths apply to lat/lon and time variables too
  long* edge;
  edge =  scalar_var->edges();

  time_vals = edge[0];
  cout << debugName << ":NCData: using "  << time_vals << " time values"  << endl;
  
  s_rho = edge[1];
  cout << debugName << ":NCData: using " << s_rho << " s values" << endl;
    
  eta_rho = edge[2];
  cout << debugName << ":NCData: using " << eta_rho << " eta values" << endl;
  
  xi_rho = edge[3];
  cout << debugName << ":NCData: using " << xi_rho << " xi_values" << endl;
  
   
  //find the remaining variables.
  NcVar* maskRho_var = findNcVar(maskRhoVarName, &File);
  NcVar* lat_var = findNcVar(latVarName , &File);
  NcVar* lon_var = findNcVar(lonVarName, &File);
  NcVar* time_var = findNcVar(timeVarName, &File);
  NcVar* s_var = findNcVar(sVarName, &File);
  NcVar* bathy_var = findNcVar(bathyVarName, &File);
  
  //make sure nothing came back false, exit if it did.
  if(!maskRho_var || !lat_var || !lon_var || !time_var || !s_var || !bathy_var){
    cout << debugName << ":NCData: exiting!"<< endl;
    return false;
  }
  
  vals = readNcVar4(scalar_var, edge);

  
  
  cout << "NCData: field for \"" << varName << "\" populated" << endl;
  
  //create maskRho array in local memory, read in maskRho values
  maskRho = new int* [eta_rho];
  for(int j = 0; j < eta_rho; j++){
    maskRho[j] = new int[xi_rho];
  }
  
  // read in row by row
  for(int j = 0; j < eta_rho; j++){
      maskRho_var->set_cur(j,0);
      maskRho_var->get(&maskRho[j][0], 1, xi_rho);
    }
  
  cout << "NCData: land mask field populated" << endl;
  
  //create lat array in local memory, read in lat values
  lat = new double* [eta_rho];
  for(int j = 0; j < eta_rho; j++)
    lat[j] = new double[xi_rho];
  
  //just like the main variable, lon/lat need to be read in row by row
  for(int j = 0; j < eta_rho; j++) 
    {
      lat_var->set_cur(j,0);
      lat_var->get(&lat[j][0], 1, xi_rho);
    }
  
  cout << "NCData: latitude field populated" << endl;
  
  
  
  //create lon array in local memory, read in lon values
  lon = new double* [eta_rho];
  for(int j = 0; j < eta_rho; j++)
    lon[j] = new double[xi_rho];
  
  //also needs to be read in row by row
  for(int j = 0; j < eta_rho; j++)
    {
      lon_var->set_cur(j,0);
      lon_var->get(&lon[j][0], 1 , xi_rho);
      }       
  cout << "NCData: longitude field populated" << endl;
  
  //read in time values 
  time = new double [time_vals]; 
  time_var->get(&time[0], time_vals);
  cout << "NCData: time field populated" << endl;
  
  // Adjust from ROMS ocean_time epoch (secs since 2009/1/1) 
  // to unix epoch (seconds since 1970/1/1) used by MOOS
  double utc2009_epoch = 1230768000;
  for(int n = 0; n < time_vals; n++){
    time[n] += utc2009_epoch;
  }   
  
  //read in s_rho values
  s_values = new double[s_rho];
  s_var->get(&s_values[0], s_rho);
  cout << "NCData: depth field populated" << endl;
  
  //create bathy array in local memory, read in lat values
  bathy  = new double* [eta_rho];
  for(int j = 0; j < eta_rho; j++)
    bathy[j] = new double[xi_rho];
  
  //needs to be read in row by row
  for(int j = 0; j < eta_rho; j++) 
    {
      bathy_var->set_cur(j,0);
      bathy_var->get(&bathy[j][0], 1, xi_rho);
    }
  
  cout << "NCData: bathymetry field populated" << endl;    
  return true;
  
 }
//---------------------------------------------------------------------
//attempts to find the specified variable and returns if it's able


NcVar* NCData::findNcVar(string varName, NcFile *pFile)
{
  NcVar* var;
  
  var = pFile->get_var((const char*) varName.c_str());
  if(!(var->is_valid())){ //check if we found the variable or not
    cout << debugName << ":NCData: error reading variable : " << varName << endl;
    return NULL;
  }else cout << debugName << "NCData: variable : " << varName << "found" << endl;

  return var;
}

//--------------------------------------------------------------------
//Allocates a 4 dimensional array in local memory and reads in values to it

double**** NCData::readNcVar4(NcVar* var, long  size[4])
{
  
  //create a value array in local memory, read in values
  double**** vals = new double***[size[0]];
  for(int n = 0; n < size[0]; n++)  //dynamic memory must be initialized row by row
    {
      vals[n] = new double**[size[1]];
      for(int k = 0; k < size[1]; k++)
	{
	  vals[n][k] = new double*[size[2]];
	  for(int j = 0; j < size[2]; j++)
	    {
	      vals[n][k][j] = new double[size[3]];
	    }
	}
    }
  
  //reads in the variable 
  //the netCDF get method can only read in values to contigous blocks of memory, meaning we have to get values
  //row by row.
  for(int n = 0; n < size[0]; n++)
    {
      for(int k = 0; k < size[1]; k++)
	{
	  for(int j = 0; j < size[2]; j++)
	    {
	      var->set_cur(n,k,j,0);
	      var->get(&vals[n][k][j][0], 1, 1, 1, size[3]);
	      
	    }
	}
    }

  return vals;
}



