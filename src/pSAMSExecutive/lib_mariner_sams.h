// This is start of the header guard.  <NAME_OF_FILE>_H can be any unique name.  By convention, we use the name of the header file.
// It's pretty common to have the header guard be all in caps with a _H suffix

#ifndef LIB_MARINER_SAMS_H
#define LIB_MARINER_SAMS_H

//double const PI = 3.14159;
#include <string>

void test_function();

struct Coordinate {
    double x,y;
    bool searched;
};

// Using a coordinate and an angle, complete an angle tranform on that coordinate
struct Coordinate transform (struct Coordinate coor, double theta);
// Returns a printable string of an (x,y) coordinate for troubleshooting
std::string coor_to_string(Coordinate coor);

// Checking that a point lies inside the bounds of a four-pointed polygon 'bounding box'
// Will not compile if try to pass an array of length != 4 Coordinates
bool check_bounds(Coordinate box[4], Coordinate coor);
// Returns a second array of an error boundary to provided box. Using check_bounds()
// on both 'box' and the resulting error boundary allows for extra behavior considerations
// for changing vehicle behavior before it fully exits the acceptable zone of operation
struct Coordinate * create_error_boundary(Coordinate box[4],double dx, double dy);

struct RunLine {
  Coordinate start, end;
};

// Returns a double value of a runline's direction in the global reference frame, in degrees
double get_theta(struct RunLine line);
// Returns a double value of a runline's length, in meters
double get_length(struct RunLine line);

int check_runline_bounds(RunLine line, Coordinate box[4], Coordinate box_error_boundary[4]);


#endif // End of the header guard
