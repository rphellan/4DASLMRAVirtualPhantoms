#include "ift.h"

iftImage *iftIsolateVesselPaths(iftImage *image, double adjacency, char *distancesPath, char *arrivalTimesPath, char *velocitiesImageInput);

iftImage *iftIsolateVesselPaths(iftImage *image, double adjacency, char *distancesPath, char *arrivalTimesPath, char *velocitiesImageInput)
{
    iftImage *finalSegmentation = NULL;
    iftImage *velocitiesImage = iftReadImage(velocitiesImageInput);
    int p;
    iftVoxel u;
    iftAdjRel *adjacencyStruct;
    adjacencyStruct = iftSpheric(adjacency);
    iftImageForest *fst;
    fst = iftCreateImageForest(image, adjacencyStruct);
    iftFImage *distancesF  = iftCreateFImage(image->xsize, image->ysize, image->zsize); //like pathval
    iftFImage *velocitiesF = iftCreateFImage(image->xsize, image->ysize, image->zsize); //like arrival time
    iftImage *distancesI;
    iftImage *velocitiesI;

    //Seeding (2, 3, and 4 are arteries)
    for (u.z = 0; u.z < image->zsize; u.z++)
        for (u.y = 0; u.y < image->ysize; u.y++)
            for (u.x = 0; u.x < image->xsize; u.x++)
            {
                p = iftGetVoxelIndex(image,u);

                if (image->val[p] == 1.0)
                {
                    fst->pathval->val[p] = INFINITY_INT;
		    distancesF->val[p] = INFINITY_INT;
                    fst->pred->val[p] = NIL;		    
                }

                if (image->val[p] == 2.0)
                {
                    fst->label->val[p] = 2;
                    fst->pathval->val[p] = 0;
		    distancesF->val[p] = 0;
                    iftInsertGQueue(&(fst->Q), p);
                    fst->root->val[p] = 0;
		    velocitiesF->val[p] = 0;
                }

                if (image->val[p] == 3.0)
                {
                    fst->label->val[p] = 3;
                    fst->pathval->val[p] = 0;
		    distancesF->val[p] = 0;
                    iftInsertGQueue(&(fst->Q), p);
                    fst->root->val[p] = 0;
		    velocitiesF->val[p] = 0;
                }
                if (image->val[p] == 4.0)
                {
                    fst->label->val[p] = 4;
                    fst->pathval->val[p] = 0;
		    distancesF->val[p] = 0;
                    iftInsertGQueue(&(fst->Q), p);
                    fst->root->val[p] = 0;
		    velocitiesF->val[p] = 0;
                }
                if (image->val[p] < 1.0) //Background seeds are ignored
                {
                    fst->label->val[p] = 0;
                    fst->pathval->val[p] = 0;
		    distancesF->val[p] = 0;
                    fst->root->val[p] = 0;
		    velocitiesF->val[p] = 0;
                }
            }

    int      i,q;
    float tmp;
    iftVoxel    v;

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
                    if (distancesF->val[q] > distancesF->val[p])
                    {
                        tmp = MAX(distancesF->val[p], distancesF->val[p] + sqrt( (u.x-v.x)*(u.x-v.x)*0.4102*0.4102 + (u.y-v.y)*(u.y-v.y)*0.4102*0.4102 + (u.z-v.z)*(u.z-v.z)*0.7*0.7 ));
                        if (tmp < distancesF->val[q])
                        {
                            if (fst->Q->L.elem[q].color == GRAY) iftRemoveGQueueElem(fst->Q, q);
                            fst->label->val[q] = fst->label->val[p];
                            distancesF->val[q] = tmp;
                            if (velocitiesF->val[q] < 32000)
                               velocitiesF->val[q] = velocitiesF->val[p] + (int)(velocitiesImage->val[q]) ; //accumulate arrival time
                            else
                               velocitiesF->val[q] = velocitiesF->val[p];
                            iftInsertGQueue(&fst->Q, q);
                        }
                    }
                }
            }
        }
    }

    distancesI = iftCreateImage(distancesF->xsize,distancesF->ysize,distancesF->zsize);
    velocitiesI = iftCreateImage(velocitiesF->xsize,velocitiesF->ysize,velocitiesF->zsize);
    
    finalSegmentation = iftCopyImage(fst->label);
    
    int r, s;
    for (u.z = 0; u.z < distancesF->zsize; u.z++)
       for (u.y = 0; u.y < distancesF->ysize; u.y++)
         for (u.x = 0; u.x < distancesF->xsize; u.x++)
            {
                p = iftGetVoxelIndex (distancesI,u);
		q = iftFGetVoxelIndex(distancesF,u);
                r = iftGetVoxelIndex (velocitiesI,u);
		s = iftFGetVoxelIndex(velocitiesF,u);

  	        distancesI->val[p] = (int)distancesF->val[q];
		if (distancesI->val[p] < 0) distancesI->val[p] = 0;		
		velocitiesI->val[r] = (int)velocitiesF->val[s];
		if (velocitiesI->val[p] < 0) velocitiesI->val[p] = 0;		
	    }   
    
    iftWriteImage(velocitiesI, arrivalTimesPath);      
    iftWriteImage(distancesI, distancesPath);

    return finalSegmentation;
}

int main(int argc, char *argv[]){
    iftImage *image, *segmentation = NULL;

    if (argc == 6)
    {
        clock_t t1,t2;
        t1=clock();

        image = iftReadImage(argv[1]);
        double adjacency = 1.0;

        segmentation = iftIsolateVesselPaths(image, adjacency, argv[3], argv[4], argv[5]);

        iftWriteImage(segmentation, argv[2]);

        iftDestroyImage(&image);

        t2=clock();
        printf("Use %5.4f \n", ((float)t2 - (float)t1)/CLOCKS_PER_SEC);

        return(0);

    } else {
        fprintf(stderr,"usage: iftIsolateVessels <image> <out> <distances image> <arrival times image> <velocities image input>\n");
        return(-1);
    }
}

