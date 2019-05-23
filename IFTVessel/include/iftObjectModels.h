#ifndef IFT_OBJECTMODELS_H_
#define IFT_OBJECTMODELS_H_

#include <dirent.h>
#include "iftCommon.h"
#include "iftImage.h"
#include "iftFImage.h"
#include "iftRepresentation.h"

typedef struct ift_fuzzy_model_extended {
  iftFImage *model;
  iftAdjRel *AIn;
  iftAdjRel *AOut;
  iftAdjRel *AUncertain;
  iftVoxel  start;
  iftVoxel  end;
  float     mean;
  float     deviation;
  int       mode;
} iftFuzzyModelExtended;

typedef struct ift_statistic_model_simple {
  iftFImage *model;
  float     mean;
  float     deviation;
  int       mode;
} iftStatisticModelSimple;

iftImage             *iftCropObjectLoose(iftImage *obj, int xLSize, int yLSize, int zLSize);
void                 iftModelDomain(char *dirname, int *xsize, int *ysize, int *zsize);
int                  iftHasUncertain6Adjacent(iftFImage *model, int i);
void                 iftModelAdjacencies(iftFImage *model, iftAdjRel **AIn, iftAdjRel **AOut, iftAdjRel **AUncertain);
void                 iftFuzzyByAveraging(iftFImage *model, fileList *imageFiles);
void                 iftFuzzyByDistanceTransform(iftFImage *model, fileList *imageFiles);
void                 iftComputeFuzzyModel(char *dirname, iftFImage *model, char option);
void                 iftWriteFuzzyModelExtended(iftFuzzyModelExtended *fuzzyModel, char *filename);
iftFuzzyModelExtended *iftReadFuzzyModelExtended(char *filename);
void                 iftModelPositionsFuzzyExtended(char *dirImages, char *dirInstances, iftFuzzyModelExtended *fuzzyModel);
void	             iftWriteStatisticModelSimple(iftStatisticModelSimple *statisticModel, char *filename);
void                 iftDestroyStatisticModelSimple(iftStatisticModelSimple **statisticModel);
iftStatisticModelSimple *iftReadStatisticModelSimple(char *filename);
void                 iftDestroyFuzzyModelExtended(iftFuzzyModelExtended **fuzzyModel);
void                 iftComputeHistogramParameters(char *dirImages, char *dirInstances, float *meanP, int *modeP, float *sdP);
void                 iftFuzzyByAveragingNoGCCentering(iftFImage *model, fileList *imageFiles);
void                 iftModelIntensitiesFuzzyExtended(char *dirImages, char *dirInstances, iftFuzzyModelExtended *fuzzyModel);
void                 iftFuzzyByAveragingDilated(iftFImage *model, char *dirname);
iftStatisticModelSimple *iftMakeStatisticModelSimple(char *imagesDirectory, char *segmentationsDirectory);
iftImage             *iftSegmentWithStatisticModelSimple(char *imageRoute, char *modelRoute);
iftImage             *iftAPosterioriGradient(iftImage *image, iftFuzzyModelExtended *model, float alpha);
iftImage             *iftAPosterioriGradientWithClassicSoam(iftImage *image, iftStatisticModelSimple *model);

#endif


