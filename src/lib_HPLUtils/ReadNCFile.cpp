/*===================================================================
File: NCData.h
Authors: Nick Nidzieko & Sean Gillen
Date: Jan-23-2015
Origin: Horn Point Laboratory
Description: Reads NetCDF data from a specified file, put into a 
             separate file because it got a bit long. 

Copyright 2015 Nick Nidzieko, Sean Gillen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===================================================================*/


#include "NCData.h"
using namespace std;

//TODO rename variables to a consistent naming convention 


//---------------------------------------------------------------------
// Procedure: ReadNcFile
// notes : should read scalar data from the specified ncfile into a four dimensional array called val
//         will also read in lat and lon coordinates coresponding to specific indexes as well as get all 
//         time steps and their values
// NJN: 20141211: Changed indexing to typical numerical conventions: (x,y,z,t) = (i,j,k,n)

bool NCData::ReadNcFile(string ncFileName, string varName, string vecVarName[3])
{
  cout << debug_name << ": NCData: opening file: " << ncFileName << endl;
  // these  don't actually do anything we need to concern ourselves with, but  the constructor requires them
  size_t* buffer = NULL;
  size_t size = 0;

  //open file
  NcFile File((const char*)ncFileName.c_str(), NcFile::ReadOnly , buffer , size , NcFile::Netcdf4); 
  if(!(File.is_valid())) //check if file is open
    {
      cout << debug_name << ": NCData: error reading file!" << endl;
      return false;
    } else cout << debug_name << ": NCData: file opened successfully" << endl;
  
  
  if(!readScalarVar(varName , &File)){
    cout << "error reading scalar variable , exiting" << endl;
    return false;
  }
  
  if(!readVectorVar(vecVarName, &File)){
    cout << "error reading vector variable , exiting" << endl;
    return false;
  }
  
  
  // Adjust from ROMS ocean_time epoch (secs since 2009/1/1) 
  // to unix epoch (seconds since 1970/1/1) used by MOOS
  double utc2009_epoch = 1230768000;
  for(int n = 0; n < time_vals; n++){
    time[n] += utc2009_epoch;
  }

  return true;
 }

//---------------------------------------------------------------------
//reads the scalar variable into local memory, this has the added effect of establishing
//time and depth values.

bool NCData::readScalarVar(string var_name , NcFile *pFile)
{
  //find specified variable
  NcVar* scalar_var = findNcVar(var_name, pFile);
  if(!scalar_var){
    cout << "could not fine main scalar variable! exiting!" << endl;
    cout << debug_name << ":NCData: exiting!" << endl;
    return false;
  }
  
  //get the size of the array our variable is stored in, edge lengths apply to lat/lon and time variables too
  
  long* edge;
  edge = scalar_var->edges();
  
  time_vals = edge[0];
  cout << debug_name << ": NCData: using " << time_vals << " time values" << endl;
  
  s_rho = edge[1];
  cout << debug_name << ": NCData: using " << s_rho << " s values" << endl;
  
  eta_rho = edge[2];
  cout << debug_name << ": NCData: using " << eta_rho << " eta values" << endl;
  
  xi_rho = edge[3];
  cout << debug_name << ": NCData: using " << xi_rho << " xi_values" << endl;
  
  //find the remaining variables.
  NcVar* mask_rho_var = findNcVar(mask_rho_var_name, pFile);
  NcVar* lat_var = findNcVar(lat_var_name , pFile);
  NcVar* lon_var = findNcVar(lon_var_name, pFile);
  NcVar* time_var = findNcVar(time_var_name, pFile);
  NcVar* s_var = findNcVar(s_var_name, pFile);
  NcVar* bathy_var = findNcVar(bathy_var_name, pFile);
  //make sure nothing came back false, exit if it did.
  
  if(!mask_rho_var || !lat_var || !lon_var || !time_var || !s_var || !bathy_var){
    cout << debug_name << ": NCData: Variable not found! exiting!"<< endl; 
    return false;
  }
  
  vals = readNcVar4(scalar_var, edge);
  cout << debug_name << ": NCData: field for \"" << var_name << "\" populated" << endl;
  
  mask_rho = readNcVar2(mask_rho_var, edge);
  cout << debug_name << ": NCData: land mask field populated" << endl;
  
  lat = readNcVar2(lat_var, edge);
  cout << debug_name << ": NCData: latitude field populated" << endl;
  
  lon = readNcVar2(lon_var, edge);
  cout << debug_name << ": NCData: longitude field populated" << endl;
  
  bathy = readNcVar2(bathy_var, edge);
  cout << debug_name << ": NCData: bathymetry field populated" << endl;

   //read in time values 
  time = new double [time_vals]; 
  time_var->get(&time[0], time_vals);
  cout << debug_name << ": NCData: time field populated" << endl;

  
  //read in s_rho values
  s_values = new double[s_rho];
  s_var->get(&s_values[0], s_rho);
  cout << debug_name << ": NCData: depth field populated" << endl;

  cout << debug_name << ": NCData: scalar variable read in successfully " << endl << endl;
  
  return true;
}


