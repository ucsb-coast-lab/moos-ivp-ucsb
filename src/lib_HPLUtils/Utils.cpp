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
