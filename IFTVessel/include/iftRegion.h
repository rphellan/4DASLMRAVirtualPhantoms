#ifndef IFTREGIONGRAPH_H_
#define IFTREGIONGRAPH_H_

#include "iftImage.h"
#include "iftCommon.h"
#include "iftImage.h"
#include "iftAdjacency.h"
#include "iftDataSet.h"
#include "iftLabeledSet.h"
#include "iftSet.h"
#include "iftMathMorph.h"

typedef struct{
	iftSet* adjacent;
} iftRegionGraphNode;

typedef struct{
	iftRegionGraphNode *node;
	iftDataSet *dataset;
	int         nnodes;

	float  *root;
	float  *pred;
	float      *pathval;
	iftFHeap   *heap;
} iftRegionGraph;

iftRegionGraph* iftCreateRegionGraph(iftDataSet* dataset, int n_regions);
void            iftDestroyRegionGraph(iftRegionGraph** rg);
iftRegionGraph* iftRegionGraphFromLabelImage(iftImage* label_image, iftDataSet* dataset, iftAdjRel* adjacency);
void            iftSuperpixelClassification(iftRegionGraph *graph, iftImage *label_image, iftLabeledSet *pixel_seeds);
void 			iftDiffSuperpixelClassification(iftRegionGraph *graph, iftImage *label_image, iftLabeledSet *new_seeds);
void			iftFastSuperpixelClassification(iftRegionGraph *graph, iftImage *label_image, iftLabeledSet *pixel_seeds);
void iftSuperpixelClassificationGeodesic(iftRegionGraph *graph, iftImage *label_image, iftLabeledSet *pixel_seeds, float beta);

iftImage* iftRelabelRegions(iftImage* labelled, iftAdjRel* adj_rel);
iftLabeledSet* iftGeodesicCenters(iftImage* label_image);
iftLabeledSet* iftGeometricCenters(iftImage* label_image);

iftImage*       iftCreateRefinedLabelImage(iftImage* image, iftLabeledSet* seed, float spatial_radius, int volume_threshold, int steps, float vol_ratio);

//Given a label_image numbered from 1 to n and a dataset with n samples, sets the classes for the superpixels from the ground-truth image.
void iftSetSuperpixelClassesFromGroundTruth(iftImage* label_image, iftImage* groud_truth, iftDataSet* dataset);

//Creates an image where each superpixel has the grayscale value defined by its label in the dataset
iftImage* iftSuperpixelLabelImageFromDataset(iftImage *label_image, iftDataSet *dataset);

//Finds the regions that are adjacent to regions with different label in the /graph/'s dataset
iftBMap		*iftAdjacentToDifferentLabels(iftRegionGraph *graph);

//Binary tree node
typedef struct iftRegionHierarchyNode{
	struct iftRegionHierarchyNode *left;
	struct iftRegionHierarchyNode *right;

	int merging_time; //Indicates when the region was merged
	int region; //If this node is a leaf contains the region id. Otherwise contains NIL
    int id; // unique id starting from 1 used to identify leaves and non-leaves alike

	int xmax, ymax, zmax, xmin, ymin, zmin; //Bounding cube coordinates
} iftRegionHierarchyNode;

//Binary tree representing a hierarchical watershed segmentation
typedef struct{
	iftRegionHierarchyNode *root; //Root of the hierarchy (node with the highest merging_time)
	int nleaves; //Number of leaves in the tree (regions)

	iftSet** pixels_by_node; //List of pixels by (unmerged) region. Speeds up prunning.
	iftImage* label_image; //Label image representing the initial regions
} iftRegionHierarchy;

//Creates an internal region hierarchy node
iftRegionHierarchyNode* iftCreateRegionHierarchyNode(int merging_time, iftRegionHierarchyNode* left, iftRegionHierarchyNode *right, int node_id);
//Creates a region hierarchy leaf node
iftRegionHierarchyNode* iftCreateRegionHierarchyLeaf(iftImage* label_image, iftSet* pixels, int region, int node_id);

typedef float* (*iftMergingFun)(float *f1, float *f2, float *alpha, int n);

iftRegionHierarchy* iftCreateRegionHierarchy(iftDataSet *dataset, iftAdjRel* adjacency, iftMergingFun merge_func);
void iftDestroyRegionHierarchy(iftRegionHierarchy** rh);

//Creates a label_image with nregions given a region hierarchy
iftImage* iftFlattenRegionHierarchy(iftRegionHierarchy* rh, int nregions);
void iftRHDrawSubregions(iftRegionHierarchy *rh, iftRegionHierarchyNode *node, iftImage* label_image, int label);
int iftRHFlatten(iftRegionHierarchy *rh, iftRegionHierarchyNode* node, iftImage *label_image, int cutoff, int current_label);

//Eliminates regions with area below a given number of pixels
iftImage* iftEliminateRegionsByArea(iftDataSet* dataset, iftAdjRel *adj, int threshold);

//Returns a list containing every node in the hierarchy (nleaves + root->merging_time elements)
iftRegionHierarchyNode** iftGetRegionHierarchyNodes(iftRegionHierarchy *rh);

#endif /* iftregiongraph_h_ */