//---------------------------------------------------------------------
//reads in a vector valued variable, will override s_rho and time_vals , so make sure your vector variables
//and your scalar variables are all using the same grid

bool NCData::readVectorVar(string vec_var_name[3], NcFile *p_file)
{
   NcVar* u_var = findNcVar(vec_var_name[0], p_file);
   NcVar* v_var = findNcVar(vec_var_name[1] , p_file);
   NcVar* w_var = findNcVar(vec_var_name[2] , p_file);
  
   NcVar* u_lat_var = findNcVar(lat_u_var_name , p_file);
   NcVar* u_lon_var = findNcVar(lon_u_var_name , p_file); 
   
   NcVar* v_lat_var = findNcVar(lat_v_var_name ,p_file);
   NcVar* v_lon_var = findNcVar(lon_v_var_name ,p_file);

   NcVar* angle_var = findNcVar(angle_var_name , p_file);

   if(!u_var || !v_var || !w_var || !u_lat_var || !u_lon_var || !v_lat_var || !v_lon_var){
     cout << "exiting!" << endl;
     return false;
   }
   
   //w lat/lon exist on rho points, which we already have from the scalar variable
   

   long* edge_v = v_var->edges();
   long* edge_u = u_var->edges();

   //this is the same edge as readScalarVar, we need it because w used rho coordinates, maybe should have just made edge a member, oh well
   long* edge_w = w_var->edges(); 


   eta_v = edge_v[2];
   cout << debug_name << ": NCData: using " << eta_v << " eta_v values" << endl;
   xi_v = edge_v[3];
   cout << debug_name << ": NCData: using " << xi_v << " xi_v values" << endl;
   

   eta_u = edge_u[2];
   cout << debug_name << ": NCData: using " << eta_u << " eta_u values" << endl;
   xi_u = edge_v[3];
   cout << debug_name << ": NCData: using " << xi_u << " eta_v values" << endl;

   //read in variables
   u_vals = readNcVar4(u_var , edge_u);
   v_vals = readNcVar4(v_var , edge_v); 
   w_vals = readNcVar4(w_var , edge_w);
   
   //read in lat/lon
   double** u_lat = readNcVar2(u_lat_var , edge_u);
   double** u_lon = readNcVar2(u_lon_var , edge_u);
   double** v_lat = readNcVar2(v_lat_var , edge_v);
   double** v_lon = readNcVar2(v_lon_var , edge_v);
   
   //convert lat/lon to meters_e / meters_n
   ConvertToMeters(&meters_n, &meters_e , lat, lon , eta_rho, xi_rho);
   ConvertToMeters(&u_meters_n , &u_meters_e , u_lat , u_lon , eta_u , xi_u);
   ConvertToMeters(&v_meters_n , &v_meters_e , v_lat , v_lon , eta_v , xi_v);

   //angle is the angle between east and xi
   angle = readNcVar2(angle_var , edge_w);

   double ****u_vals_east;
   double ****u_vals_north;
   
   double ****v_vals_east;
   double ****v_vals_north;


   convertToEastNorth(&u_vals_east, &u_vals_north, edge_u, u_vals, angle);
   convertToEastNorth(&v_vals_east, &v_vals_north, edge_v, v_vals, angle);

   e_values = combineVector(u_vals_east, v_vals_east, (int*)edge_u , (int*)edge_v);
   w_values = combineVector(u_vals_north, v_vals_east, (int*)edge_v, (int*)edge_u);

   for(int i = 0; i < 4; i++){
     vec_size[i] = edge_u[i] + edge_v[i];
   }

   //free all the dynamic local variables 
   freeDouble4DArray(u_vals_east, edge_u);
   freeDouble4DArray(u_vals_north , edge_u);
   freeDouble4DArray(v_vals_east, edge_v);
   freeDouble4DArray(v_vals_north, edge_v);
   
   freeDouble2DArray(u_lat, edge_u[2]);
   freeDouble2DArray(u_lon, edge_u[2]);
   freeDouble2DArray(v_lat, edge_v[2]);
   freeDouble2DArray(v_lon, edge_v[2]);
   
  
   return true;
}
   

//---------------------------------------------------------------------
//attempts to find the specified variable and returns if it's able


NcVar* NCData::findNcVar(string var_name, NcFile *p_file)
{
  NcVar* var;
  
  var = p_file->get_var((const char*) var_name.c_str());
  if(!(var->is_valid())){ //check if we found the variable or not
    cout << debug_name << ": NCData: error reading variable : " << var_name << endl;
    return NULL;
  }else cout << debug_name << ": NCData: variable : " << var_name << " found" << endl;

  return var;
}

//--------------------------------------------------------------------
//Allocates a 4 dimensional array in local memory and reads in values to it

