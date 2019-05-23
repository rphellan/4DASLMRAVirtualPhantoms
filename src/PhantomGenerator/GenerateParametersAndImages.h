#ifndef GENERATE_PARAMETER_IMAGES_H
#define GENERATE_PARAMETER_IMAGES_H

//This program is the second step for vessel flow simulation.
//The inputs are:
//1) Coloured binary segmentation for 3 vessel regions.
//2) Distances from seed point to voxel.
//3) Sum of speeds up to a position.
//4) Flip angle value.
//5) Temporal resolution.
//6) First time point in ms.
//7) Last time point in ms.
//8) Repetition time.
//9) Output directory to save blood flow parameter images.
// The outputs are:
//1) Images for parameters A, delta_t, s, and p according to Okell's model.
//2) Not noisy cerebrovascular tree image, with signal variation along the path.

//*Self note: Images are in TOF MRA space

//Common headers
#include "ArgumentsReader.h"
#include "ImageManagement.h"
#include "VesselRadii.h"

//Generates the blood parameter images.
void GenerateTheParametersAndImages(char *separatedVesselsImagePath, char *distancesImagePath, char *accVelocityImagePath,
                           int flipAngle, int imagingFrequencyInMs, int labelingTime, int firstTimePoint, int lastTimePoint, double TR, char *outputDir);

//Tells the user which arguments are required for this application.
void DisplayHelpParameterImages();

//Reads the arguments required for this application.
bool ReadArgumentsParameterImages(
        int argc, char **argv, char **separatedVesselsImagePath, char **distancesImagePath, char **accVelocityImagePath,
        int & flipAngle, int & imagingFrequencyInMs, int & labelingTime, int & firstTimePoint, int & lastTimePoint, double &TR, char **outputDir);
#endif
