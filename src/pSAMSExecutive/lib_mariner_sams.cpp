// Note that we have to do #include "<header-file-name>.h", or else it won't compile

#include <iostream>
#include <cmath>
#include <string>
#include "lib_mariner_sams.h"

using namespace std;

double const PI = 3.14159;

void test_function() {
  cout << "From 'lib_mariner_sams' " << endl;
}

// Coordinate Functions

struct Coordinate transform (struct Coordinate coor, double theta) {
    Coordinate coor_1 = coor;
    Coordinate coor_2;
    coor_2.x = coor_1.x * cos(theta * PI / 180) - coor_1.y * sin(theta * PI / 180);
    coor_2.y = coor_1.x * sin(theta * PI / 180) + coor_1.y * cos(theta * PI / 180);
    return coor_2;
}

std::string coor_to_string(struct Coordinate coor) {
  std::string coor_str = "("+to_string(coor.x)+", "+to_string(coor.y)+","+to_string(coor.searched)+")";
  return coor_str;
}

// Checking that a point lies inside the bounds of a four-pointed polygon 'bounding box'
//
// It's possible to create a box with more than four points, but if we try to
// pass that box to check_bounds(), the compiler will report it as an error
bool check_bounds(Coordinate box[4], Coordinate coor) {

  // Creating a new coordinate for each corner point
  Coordinate point_i, point_ii, point_iii, point_iv;

  // Start by finding the centroid of all box points, and creating a Coordinate at that point
  double sum_x = 0;
  double sum_y = 0;
  for( int j = 0; j < 4; j++ ) {
    sum_x = sum_x + box[j].x;
    sum_y = sum_y + box[j].y;
  }
  Coordinate centroid = {sum_x/4, sum_y/4};

  // Sorting values in box to quadrants based on x,y position relative to centroid
  // By definition, centroid can't be equal to any of the
  for (int k = 0; k < 4; k++) {
    if ( (box[k].x > centroid.x ) && (box[k].y > centroid.y) ) {
      point_i = box[k];
      //cout << "Box[" << k <<"] is point_i" << endl;
    }
    else if ( (box[k].x < centroid.x ) && (box[k].y > centroid.y) ) {
      point_ii = box[k];
      //cout << "Box[" << k <<"] is point_ii" << endl;
    }
    else if ( (box[k].x < centroid.x ) && (box[k].y < centroid.y) ) {
      point_iii = box[k];
      //cout << "Box[" << k <<"] is point_iii" << endl;
    }
    else if ( (box[k].x > centroid.x ) && (box[k].y < centroid.y) ) {
      point_iv = box[k];
      //cout << "Box[" << k <<"] is point_iv" << endl;
    }
  }

  // Checking to see if the coordinate 'coor' is located in the bounding box
  // If the coordinate is on the correct side of the lines formed by points I-II,
  // II-III, III-IV, and IV-I, then it is located within the box by definition
  // Therefore, we create a boolean for each of those conditions and check to make
  // sure that all of them are considered correct

    // Is the point left of the I-IV line?
    double i_iv_slope = (point_i.y - point_iv.y)/(point_i.x - point_iv.x);
    double height_i_iv = coor.y - point_iv.y;
    double x_intercept_i_iv = point_iv.x + height_i_iv/i_iv_slope;
    //cout << "i_iv x-int = " << x_intercept_i_iv << endl;
    bool left_of_i_iv = (coor.x <= x_intercept_i_iv);
    //cout << "left_of_i_iv = " << left_of_i_iv << endl;

    // Is the point right of II-III line?
    double ii_iii_slope = (point_ii.y - point_iii.y)/(point_ii.x - point_iii.x);
    double height_ii_iii = coor.y - point_iii.y;
    double x_intercept_ii_iii = point_iii.x + height_ii_iii/ii_iii_slope;
    //cout << "ii_iii x-int = " << x_intercept_ii_iii << endl;
    bool right_of_ii_iii = (coor.x >= x_intercept_ii_iii);
    //cout << "right_of_ii_iii = " << right_of_ii_iii << endl;

    // Is the point below the I-II line?
    double i_ii_slope = (point_i.y - point_ii.y)/(point_i.x - point_ii.x);
    double length_i_ii = coor.x - point_ii.x;
    double y_intercept_i_ii = point_ii.y + length_i_ii*i_ii_slope;
    //cout << "i_ii y-int = " << y_intercept_i_ii << endl;
    bool below_i_ii = (coor.y <= y_intercept_i_ii);
    //cout << "below_i_ii = " << below_i_ii << endl;

    // Is the point above the III-IV line?
    double iii_iv_slope = (point_iii.y - point_iv.y)/(point_iii.x - point_iv.x);
    double length_iii_iv = coor.x - point_iii.x;
    double y_intercept_iii_iv = point_iii.y + length_iii_iv*iii_iv_slope;
    //cout << "iii_iv y-int = " << y_intercept_iii_iv << endl;
    bool above_iii_iv = (coor.y >= y_intercept_iii_iv);
    //cout << "above_iii_iv = " << above_iii_iv << endl;

    if ( (left_of_i_iv == true ) && (right_of_ii_iii == true) && (below_i_ii == true) && (above_iii_iv == true) ) {
      return true;
    }
    else {
      return false;
    }

}

