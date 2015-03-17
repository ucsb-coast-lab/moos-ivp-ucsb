#include "Utils.h"
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
      weights[i] = .000000001; // a bit hacky, but it gets the job doen
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


//---------------------------------------------------------------------
//same idea as 4DArray but with 2 dimensions, assumes you want the 
void FreeDouble2DArray(double **array, long size_1)
{
  for(int i = 0; i < size_1; i++){
    delete array[i];       
  }
  
}


//---------------------------------------------------------------------
//Free4DArray
//frees a 4 dimensional array made of raw pointers
void FreeDouble4DArray(double ****array, long size[4])
{
  for(int i = 0; i < size[0]; i++){
    for(int j = 0; j < size[1]; j++){
      for(int k = 0; k < size[2]; k++){
	  delete array[i][j][k];
      }
    }
  }
  
}


  
