#ifndef SEPARATE_VESSEL_REGIONS_H
#define SEPARATE_VESSEL_REGIONS_H

//This program is the first step for vessel flow simulation.
//The inputs are:
//1) Binary vessel segmentation.
//2) A mask indicating the region that belongs to each vessel, manually done or using an atlas.
//3) Three manual seeds (x, y, and z coordinates of the start point of left and right internal carotid arteries, and basiliar artery).
//Note: The seeds must be in the skeleton of the binary vessel segmentation, as close to the base of the neck as possible.
//The outputs are:
//1) Coloured binary segmentation for 3 vessel regions.
//2) Distances from seed point to voxel.
//3) Sum of speeds up to a position.

//Common headers
#include "ArgumentsReader.h"
#include "ImageManagement.h"
#include "VesselRadii.h"

//Applies the vessel separation algorithm.
void SepVesselRegions(char *inputSegmentationPath, char *vesselRegionsPath, int xl, int yl, int zl, int xr, int yr, int zr, int xb, int yb, int zb,
                      char *separatedVesselsImagePath, char *distancesImagePath, char *accVelocityImagePath);

//Tells the user which arguments are required for this application.
void DisplayHelpSepVesselRegions();

//Reads the arguments required for this application.
bool ReadArgumentsSepVesselRegions(
        int argc, char **argv, char **inputSegmentationPath, char **vesselRegionsPath, int &xl, int &yl, int &zl, int &xr, int &yr, int &zr, int &xb, int &yb, int &zb,
        char **separatedVesselsImagePath, char **distancesImagePath, char **accVelocityImagePath);

#endif
