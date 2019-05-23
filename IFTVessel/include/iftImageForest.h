#ifndef IFT_IMAGEFOREST_H_
#define IFT_IMAGEFOREST_H_

#include "iftCommon.h"
#include "iftImage.h"
#include "iftAdjacency.h"
#include "iftGQueue.h"
#include "iftSet.h"
#include "iftLabeledSet.h"
#include "iftSeeds.h"
#include "iftBMap.h"


typedef struct ift_imageforest {
  iftImage  *pathval;
  iftImage  *label;
  iftImage  *root;
  iftImage  *pred;
  iftBMap   *processed;
  iftGQueue *Q;
  iftImage  *img;
  iftAdjRel *A;
} iftImageForest;

iftImageForest  *iftCreateImageForest(iftImage *img, iftAdjRel *A);
void             iftResetImageForest(iftImageForest *fst);
void             iftDestroyImageForest(iftImageForest **fst);
void             iftCompRemovalWithoutFrontier(iftImageForest *fst, iftLabeledSet *seed);
iftSet          *iftCompRemoval(iftImageForest *fst, iftLabeledSet *seed);
iftSet          *iftTreeRemoval(iftImageForest *fst, iftLabeledSet *seed);
iftImage        *iftSwitchTreeLabels(iftImageForest *fst, iftLabeledSet *seed, iftImage *label);
void             iftRelabelBorderTreesAsBkg(iftImageForest *fst);
void 			 iftPropagateRootandLabelToSubtree(iftImageForest *fst, iftAdjRel *A, int new_label, int r);
iftSet 		  	*iftPropagationWaveBorderSet(iftImageForest *fst);


#endif