double**** NCData::readNcVar4(NcVar* var, long  size[4])
{  
  //create a value array in local memory, read in values
  double**** values = new double***[size[0]];
  for(int n = 0; n < size[0]; n++)  //dynamic memory must be initialized row by row
    {
      values[n] = new double**[size[1]];
      for(int k = 0; k < size[1]; k++)
	{
	  values[n][k] = new double*[size[2]];
	  for(int j = 0; j < size[2]; j++)
	    {
	      values[n][k][j] = new double[size[3]];
	    }
	}
    }
  
  //reads in the variable 
  //the netCDF get method can only read in values to contiguous blocks of memory, meaning we have to get values
  //row by row.
  for(int n = 0; n < size[0]; n++){
      for(int k = 0; k < size[1]; k++){
	  for(int j = 0; j < size[2]; j++){
	      var->set_cur(n,k,j,0);
	      var->get(&values[n][k][j][0], 1, 1, 1, size[3]);
	    }
	}
    }

  return values;
}

//---------------------------------------------------------------------
//allocates a 2 dimensional array and reads NC data into it, assumes
//you want xi and eta as your grid boundaries 
double** NCData::readNcVar2(NcVar* var , long size[4])
{
  double **vals = new double* [size[2]];
  for(int j = 0; j < size[2]; j++){
    vals[j] = new double[size[3]];
  }
  //needs to be read in row by row
  for(int j = 0; j < size[2]; j++){
    var->set_cur(j,0);
    var->get(&vals[j][0], 1, size[3]);
  }
  return vals;
}



//---------------------------------------------------------------------
//convertToEastWest
//Converts xi / eta into east and west components using simple trig

bool NCData::convertToEastNorth(double *****pvals_east , double *****pvals_north, long size[4], double ****vals, double **angle)
{
  
  //create a value array in local memory, read in values
  (*pvals_east) = new double***[size[0]];
  (*pvals_north) = new double***[size[0]];
  for(int n = 0; n < size[0]; n++)  //dynamic memory must be initialized row by row
    {
      (*pvals_east)[n] = new double**[size[1]];
      (*pvals_north)[n] = new double**[size[1]];
      for(int k = 0; k < size[1]; k++)
	{
	  (*pvals_east)[n][k] = new double*[size[2]];
	  (*pvals_north)[n][k] = new double*[size[2]];
	  for(int j = 0; j < size[2]; j++)
	    {
	      (*pvals_east)[n][k][j] = new double[size[3]];
	      (*pvals_north)[n][k][j] = new double[size[3]];
	    }
	}
    }

  for(int n = 0 ;  n < size[0]; n++){
    for(int k = 0;  k < size[1];k++){
      for(int j = 0; j < size[2]; j++){
	for(int i = 0; i < size[3]; i++){
	  (*pvals_east)[n][k][j][i] = vals[n][k][j][i] * cos(angle[j][i]);
	  (*pvals_north)[n][k][j][i] = vals[n][k][j][i] * sin(angle[j][i]);
	}
      }
    }
  }

  return true;
  
}

//---------------------------------------------------------------------
//combineVector
//allocates space and fills the vector variable by combining all the east components (or all the north)
//into one large array, of course the function is more general than that (need not be east or north really)
//but that's what we're using it for..

double**** NCData::combineVector(double ****vals_1, double ****vals_2, int size_1[4], int size_2[4])
{
 
  //create a value array in local memory, read in values
  double**** values = new double***[size_1[0]+size_2[0]];
  for(int n = 0; n < (size_1[0] + size_2[0]); n++)  //dynamic memory must be initialized row by row
    {
      values[n] = new double**[size_1[1] + size_2[1]];
      for(int k = 0; k < (size_1[1] + size_2[1]); k++)
	{
	  values[n][k] = new double*[size_1[2] + size_2[2]];
	  for(int j = 0; j < (size_1[2] + size_2[2]); j++)
	    {
	      values[n][k][j] = new double[size_1[3] + size_2[3]];
	    }
	}
    }
  
  //reads in the variable 
  //the netCDF get method can only read in values to contiguous blocks of memory, meaning we have to get values
  //row by row.
  
  for(int n = 0; n < size_1[0]; n++){
      for(int k = 0; k < size_1[1]; k++){
	  for(int j = 0; j < size_1[2]; j++){
	      for(int i = 0; i < size_1[3]; i++){
		  vals[n][k][j][i] = vals_1[n][k][j][i];
		}
	    }
	}
    }
  
  for(int n = size_1[0]; n < (size_1[0] + size_2[0]); n++){
      for(int k = size_1[1]; k < (size_1[1]+ size_2[1]); k++){
	  for(int j = size_1[2]; j < (size_1[2] + size_2[2]); j++){
	      for(int i = size_1[3]; i < (size_1[3] + size_2[3]); i++){ 
		  vals[n][k][j][i] = vals_1[n][k][j][i];
		}
	    }
	}
    }

  return values;
  
}
  





