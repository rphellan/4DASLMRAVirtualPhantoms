#include "ift.h"

iftImage *iftCountBranches(iftImage *image, double adjacency);
int branchSize(iftImage *binaryImage, int root, int branchStart, int depth);

int branchSize(iftImage *binaryImage, int root, int branchStart, int depth)
{
    if (depth > 7.0) return 1.0;
    if (branchStart == -1.0) return 1.0;

    iftAdjRel *adjacencyStruct;
    adjacencyStruct = iftSpheric(sqrt(3.0));
    
    int branchSizeValue, branchContinue = -1.0, nbi;
    iftVoxel u, nbv;
    u = iftGetVoxelCoord(binaryImage, branchStart);
    for (int nb = 1; nb < adjacencyStruct->n; nb++)
    {
       nbv = iftGetAdjacentVoxel(adjacencyStruct, u, nb);
       if (iftValidVoxel(binaryImage, nbv))
       {
          nbi = iftGetVoxelIndex(binaryImage, nbv);
          if ((nbi != root) && (binaryImage->val[nbi] > 0.0))
          {
             branchContinue = nbi;  
          }
       }
    }
    branchSizeValue = 1 + branchSize(binaryImage, branchStart, branchContinue, depth + 1);   

    return branchSizeValue;
}

iftImage *iftCountBranches(iftImage *image, double adjacency)
{
    iftImage *finalSegmentation = NULL;
    int p;
    iftVoxel u;
    iftAdjRel *adjacencyStruct;
    adjacencyStruct = iftSpheric(adjacency);
    iftImageForest *fst;
    fst = iftCreateImageForest(image, adjacencyStruct);

    //Seeding (2, 3, and 4 are arteries)
    for (u.z = 0; u.z < image->zsize; u.z++)
        for (u.y = 0; u.y < image->ysize; u.y++)
            for (u.x = 0; u.x < image->xsize; u.x++)
            {
                p = iftGetVoxelIndex(image,u);

                if (image->val[p] == 1.0)
                {
                    fst->pathval->val[p] = INFINITY_INT;
                    fst->pred->val[p] = NIL;		    
                }

                if (image->val[p] == 2.0)
                {
                    fst->label->val[p] = 2;
                    fst->pathval->val[p] = 0;
                    iftInsertGQueue(&(fst->Q), p);
                    fst->root->val[p] = 0;
                }

                if (image->val[p] == 3.0)
                {
                    fst->label->val[p] = 3;
                    fst->pathval->val[p] = 0;
                    iftInsertGQueue(&(fst->Q), p);
                    fst->root->val[p] = 0;
                }
                if (image->val[p] == 4.0)
                {
                    fst->label->val[p] = 4;
                    fst->pathval->val[p] = 0;
                    iftInsertGQueue(&(fst->Q), p);
                    fst->root->val[p] = 0;
                }
                if (image->val[p] < 1.0) //Background seeds are ignored
                {
                    fst->label->val[p] = 0;
                    fst->pathval->val[p] = 0;
                    fst->root->val[p] = 0;
                    //iftInsertGQueue(&(fst->Q), p);
                }
            }

    //Watershed
    int      i,q, isBranching = 0, nbi;
    int tmp;
    iftVoxel    v, nbv;

    while(!iftEmptyGQueue(fst->Q))
    {
        p = iftRemoveGQueue(fst->Q);
        u = iftGetVoxelCoord(fst->img, p);
        for (i=1; i < fst->A->n; i++)
        {
            v = iftGetAdjacentVoxel(fst->A, u, i);
            if (iftValidVoxel(fst->img,v))
            {
                q = iftGetVoxelIndex(fst->img, v);
                if (fst->Q->L.elem[q].color != BLACK)
                {
                    //Use the basic IFT function for basins, which is smooth
                    if (fst->pathval->val[q] > fst->pathval->val[p])
                    {
                        int onNeighbors = 0, previousIsNotLess = 1;
                        for (int nb = 1; nb < fst->A->n; nb++)
                        {
                           nbv = iftGetAdjacentVoxel(fst->A, u, nb);
                           if (iftValidVoxel(fst->img, nbv))
                           {
                               nbi = iftGetVoxelIndex(fst->img, nbv);
                               //Require that the branch is longer than 6 voxels
                               if ( (fst->img->val[nbi] >= 1.0) && (branchSize(fst->img, p, nbi, 1.0) > 5.0)) onNeighbors++;
                               if ( (fst->img->val[nbi] >= 1.0) && (fst->pathval->val[nbi] < fst->pathval->val[p]) ) previousIsNotLess = 0;
                           }                                                     
                        }
                        isBranching = (onNeighbors >= 3.0);
                        if (isBranching && previousIsNotLess)
                        {
                           tmp = fst->pathval->val[p] + 1;
                        } 
                        else
                           tmp = fst->pathval->val[p];
                        if (tmp < fst->pathval->val[q])
                        {
                            if (fst->Q->L.elem[q].color == GRAY) iftRemoveGQueueElem(fst->Q, q);
                            fst->pathval->val[q] = tmp;
                            fst->label->val[q] = fst->label->val[p];
                            iftInsertGQueue(&fst->Q, q);
                        }
                    }
                }
            }
        }
    }
  
    finalSegmentation = iftCopyImage(fst->pathval);

    return finalSegmentation;
}

int main(int argc, char *argv[]){
    iftImage *image, *segmentation = NULL;

    if (argc == 3)
    {
        clock_t t1,t2;
        t1=clock();

        image = iftReadImage(argv[1]);
        double adjacency = sqrt(3.0);

        segmentation = iftCountBranches(image, adjacency);

        iftWriteImage(segmentation, argv[2]);

        iftDestroyImage(&image);

        t2=clock();
        printf("Use %5.4f \n", ((float)t2 - (float)t1)/CLOCKS_PER_SEC);

        return(0);

    } else {
        fprintf(stderr,"usage: iftCountBranching <image> <out>\n");
        return(-1);
    }
}

