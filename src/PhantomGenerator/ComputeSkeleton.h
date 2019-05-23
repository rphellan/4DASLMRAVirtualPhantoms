#ifndef COMPUTE_SKELETON_H
#define COMPUTE_SKELETON_H

//This program calculates the skeleton of a binary segmentation.
//Note that it cleans the segmentation and only keeps one connected component.

//Common headers
#include "ArgumentsReader.h"
#include "ImageManagement.h"

//Applies the nii to scn conversion
void Skeletonize(char *input, char *output);

//Tells the user which arguments are required for this application
void DisplayHelpSkel();

//Reads the arguments required for this application
bool ReadArgumentsSkel(
        int argc, char **argv, char **input, char **output);

#endif
