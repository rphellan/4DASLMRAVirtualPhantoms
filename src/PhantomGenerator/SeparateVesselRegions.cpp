#include "SeparateVesselRegions.h"

using namespace std;

int MinBranching(DoubleImageType::Pointer & branchingNumbers, DoubleImageType::Pointer & regions, int regionNumber)
{
   int min = 10000;
   for (int x = 0; x < branchingNumbers->GetLargestPossibleRegion().GetSize()[0]; x++)
      for (int y = 0; y < branchingNumbers->GetLargestPossibleRegion().GetSize()[1]; y++)
         for (int z = 0; z < branchingNumbers->GetLargestPossibleRegion().GetSize()[2]; z++)
         {
            DoubleImageType::IndexType index;
            index[0] = x; index[1] = y; index[2] = z;

            if (regions->GetPixel(index) == regionNumber)            
               if (branchingNumbers->GetPixel(index) < min) min = branchingNumbers->GetPixel(index);            
        }
   return min;
}

void SepVesselRegions(char *inputSegmentationPath, char *vesselRegionsPath, int xl, int yl, int zl, int xr, int yr, int zr, int xb, int yb, int zb,
                      char *separatedVesselsImagePath, char *distancesImagePath, char *accVelocityImagePath)
{
   DoubleImageType::Pointer inputSegmentationImage, inputSegmentationImageMainComponent, vesselRadiiImage;
   DoubleImageType::Pointer distancesImage, vesselRegionsImage, velocitiesRandomizedImage, accumulatedVelocityIftImage, branchedTimesImage;
   double PI = 3.14159265;
   char cmd[1000] = {0}, path[1000] = {0};

   ReadImage(inputSegmentationPath, inputSegmentationImage);

   //Keep only the largest connected component.
   IntImageType::Pointer inputSegmentationImageInt;
   CastDoubleImageAsIntImage(inputSegmentationImage, inputSegmentationImageInt);

   //This filters finds the connected components.
   typedef itk::ConnectedComponentImageFilter <IntImageType, IntImageType> ConnectedComponentImageFilterType;
   ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
   connected->SetInput(inputSegmentationImageInt);
   connected->SetFullyConnected(1);
   connected->Update();
 
   //This filter orders the labels. The biggest object has label 1, the second biggest is number 2 ... 0 is background.
   typedef itk::RelabelComponentImageFilter<IntImageType, DoubleImageType>  RelabelFilterType;
   RelabelFilterType::Pointer relabeler = RelabelFilterType::New();
   relabeler->SetInput(connected->GetOutput());
   relabeler->SetMinimumObjectSize(1000);
   relabeler->Update();

   //Keep only component 1.
   inputSegmentationImageMainComponent = relabeler->GetOutput();
   itk::ImageRegionIterator<DoubleImageType> imageIt(inputSegmentationImageMainComponent, inputSegmentationImageMainComponent->GetLargestPossibleRegion());
   for (imageIt.GoToBegin(); !imageIt.IsAtEnd(); ++imageIt)
       if (imageIt.Get() > 1.0)
           imageIt.Set(0);

   DoubleImageType::Pointer simpleSkeletonImage, numberedSkeletonImage, voronoiImage;
   SkeletonizeAndShowVoronoiImage(inputSegmentationImageMainComponent, simpleSkeletonImage, numberedSkeletonImage, voronoiImage);

   //Calculate the radius of each vessel
   IntImageType::Pointer inputSegmentationImageMainComponentInt;
   CastDoubleImageAsIntImage(inputSegmentationImageMainComponent, inputSegmentationImageMainComponentInt);
   typedef itk::BinaryThresholdImageFilter<IntImageType, DoubleImageType> BinaryFilterType;
   BinaryFilterType::Pointer binarizer = BinaryFilterType::New();
   binarizer->SetInput(inputSegmentationImageMainComponentInt);
   binarizer->SetOutsideValue( 0 );
   binarizer->SetInsideValue( 1 );
   binarizer->SetLowerThreshold( 1 );
   binarizer->SetUpperThreshold( 1000 );
   binarizer->Update();

   DuplicateDoubleImage(inputSegmentationImage, vesselRadiiImage);
   ComputeVesselRadiiImage(binarizer->GetOutput(), vesselRadiiImage);
   
   //Calculate branching information with IFT algorithm.
      //Use a cool IFT strategy to isolate LICA, RICA, and Basiliar regions (3 manual initial points are required).
      //Make sure the seeds are located in the skeleton!!!
   DoubleImageType::IndexType indexLICA, indexRICA, indexBasiliar;
   indexLICA[0] = xl; indexLICA[1] = yl; indexLICA[2] = zl;
   indexRICA[0] = xr; indexRICA[1] = yr; indexRICA[2] = zr;
   indexBasiliar[0] = xb; indexBasiliar[1] = yb; indexBasiliar[2] = zb;
   inputSegmentationImageMainComponent->SetPixel(indexLICA, 2);
   inputSegmentationImageMainComponent->SetPixel(indexRICA, 3);
   inputSegmentationImageMainComponent->SetPixel(indexBasiliar, 4); 

   simpleSkeletonImage->SetPixel(indexLICA, 2);
   simpleSkeletonImage->SetPixel(indexRICA, 3);
   simpleSkeletonImage->SetPixel(indexBasiliar, 4); 

      //Now, call the IFT.
   sprintf(cmd, "mkdir tmp%d", getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));

      //Write image in hdr format, preparing for IFT. Also, write image as short int, because that format is supported in IFT.
   sprintf(path, "./tmp%d/inputSegmentationImageMainComponent.hdr", getpid());
   ShortIntImageType::Pointer inputSegmentationImageMainComponentShortInt;
   CastDoubleImageAsShortIntImage(inputSegmentationImageMainComponent, inputSegmentationImageMainComponentShortInt);
   WriteShortIntImage(path, inputSegmentationImageMainComponentShortInt);
   memset(path, 0, sizeof(path));

   sprintf(path, "./tmp%d/simpleSkeletonImage.hdr", getpid());
   ShortIntImageType::Pointer simpleSkeletonImageShortInt;
   CastDoubleImageAsShortIntImage(simpleSkeletonImage, simpleSkeletonImageShortInt);
   WriteShortIntImage(path, simpleSkeletonImageShortInt);
   memset(path, 0, sizeof(path));

      //Transform everything to IFT scn format.
   sprintf(cmd, "$NEWIFT_DIR/bin/iftAnalyze2Scn ./tmp%d/inputSegmentationImageMainComponent ./tmp%d/inputSegmentationImageMainComponent.scn", getpid(), getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));

   sprintf(cmd, "$NEWIFT_DIR/bin/iftAnalyze2Scn ./tmp%d/simpleSkeletonImage ./tmp%d/simpleSkeletonImage.scn", getpid(), getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));

      //Count the number of branching points up to the observed voxel using an IFT-based algorithm.
   sprintf(cmd, "$NEWIFT_DIR/bin/iftCountBranching ./tmp%d/simpleSkeletonImage.scn ./tmp%d/branchedTimesImage.scn", getpid(), getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));

      //Go back from .scn to .hdr to .nii.
   sprintf(cmd, "$NEWIFT_DIR/bin/iftScn2Analyze ./tmp%d/branchedTimesImage.scn ./tmp%d/branchedTimesImage", getpid(), getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));

   sprintf(path, "./tmp%d/branchedTimesImage.hdr", getpid());
   ReadImage(path, branchedTimesImage);
   memset(path, 0, sizeof(path));

   CopyDirectionOriginSpacing(inputSegmentationImage, branchedTimesImage, branchedTimesImage);
   double branchNumbersVector[1000000] = {0.0};

   for (int x = 0; x < numberedSkeletonImage->GetLargestPossibleRegion().GetSize()[0]; x++)
      for (int y = 0; y < numberedSkeletonImage->GetLargestPossibleRegion().GetSize()[1]; y++)
         for (int z = 0; z < numberedSkeletonImage->GetLargestPossibleRegion().GetSize()[2]; z++)
         {
            DoubleImageType::IndexType indexSk;
            indexSk[0] = x; indexSk[1] = y; indexSk[2] = z;
            if (numberedSkeletonImage->GetPixel(indexSk) > 0.0)
               branchNumbersVector[(int)(numberedSkeletonImage->GetPixel(indexSk))] = branchedTimesImage->GetPixel(indexSk);
        }

   for (int x = 0; x < branchedTimesImage->GetLargestPossibleRegion().GetSize()[0]; x++)
      for (int y = 0; y < branchedTimesImage->GetLargestPossibleRegion().GetSize()[1]; y++)
         for (int z = 0; z < branchedTimesImage->GetLargestPossibleRegion().GetSize()[2]; z++)
         {
            DoubleImageType::IndexType indexBT;
            indexBT[0] = x; indexBT[1] = y; indexBT[2] = z;

            if (inputSegmentationImageMainComponent->GetPixel(indexBT) >= 1.0)            
               branchedTimesImage->SetPixel(indexBT, branchNumbersVector[(int)(voronoiImage->GetPixel(indexBT))]);            
        }

   //Vessel regions have to be manually identified, but an atlas could help.
      //Vessel region labels used in this project.
   double RightCavernousICA = 1.0, LeftCavernousICA = 2.0, RightSupraclinoidICA = 3.0, LeftSupraclinoidICA = 4.0, Basilar = 5.0;
   double RightPCA = 6.0, LeftPCA = 7.0, RightMCA1 = 8.0, RightMCASuperiorInferiorM2 = 9.0, LeftMCA1 = 10.0, LeftMCASuperiorInferiorM2 = 11.0, ACA = 12.0;

   ReadImage(vesselRegionsPath, vesselRegionsImage);
   ReadImage(vesselRegionsPath, velocitiesRandomizedImage);

   CopyDirectionOriginSpacing(inputSegmentationImage, vesselRegionsImage, vesselRegionsImage);
   CopyDirectionOriginSpacing(inputSegmentationImage, velocitiesRandomizedImage, velocitiesRandomizedImage);

      //Randomly generated speeds.
   double RICAFlow, LICAFlow, BAFlow;
  
   //Distribute velocities conserving mass, up to where it is possible.
   //Consider flow distribution at branching points according to "Blood flow distribution in cerebral arteries" by Laleh Zarrinkoob.

   //initialize random seed
   srand(time(NULL));
   
   //The flow 
   LICAFlow = 500 + ((double)rand() / RAND_MAX) * 10;
   RICAFlow = 500 + ((double)rand() / RAND_MAX) * 10;
   BAFlow = 200 + ((double)rand() / RAND_MAX) * 10;

   // Find branching minimum in relevant regions
   double minBRightPCA, minBLeftPCA, minBRightMCASuperiorInferiorM2, minBLeftMCASuperiorInferiorM2, minBACA;
   minBRightPCA = MinBranching(branchedTimesImage, vesselRegionsImage, RightPCA);
   minBLeftPCA = MinBranching(branchedTimesImage, vesselRegionsImage, LeftPCA);
   minBRightMCASuperiorInferiorM2 = MinBranching(branchedTimesImage, vesselRegionsImage, RightMCASuperiorInferiorM2);
   minBLeftMCASuperiorInferiorM2 = MinBranching(branchedTimesImage, vesselRegionsImage, LeftMCASuperiorInferiorM2);
   minBACA = MinBranching(branchedTimesImage, vesselRegionsImage, ACA);

   itk::ImageRegionIterator<DoubleImageType> vesselRegionsIt(vesselRegionsImage, vesselRegionsImage->GetLargestPossibleRegion());
   itk::ImageRegionIterator<DoubleImageType> velocitiesRanIt(velocitiesRandomizedImage, velocitiesRandomizedImage->GetLargestPossibleRegion());
   itk::ImageRegionIterator<DoubleImageType> radiiIt(vesselRadiiImage, vesselRadiiImage->GetLargestPossibleRegion());
   itk::ImageRegionIterator<DoubleImageType> branchedTimesIt(branchedTimesImage, branchedTimesImage->GetLargestPossibleRegion());
   for (vesselRegionsIt.GoToBegin(), velocitiesRanIt.GoToBegin(), radiiIt.GoToBegin(), branchedTimesIt.GoToBegin(); 
        !vesselRegionsIt.IsAtEnd(); ++vesselRegionsIt, ++velocitiesRanIt, ++radiiIt, ++branchedTimesIt)
       if (vesselRegionsIt.Get() > 0.0)
       {
           if (vesselRegionsIt.Get() == RightCavernousICA) velocitiesRanIt.Set(RICAFlow / (PI * radiiIt.Get()* radiiIt.Get() ) );
           if (vesselRegionsIt.Get() == LeftCavernousICA) velocitiesRanIt.Set(LICAFlow / (PI * radiiIt.Get()* radiiIt.Get() ));
           if (vesselRegionsIt.Get() == RightSupraclinoidICA) velocitiesRanIt.Set(RICAFlow / (PI * radiiIt.Get()* radiiIt.Get() ));
           if (vesselRegionsIt.Get() == LeftSupraclinoidICA) velocitiesRanIt.Set(LICAFlow / (PI * radiiIt.Get()* radiiIt.Get() ));
           if (vesselRegionsIt.Get() == Basilar) velocitiesRanIt.Set(BAFlow / (PI * radiiIt.Get()* radiiIt.Get() ));
           if (vesselRegionsIt.Get() == RightMCA1) velocitiesRanIt.Set(RICAFlow * 2.0 / (3.0 * PI * radiiIt.Get()* radiiIt.Get() ) );
           if (vesselRegionsIt.Get() == LeftMCA1) velocitiesRanIt.Set(LICAFlow * 2.0 / (3.0 * PI * radiiIt.Get()* radiiIt.Get() ) );
 
           if (vesselRegionsIt.Get() == RightPCA)           
              velocitiesRanIt.Set( (BAFlow * 1.0 / pow(2, branchedTimesIt.Get() - minBRightPCA) ) / (2.0 * PI * radiiIt.Get()* radiiIt.Get() )); 
           if (vesselRegionsIt.Get() == LeftPCA)
              velocitiesRanIt.Set( (BAFlow * 1.0 / pow(2, branchedTimesIt.Get() - minBLeftPCA) ) / (2.0 * PI * radiiIt.Get()* radiiIt.Get() )); 
           if (vesselRegionsIt.Get() == RightMCASuperiorInferiorM2)
              velocitiesRanIt.Set( (RICAFlow * 1.0 / pow(2, branchedTimesIt.Get() - minBRightMCASuperiorInferiorM2) ) / (2.0 / 3.0 * PI * radiiIt.Get()* radiiIt.Get() ));
           if (vesselRegionsIt.Get() == LeftMCASuperiorInferiorM2)
              velocitiesRanIt.Set( (LICAFlow * 1.0 / pow(2, branchedTimesIt.Get() - minBLeftMCASuperiorInferiorM2) ) / (2.0 / 3.0 * PI * radiiIt.Get()* radiiIt.Get() ));
           if (vesselRegionsIt.Get() == ACA)
              velocitiesRanIt.Set( (RICAFlow + LICAFlow) / (2.0 * 3.0 * PI * radiiIt.Get()* radiiIt.Get()) / pow(2, branchedTimesIt.Get() - minBACA) );
           if (velocitiesRanIt.Get() > 50.0) velocitiesRanIt.Set(50.0);   
           if (velocitiesRanIt.Get() < 10.0) velocitiesRanIt.Set(velocitiesRanIt.Get() * 10.0);   
       }

   //Write image in hdr format, preparing for IFT. Also, write image as short int for IFT.
   sprintf(path, "./tmp%d/velocitiesRandomizedImage.hdr", getpid());
   ShortIntImageType::Pointer velocitiesRandomizedImageShortInt;
   CastDoubleImageAsShortIntImage(velocitiesRandomizedImage, velocitiesRandomizedImageShortInt);
   WriteShortIntImage(path, velocitiesRandomizedImageShortInt);
   memset(path, 0, sizeof(path));

      //Move everything to IFT scn format.
   sprintf(cmd, "$NEWIFT_DIR/bin/iftAnalyze2Scn ./tmp%d/velocitiesRandomizedImage ./tmp%d/velocitiesRandomizedImage.scn", getpid(), getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));

      //Run the IFT.
   sprintf(cmd, 
           "$NEWIFT_DIR/bin/iftIsolateVessels ./tmp%d/inputSegmentationImageMainComponent.scn ./tmp%d/separatedVesselsImage.scn ./tmp%d/distancesImage.scn ./tmp%d/arrivalTimesIft.scn ./tmp%d/velocitiesRandomizedImage.scn"
           ,getpid(), getpid(), getpid(), getpid(), getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));

   //Outputs are: 
   //1) Coloured binary segmentation for 3 vessel regions.
   //2) Distances from seed point to voxel.
   //3) Sum of speeds up to a position.

      //Go back from .scn to .hdr to .nii.
   sprintf(cmd, "$NEWIFT_DIR/bin/iftScn2Analyze ./tmp%d/separatedVesselsImage.scn ./tmp%d/separatedVesselsImage", getpid(), getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));
   sprintf(path, "./tmp%d/separatedVesselsImage.hdr", getpid());
   ReadImage(path, inputSegmentationImageMainComponent);
   memset(path, 0, sizeof(path));

   CopyDirectionOriginSpacing(inputSegmentationImage, inputSegmentationImageMainComponent, inputSegmentationImageMainComponent);
   WriteImage(separatedVesselsImagePath, inputSegmentationImageMainComponent);

   sprintf(cmd, "$NEWIFT_DIR/bin/iftScn2Analyze ./tmp%d/distancesImage.scn ./tmp%d/distancesImage", getpid(), getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));
   sprintf(path, "./tmp%d/distancesImage.hdr", getpid());
   ReadImage(path, distancesImage);
   memset(path, 0, sizeof(path)); 

   CopyDirectionOriginSpacing(inputSegmentationImage, distancesImage, distancesImage);
   WriteImage(distancesImagePath, distancesImage);

   sprintf(cmd, "$NEWIFT_DIR/bin/iftScn2Analyze ./tmp%d/arrivalTimesIft.scn ./tmp%d/arrivalTimesIft", getpid(), getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));
   sprintf(path, "./tmp%d/arrivalTimesIft.hdr", getpid());
   ReadImage(path, accumulatedVelocityIftImage);
   memset(path, 0, sizeof(path));

   CopyDirectionOriginSpacing(inputSegmentationImage, accumulatedVelocityIftImage, accumulatedVelocityIftImage);
   WriteImage(accVelocityImagePath, accumulatedVelocityIftImage);   

   //remove tmp dir
   sprintf(cmd, "rm -rf tmp%d", getpid());
   system(cmd);
   memset(cmd, 0, sizeof(cmd));
}

