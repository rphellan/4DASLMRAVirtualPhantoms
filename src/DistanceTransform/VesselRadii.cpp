#include "VesselRadii.h"

using namespace std;

void ComputeVesselRadiiImage(DoubleImageType::Pointer input, DoubleImageType::Pointer & output)
{
   DoubleImageType::Pointer distanceValues;
   IntImageType::Pointer inputInteger, voronoiMap, numberedSkeleton;
   CastDoubleImageAsIntImage(input, inputInteger);

   //Fill possible holes in the binary image
   typedef itk::VotingBinaryIterativeHoleFillingImageFilter<IntImageType> HoleFillerType;
   HoleFillerType::Pointer holeFiller = HoleFillerType::New();
   holeFiller->SetInput(inputInteger);
   holeFiller->SetMaximumNumberOfIterations(20);
   holeFiller->SetBackgroundValue(0);
   holeFiller->SetForegroundValue(1);
   holeFiller->Update();

   //Invert binary image to make sure distances are positive values
   typedef itk::NotImageFilter<IntImageType, IntImageType> NotFilterType;
   NotFilterType::Pointer notFilter = NotFilterType::New();
   notFilter->SetInput(holeFiller->GetOutput());
   notFilter->Update();

   //Calculate distance from border to center of the vessels
   typedef itk::DanielssonDistanceMapImageFilter<IntImageType, DoubleImageType> DistanceMapFilterType;
   DistanceMapFilterType::Pointer distanceMap = DistanceMapFilterType::New();
   distanceMap->SetInput(notFilter->GetOutput());
   distanceMap->InputIsBinaryOn();
   distanceMap->UseImageSpacingOn();
   distanceMap->Update();

   //Calculate skeleton of the vessels
   typedef itk::BinaryThinningImageFilter3D< IntImageType, IntImageType > ThinningFilterType;
   ThinningFilterType::Pointer thinningFilter = ThinningFilterType::New();
   thinningFilter->SetInput( holeFiller->GetOutput() );
   thinningFilter->Update();

   distanceValues = distanceMap->GetDistanceMap();
   numberedSkeleton = thinningFilter->GetOutput();

   //Assign a number to each vessel and store the corresponding distance.
   int count = 1;
   double distancesVector[10000] = {0.0};
   for (int x = 0; x < numberedSkeleton->GetLargestPossibleRegion().GetSize()[0]; x++)
      for (int y = 0; y < numberedSkeleton->GetLargestPossibleRegion().GetSize()[1]; y++)
         for (int z = 0; z < numberedSkeleton->GetLargestPossibleRegion().GetSize()[2]; z++)
         {
            IntImageType::IndexType indexInput;
            indexInput[0] = x; indexInput[1] = y; indexInput[2] = z;
            DoubleImageType::IndexType indexDistance;
            indexDistance[0] = x; indexDistance[1] = y; indexDistance[2] = z;
            if (numberedSkeleton->GetPixel(indexInput) > 0.0)
            {
               numberedSkeleton->SetPixel(indexInput, count);
               distancesVector[count] = distanceValues->GetPixel(indexDistance);
               count++;
            }
         }

   //Calculate Voronoi map of numberedSkeleton
   DistanceMapFilterType::Pointer distanceMap2 = DistanceMapFilterType::New();
   distanceMap2->SetInput(numberedSkeleton);
   distanceMap2->InputIsBinaryOff();
   distanceMap2->UseImageSpacingOn();
   distanceMap2->Update();
   voronoiMap = distanceMap2->GetVoronoiMap();

   // Propogate distance values along the voronoi regions
   for (int x = 0; x < output->GetLargestPossibleRegion().GetSize()[0]; x++)
      for (int y = 0; y < output->GetLargestPossibleRegion().GetSize()[1]; y++)
         for (int z = 0; z < output->GetLargestPossibleRegion().GetSize()[2]; z++)
         {
            DoubleImageType::IndexType indexInput;
            indexInput[0] = x; indexInput[1] = y; indexInput[2] = z;
            DoubleImageType::IndexType indexOutput;
            indexOutput[0] = x; indexOutput[1] = y; indexOutput[2] = z;
            IntImageType::IndexType indexVoronoi;
            indexVoronoi[0] = x; indexVoronoi[1] = y; indexVoronoi[2] = z;

            if (input->GetPixel(indexInput) > 0.0)
               output->SetPixel(indexOutput, distancesVector[voronoiMap->GetPixel(indexVoronoi)]);
         }
}
