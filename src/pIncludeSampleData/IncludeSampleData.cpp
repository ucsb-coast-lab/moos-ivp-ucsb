/************************************************************/
/*    NAME: cmoran                                               */
/*    ORGN: UCSB Coastal Oceanography and Autonomous Systems Lab */
/*    FILE: IncludeSampleData.cpp                                */
/*    DATE: 13 February 2019                                     */
/************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <new>
#include <algorithm>
#include <typeinfo> // for debugging
#include <vector>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <cmath>
#include <random>

#include "MBUtils.h"
#include "IncludeSampleData.h"
// Header file for the Rust image processing function library used for the sample data -> .csv file conversion 
#include "image_to_csv_rs.h"

using namespace std;

//---------------------------------------------------------
// Constructor

IncludeSampleData::IncludeSampleData()
{
		// m_nav_x,m_nav_y, and m_nav_heading are the real-time x,y, and vehicle-reference frame heading
		// data read in from MOOSDB
    m_nav_x = 0.0;
    m_nav_y = 0.0;
    m_nav_heading = 0.0;

		// Is the iterator used to mark which ping/signal vector from the data file is currently being accessed
    m_iterations = 0;
		// TO_DO: Think about making filenames specifiable in a config file
		// Specifies the image file (of .csv format) and the output file that the data will be written to, as a check
		// that none of our operations have messed up that file during processing
		m_input_filename = "csv_image_import.csv";
		m_output_filename = "output.csv";
		// m_row/colCount will be assigned the number of rows/columns in the input image file during OnStartUp()
		m_colCount = 0;
		m_rowCount = 0;
		// Reads the MODE from MOOSDB, which can be used to dictate certain conditional behaviors
		m_mode = "";

}

//---------------------------------------------------------
// Procedure: OnNewMail

bool IncludeSampleData::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
  	CMOOSMsg &msg = *p;
    string key   = msg.GetKey();
    double dval  = msg.GetDouble();
		string sval  = msg.GetString();

         if(key=="NAV_X") {
             m_nav_x = dval;
             //cout << "NAV_X = " << m_nav_x << endl;
         }

         if(key=="NAV_Y") {
             m_nav_y = dval;
						 //cout << "NAV_Y = " << m_nav_y << endl;
         }

         if(key=="NAV_HEADING") {
           	m_nav_heading = dval;
  					//cout << "NAV_HEADING = " << m_nav_heading << endl;
         }

				 if(key=="MODE") {
						m_mode = sval;
				 }

     }
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool IncludeSampleData::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);

   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void IncludeSampleData::RegisterVariables()
{
  if(m_nav_x_received != "")
       m_Comms.Register(m_nav_x_received, 0);
  if(m_nav_y_received != "")
       m_Comms.Register(m_nav_y_received, 0);
  if(m_nav_heading_received != "")
       m_Comms.Register(m_nav_heading_received, 0);
	if(m_mode_received != "")
	 		 m_Comms.Register(m_mode_received, 0);
}


//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool IncludeSampleData::Iterate()
{
	cout << "m_mode = " << m_mode << endl;
	ofstream output_file_position ("position_output.csv",ios::app);
	// TO_DO: Once file size is known, set these indices as a fraction of mColCount to prevent breaking
	// max_index variables are supposed to the 'best guess' as a initialized index for max returned value, to make sure we never
	// have a null or completely wrong value in this field. See above TO_DO for making this a little more dynamic
	int max_index = 82; // This needs to go here into order to be available during lifetimes
										  // Estimated this value, since we don't want it to ever be unassigned
											// TO_DO: Make this more resilient
	int max_index_padded = 82;
	// TO_DO: Feels like this could be the place for a switch statement or other FSM pattern

	if (m_mode == "ACTIVE:SURVEYING:LINE_FOLLOWING") {
			// Open the input and output files, output file such that values append
			ifstream input_file (m_input_filename);
			ofstream output_file (m_output_filename,ios::app);
			ofstream output_file_padded ("padded_output.csv",ios::app);
			ofstream output_file_maximums ("maximums_output.csv",ios::app);

			// Opens array
			// TO_DO: Is this actually necessary? Or should be just go straight from vect(i), declaring here instead?
			// TO_DO: Might be adding unnecessary computation by doing this conversion ^. Check back after ping analysis methodology is finalized
			int arr[m_colCount * m_rowCount] = {};
			std::vector<int> vect;
			if (input_file.is_open() && output_file.is_open() && m_iterations < m_rowCount) {

				string line;
				char delim = ',';
				string delimiter = ",";

				unsigned int row_num = 0;
				unsigned int col_num = 0;
				while (std::getline(input_file, line))
				{
					if (row_num == m_iterations) {
						//cout << line << endl;
						std::stringstream ss(line);
						//std::vector<int> vect;

						int i;
						while (ss >> i)
						{
								// Writes non-delimiter values to the signal vector
								vect.push_back(i);
								if (ss.peek() == ',') {
										ss.ignore();
								}
						}

						for (i=0; i< vect.size(); i++) {
								//arr[col_num + row_num*m_colCount] = vect.at(i); // If was maintaining entire array instead of one line at a time
								arr[col_num] = vect.at(i);
								col_num++;
						}
					}
					// TO_DO: Might be unnecessarily iterating through the entire file on each run-through, possible to simplify?
					row_num++;
					col_num = 0;
				}

				// NOTE: Here's where any operations on the line array should occur; could end up publishing resulting value
				// Finds the maximum value of the ping for standard arr[]
				int max = arr[0]; // First assignment of the max value and index
				max_index = 0;
				int j = 0;
				int bottom_edge = 150; // This is a value at which the bottom return of the acoustic image appears
				for (j = 0; j < bottom_edge; j++) {
					// If the next value is larger than the existing maximum, set that and the values index to 'max' and record its index
					if (arr[j] > max) {
						max = arr[j];
						max_index = j;
					}
				}

				// Writes maximum values to file
				output_file_maximums << max_index << "," << max << endl;

				// Intermediate step of padding output and fluctuating arr[]'s locations within it
				// Not actually going to perform operations on arr[]; prefer to leave it alone, at least while we're still in simulation
				double pad_percentage = 0.2; // Percentage of the total number of columns we're going to pad the image array with, based on the original image's size
			  int offset = round(pad_percentage * m_colCount / 2);
			  int padded_size = m_colCount + (offset * 2);
			  int padded_arr[padded_size] = {}; // Creates array with padded size

				// Generates oscillation of using a random number generator
				int bounds = 10; // sets the max and min bounds
				random_device rd;     // used to initialise (seed) engine
				mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
				uniform_int_distribution<int> uni(-bounds,bounds); // guaranteed unbiased; uni(min,max)
				int wiggle = uni(rng); // assigns the generated random number to the value 'wiggle', which gets used to assign values in the padded array

				// Needs to make sure the the wiggle bounding stays inside the maximum array size, so checks before assigning the values to the padded array
				if (bounds < offset) {
				  for (int k = 0; k < padded_size; k++) {
				    if (k >= (offset + wiggle) && k < (offset + wiggle + m_colCount) ) {
				        padded_arr[k] = arr[k-(offset + wiggle)];
				    }
				    else {
				      padded_arr[k] = 0;
				    }
				  }

					// Finds the max value and max value index for the padded array using the same method as the unpadded image above
					// Finds the maximum value of the ping for the padded_arr[]
					int max_padded = padded_arr[0]; // First assignment of the max value and index
					max_index_padded = 0;
					int h = 0;
					int bottom_edge_padded = 150; // This is a value at which the bottom return of the acoustic image appears
					for (h = 0; h < bottom_edge_padded; h++) {
						if (padded_arr[h] > max_padded) {
							max_padded = padded_arr[h];
							max_index_padded = h;
						}
					}

					// Printing padded_arr[] to file; appends row values to the end of the output file
					for (int n = 0; n < padded_size; n++) {
						if (n == (padded_size - 1)) {
							output_file_padded << padded_arr[n] << "\n";
						}
						else {
							output_file_padded << padded_arr[n] << ",";
						}
					}
				}
				else {
					// If we're out of bounds, will notify us that that's the case, and write that to file. Be careful of this when
					// setting these values in the mission before compiling
					output_file_padded << "$bounds is less that $offset, would write out of bounds" << endl;
					cout << "$bounds is less that $offset, would write out of bounds" << endl;
				}


				// Printing arr[] to file; appends row values to the end of the output file
				for (int m = 0; m < m_colCount; m++) {
					if (m == (m_colCount - 1)) {
						//printf("%d \n",arr[m]);
						output_file << arr[m] << "\n";

					}
					else {
						//printf("%d, ",arr[m]);
						output_file << arr[m] << ",";
					}
				}
				cout << "From m_iterations, wrote row number " << m_iterations << " to file" << endl;

			} else { cout << "Did NOT succesfully open INPUT or OUTPUT file successfully, OR passed end of file " << endl; }
	m_iterations++; // Putting this INSIDE the if-statement, so that we don't end up with discontinuities in the image
	}

	cout << "Max value index is: " << max_index << endl;
	// TO_DO: This conversion factor's NOT ACCURATE, but is chosen for convenience in the simulation for the moment
	double conversion_factor = 10.5/82; // dist_ideal/m_colCount
	double distance_from_pixels = max_index * conversion_factor; //
	Notify(m_outgoing_var,distance_from_pixels); // Writes the "distance" of the signal return (should be longline distance) to MOOSDB for pLineFollow
  //Notify(m_outgoing_var,10.5); // Can write SIM_DISTANCE == dist_ideal s.t. produces ideal behavior


	// Writes position and heading data to file for use in post-run analytics
	output_file_position << m_nav_x << "," << m_nav_y << ","<< m_nav_heading << "," << max_index << "," << distance_from_pixels << "," << endl;

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool IncludeSampleData::OnStartUp()
{

	// Start by converting the specified image to a .csv file
  // This process calls a Rust function from the image_to_csv_rs library
  cout << "STARTING image->.csv file conversion using Rust binary" << endl;
  string image_path = "./synthetic_image.png";
  string csv_export_path = "./csv_image_import.csv";
  int32_t image_return = process_image(image_path.c_str(),csv_export_path.c_str());
  if (image_return == 0) {
    cout << "Image processing in Rust function was SUCCESSFUL" << endl;
  }
  else {
    cout << "Image processing in Rust function was NOT SUCESSFUL" << endl;
  }

	// Opens input file and find rowCount and colCount numbers
	string line;
	char delim = ','; // Both 'delim' and 'delimiter' name the type of delimter used in the csv file (could be a colon or something)
	string delimiter = ",";
	size_t num_of_delim;
	ifstream input_file (m_input_filename);
	if (input_file.is_open())   // IF we can open the file...
  {
    while ( getline (input_file,line) ) // ... iterate through each row until EOF and increase m_rowCount each time
    {
      // If we're on the first line, use the number of delimters to determine the number of columns in the file
      if (m_rowCount == 0) {
        num_of_delim = std::count(line.begin(), line.end(), delim);
        m_colCount = (num_of_delim+1);
      }
      m_rowCount++; // Increases rowCount by 1 until the EOF
    }
  } else { cout << "Unable to open file"; } // Otherwise return an error saying that the file hasn't been opened
	cout << "In OnStartUp, m_rowCount and m_colCount are " << m_rowCount << " " << m_colCount << endl;


  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  m_MissionReader.GetConfiguration(GetAppName(), sParams);

	// Get variables from environment and Register
  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string sLine     = *p;
    string sVarName  = MOOSChomp(sLine, "=");
    sLine = stripBlankEnds(sLine);

		if(MOOSStrCmp(sVarName, "OUTGOING_VAR")) {
			if(!strContains(sLine, " "))
	  m_outgoing_var = stripBlankEnds(sLine);
	  }

    if(MOOSStrCmp(sVarName, "NAV_X_RECEIVED")) {
      if(!strContains(sLine, " "))
    m_nav_x_received = stripBlankEnds(sLine);
    }

		if(MOOSStrCmp(sVarName, "NAV_Y_RECEIVED")) {
	     if(!strContains(sLine, " "))
	  m_nav_y_received = stripBlankEnds(sLine);
	  }

		if(MOOSStrCmp(sVarName, "NAV_HEADING_RECEIVED")) {
		    if(!strContains(sLine, " "))
		  m_nav_heading_received = stripBlankEnds(sLine);
		}

		if(MOOSStrCmp(sVarName, "MODE_RECEIVED")) {
				if(!strContains(sLine, " "))
			m_mode_received = stripBlankEnds(sLine);
		}


  }
  RegisterVariables();

  return(true);
}