void DisplayHelpSepVesselRegions()
{
    cout << "Usage:" << endl
         << "SeparateVesselRegions -v vascular system -r vessel regions -a xl -b yl -c zl -d xr -e yr -f zr -g xb -h yb -i zb -s separated vessel regions -p distances -u accumulated velocity" << endl
         << "vascular system: binary vessel segmentation (background value 0, vessels value 1)." << endl
         << "vessel regions: separated vessel regions (manually or with atlas, with labels indicated in the code)." << endl
         << "a to i: (x, y, z) coordinates of a point in the skeleton of the LICA (l), RICA (r), or Basilar artery (b). You can use ComputeSkeleton to calculate the skeleton of your vessel segmentation." << endl
         << "separated vessel regions: output image showing the LICA, RICA, and BA regions." << endl
         << "distances: output image with distances from seed point to any voxel." << endl
         << "accumulated velocity: output image with sum of velocities from seed point to any voxel." << endl
         ;
}

bool ReadArgumentsSepVesselRegions(
        int argc, char **argv, char **inputSegmentationPath, char **vesselRegionsPath, int &xl, int &yl, int &zl, int &xr, int &yr, int &zr, int &xb, int &yb, int &zb,
        char **separatedVesselsImagePath, char **distancesImagePath, char **accVelocityImagePath)
{
   int option;

   if (argc < 29)
   {
      DisplayHelpSepVesselRegions();
      return false;
   }

   while ((option = getopt(argc, argv, "v:r:a:b:c:d:e:f:g:h:i:s:p:u:")) != EOF)
   {
       switch (option)
       {
            case 'v' : *inputSegmentationPath = optarg;
                break;
            case 'r' : *vesselRegionsPath = optarg;
                break;
            case 'a' : xl = atoi(optarg);
                break;
            case 'b' : yl = atoi(optarg);
                break;
            case 'c' : zl = atoi(optarg);
                break;
            case 'd' : xr = atoi(optarg);
                break;
            case 'e' : yr = atoi(optarg);
                break;
            case 'f' : zr = atoi(optarg);
                break;
            case 'g' : xb = atoi(optarg);
                break;
            case 'h' : yb = atoi(optarg);
                break;
            case 'i' : zb = atoi(optarg);
                break;
            case 's' : *separatedVesselsImagePath = optarg;
                break;				
            case 'p' : *distancesImagePath = optarg;
               break;
            case 'u' : *accVelocityImagePath = optarg;
                break;
            default  : DisplayHelpSepVesselRegions();
                       return false;
       }
   }

   return true;
}

int main(int argc, char *argv[])
{

    char *inputSegmentationPath, *vesselRegionsPath, *separatedVesselsImagePath, *distancesImagePath, *accVelocityImagePath;
    int xl, yl, zl, xr, yr, zr, xb, yb, zb;

    if (!ReadArgumentsSepVesselRegions(argc, argv, &inputSegmentationPath, &vesselRegionsPath, xl, yl, zl, xr, yr, zr, xb, yb, zb,
        &separatedVesselsImagePath, &distancesImagePath, &accVelocityImagePath))
    {
       //Bad arguments.
       return 1;
    }

    SepVesselRegions(inputSegmentationPath, vesselRegionsPath, xl, yl, zl, xr, yr, zr, xb, yb, zb, separatedVesselsImagePath, distancesImagePath, accVelocityImagePath);
}

