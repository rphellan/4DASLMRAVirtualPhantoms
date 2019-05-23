#ifndef VESSEL_RADII_H
#define VESSEL_RADII_H

//This program computes the vessels radii in a binary image.
//The input is a binary image with segmented vessels (value 0 for background, value 1 for vessels).
//The output is an image with the radius of each vessel.

//Common headers
#include "ImageManagement.h"

//Compute the radius of the vessels and generate an image.
void ComputeVesselRadiiImage(DoubleImageType::Pointer input, DoubleImageType::Pointer & output);

#endif
