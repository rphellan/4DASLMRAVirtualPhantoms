#ifndef IFT_REGISTRATIONMSPS_H_
#define IFT_REGISTRATIONMSPS_H_

#include "iftCommon.h"
#include "iftAdjacency.h"
#include "iftImage.h"
#include "iftFImage.h"
#include "iftRepresentation.h"
#include "iftGeometric.h"
#include "iftObjectModels.h"

void             iftRegisterImageElastix(char *fixedImagePath, char *movingImagePath, char *paramsAffine, char *paramsBSpline, char *outBasename);
void             iftRegisterDirectoryElastix(char *fixedImagePath, char *movingImagesDir, char *paramFileAffine, char *paramFileBSpline, char *outImagesDir, char *outMasksDir);
void             iftDeformMaskTransformix(char *binaryMaskPath, char *paramsBSpline, char *outPath);
void             iftDeformDirectoryTransformix(char *masksDirectory, char *outDirectory);
void             iftAlignImagesForSimilarity(char *dirIn, char *dirOut);

#endif