// Creates a second error boundary outside of the original, using a set of coordinates based
// on a given +/- dx,dy provided to the function. Please note that this function returns a
// pointer to a static array, not just an array.
struct Coordinate * create_error_boundary(Coordinate box[4],double dx, double dy) {
  // Need to declare a static here, because can't return an array, so a pointer is required
  // If we don't use 'static', the array won't be accessible to main(), and we'll end up with a segfault
  static Coordinate box_error_boundary[4];

  // TO_DO: Re-used the same sorting code from check_bounds(); this could probably be written into a
  // separate function to improve code reuse and readability, but should be considered lower priority
  // for the moment.

  // Creating a new coordinate for each corner point
  Coordinate point_i, point_ii, point_iii, point_iv;

  // Start by finding the centroid of all box points, and creating a Coordinate at that point
  double sum_x = 0;
  double sum_y = 0;
  for( int j = 0; j < 4; j++ ) {
    sum_x = sum_x + box[j].x;
    sum_y = sum_y + box[j].y;
  }
  Coordinate centroid = {sum_x/4, sum_y/4};

  // Sorting values in box to quadrants based on x,y position relative to centroid
  // By definition, centroid can't be equal to any of the
  for (int k = 0; k < 4; k++) {
    if ( (box[k].x > centroid.x ) && (box[k].y > centroid.y) ) {
      point_i = box[k];
      //cout << "Box[" << k <<"] is point_i" << endl;
    }
    else if ( (box[k].x < centroid.x ) && (box[k].y > centroid.y) ) {
      point_ii = box[k];
      //cout << "Box[" << k <<"] is point_ii" << endl;
    }
    else if ( (box[k].x < centroid.x ) && (box[k].y < centroid.y) ) {
      point_iii = box[k];
      //cout << "Box[" << k <<"] is point_iii" << endl;
    }
    else if ( (box[k].x > centroid.x ) && (box[k].y < centroid.y) ) {
      point_iv = box[k];
      //cout << "Box[" << k <<"] is point_iv" << endl;
    }
  }

  box_error_boundary[0] = {point_i.x + dx, point_i.y + dy};
  box_error_boundary[1] = {point_ii.x - dx, point_ii.y + dy};
  box_error_boundary[2] = {point_iii.x - dx, point_iii.y - dy};
  box_error_boundary[3] = {point_iv.x + dx, point_iv.y - dy};

  return box_error_boundary;
}

// RunLine Functions

// TO_DO: Would it make sense to use C++11 std::tuple instead of pointers to static arrays
// or defining a new type for returning the (l,theta) values?

// Returns the angle of the RunLine, from Start to End in the global reference frame
double get_theta(struct RunLine line) {
  double dy = line.end.y - line.start.y;
  double dx = line.end.x - line.start.x;
  double angle = atan(dy/dx) * 180 / PI;
  //cout << "(dx, dy) = (" << dx << "," << dy << ")" << endl;

  // Need to adjust angle calclation based on the quadrant that the line leans into
  if ( (dy < 0) && (dx > 0)) {
    angle = angle + 270;
    //cout << "Line ran into quadrant IV" << endl;
  }
  else if ( (dy < 0) && (dx < 0) ) {
    angle = angle + 180;
    //cout << "Line ran into quadrant III" << endl;
  }
  else if ( (dy > 0) && (dx < 0) ) {
    angle = angle + 90;
    //cout << "Line ran into quadrant II" << endl;
  }
  else {
    //cout << "Make no corrections" << endl;
  }

  return angle;

}

// Returns the length of the RunLine as a scalar value
double get_length(struct RunLine line) {
  return pow(pow((line.end.y - line.start.y),2.0) + pow((line.end.x - line.start.x),2.0),0.5);
}

// Checks if the RunLine is inside both the primary boundary as well as the error box_error_boundary
// If the line is entirely within the primary boundary, returns a 1
// If either the start or end of the line is outside of the error boundary, returns  0
// If the line is within the error boundary, but exceeds the primary boundary, returns 2
// This allows for boolean checkers with (if check_runline_bounds(...) != true ), although
// it might be a good idea to use the actual return numbers instead
int check_runline_bounds(RunLine line, Coordinate box[4], Coordinate box_error_boundary[4]) {
  if ( (check_bounds(box, line.start) == true) && check_bounds(box, line.end) == true) {
    //cout << "Run line is entirely within the bounding box" << endl;
    return 1;
  }
  else if ( (check_bounds(box_error_boundary,line.start) == true) && (check_bounds(box_error_boundary,line.end) == true ) ) {
    //cout << "Run line goes outside the bounding box, but inside the error boundary" << endl;
    return 2;
  }
  else {
    //cout << "Run line passes outside of the error boundary" << endl;
    return 0;
  }
}
