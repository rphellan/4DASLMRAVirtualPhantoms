#include "GenerateParametersAndImages.h"

using namespace std;

double AssignOkellValue(double timePoint, double labelingTime, double flipAngle, double tInitial, double TR, double bloodVolume, double bloodArrivalTime, double dispersionSharpness, double dispersionTTP)
{
   //TimePoint, labelingTime, tInitial in milliseconds, flipAngle in degrees.
   double result, integralDTR = 0.0, Dispersion, T1Relaxation, RFPulseMitigation, gammaPS, Nvalue;

   double T1Blood = 1664; //miliseconds
   double PI = 3.14159265;
   flipAngle = flipAngle * PI / 180.0; //convert to radians
   double step = 1.0;
   
   //Using the midpoint rule http://tutorial.math.lamar.edu/Classes/CalcII/ApproximatingDefIntegrals.aspx
   for (double td = timePoint - bloodArrivalTime - labelingTime + (step * 1.0 / 2.0) ; td <= timePoint - bloodArrivalTime - (step * 1.0 / 2.0); td += step)
   {  
      if ( ((dispersionSharpness * td) > 0) && ((dispersionTTP * dispersionSharpness) > -1) )
      {
         double eExcess = dispersionSharpness * (td - dispersionTTP);
         Dispersion = 1.0;
         for (int i = 0; i < (dispersionTTP * dispersionSharpness); i++ )
            Dispersion = Dispersion * dispersionSharpness * td / ( (dispersionTTP * dispersionSharpness - i) * exp(1.0) );
         Dispersion = Dispersion * dispersionSharpness * exp(-eExcess) / ( 1 + (dispersionTTP * dispersionSharpness) );          
      }
      else
         Dispersion = 0.0;

      //The signal is reduced by a factor of pow ( 1 / e = 0.36 , fraction of T1 of blood at 3T )
      T1Relaxation = exp(-1.0*(bloodArrivalTime + td)/T1Blood); 

      integralDTR += Dispersion * T1Relaxation * step;

   }

   Nvalue = (timePoint - tInitial) * 1.0 / TR;
   RFPulseMitigation = pow(cos(flipAngle), Nvalue);

   result = integralDTR * RFPulseMitigation * bloodVolume * sin(flipAngle);

   return result;
}

void GenerateTheParametersAndImages(char *separatedVesselsImagePath, char *distancesImagePath, char *accVelocityImagePath,
                           int flipAngle, int imagingFrequencyInMs, int labelingTime, int firstTimePoint, int lastTimePoint, double TR, char *outputDir)
{
   char path[1000] = {0};
   DoubleImageType::Pointer vesselPathsImage, vesselPathsRadiiImage, vesselVelocitiesAccumulatedImage, vesselDistancesImage, vesselPredecessorsImage;
   DoubleImageType::Pointer AImage, deltaTImage, sImage, pImage;
   IntImageType::Pointer vesselPathsImageInt;

   ReadImage(separatedVesselsImagePath, vesselPathsImage);

   ReadImage(distancesImagePath, vesselDistancesImage);   
   ReadImage(accVelocityImagePath, vesselVelocitiesAccumulatedImage);
   CopyDirectionOriginSpacing(vesselPathsImage, vesselVelocitiesAccumulatedImage, vesselVelocitiesAccumulatedImage);
   CopyDirectionOriginSpacing(vesselPathsImage, vesselDistancesImage, vesselDistancesImage);   

   //1. Calculate relative volume image (A)

   //Assign a radius/diameter value to the whole vascular tree
   CastDoubleImageAsIntImage(vesselPathsImage, vesselPathsImageInt);
   typedef itk::BinaryThresholdImageFilter<IntImageType, DoubleImageType> BinaryFilterType;
   BinaryFilterType::Pointer binarizer = BinaryFilterType::New();
   binarizer->SetInput(vesselPathsImageInt);
   binarizer->SetOutsideValue( 0 );
   binarizer->SetInsideValue( 1 );
   binarizer->SetLowerThreshold( 1 );
   binarizer->SetUpperThreshold( 1000 );
   binarizer->Update();

   DuplicateDoubleImage(vesselPathsImage, vesselPathsRadiiImage);
   ComputeVesselRadiiImage(binarizer->GetOutput(), vesselPathsRadiiImage);

   //Distribute A according to the square of the radius: First find the maximum radius, and assign it the maxVolume, and then go proportionally.
   double maxRelativeVolume = 100.0, maxRadius = 0.0;
   DuplicateDoubleImage(vesselPathsImage, AImage);

   itk::ImageRegionIterator<DoubleImageType> radiiIt(vesselPathsRadiiImage, vesselPathsRadiiImage->GetLargestPossibleRegion());
   for (radiiIt.GoToBegin(); !radiiIt.IsAtEnd(); ++radiiIt)
       if (radiiIt.Get() > maxRadius)
           maxRadius = radiiIt.Get();

   itk::ImageRegionIterator<DoubleImageType> AIt(AImage, AImage->GetLargestPossibleRegion());
   for (radiiIt.GoToBegin(), AIt.GoToBegin(); !radiiIt.IsAtEnd(); ++radiiIt, ++AIt)
       if (radiiIt.Get() > 0)
          AIt.Set( round(maxRelativeVolume * ( radiiIt.Get() * radiiIt.Get() ) / ( maxRadius * maxRadius )) + 5.0);
       else
          AIt.Set(0);

   sprintf(path, "%sAImage.nii", outputDir);
   WriteImage(path, AImage);
   memset(path, 0, sizeof(path));

   //2. Distribute parameters s and p according to the distance
   
   double maxDistance = 0.0, maxS = 5.0, maxP = 7.0; // S in 10 to 5, P in 5 to 12
   DuplicateDoubleImage(vesselPathsImage, sImage);
   DuplicateDoubleImage(vesselPathsImage, pImage);

   itk::ImageRegionIterator<DoubleImageType> distIt(vesselDistancesImage, vesselDistancesImage->GetLargestPossibleRegion());
   for (distIt.GoToBegin(); !distIt.IsAtEnd(); ++distIt)
       if (distIt.Get() > maxDistance)
           maxDistance = distIt.Get();

   itk::ImageRegionIterator<DoubleImageType> sIt(sImage, sImage->GetLargestPossibleRegion());
   itk::ImageRegionIterator<DoubleImageType> pIt(pImage, pImage->GetLargestPossibleRegion());
   itk::ImageRegionIterator<DoubleImageType> pathsIt(vesselPathsImage, vesselPathsImage->GetLargestPossibleRegion());

   for (distIt.GoToBegin(), pathsIt.GoToBegin(), sIt.GoToBegin(), pIt.GoToBegin(); !distIt.IsAtEnd(); ++distIt, ++pathsIt, ++sIt, ++pIt)
       if (pathsIt.Get() > 0)
       {
          if (distIt.Get() > 0)
             sIt.Set( maxS * ( 1.0 - ( distIt.Get() * 1.0 / maxDistance ) ) + 5.0);
          else
             sIt.Set( 5.0 );
          pIt.Set( maxP * ( distIt.Get() * 1.0 / maxDistance ) + 5.0);
       }
       else
       {
          sIt.Set(0);
          pIt.Set(0);
       }

   sprintf(path, "%ssImage.nii", outputDir);
   WriteImage(path, sImage);
   memset(path, 0, sizeof(path));

   sprintf(path, "%spImage.nii", outputDir);
   WriteImage(path, pImage);
   memset(path, 0, sizeof(path));

   //3. Distribute arrival times according to the average speeds average speed in the path, and use the distance information to compute the right arrival time
   DuplicateDoubleImage(vesselPathsImage, deltaTImage);
   itk::ImageRegionIterator<DoubleImageType> accVelIt(vesselVelocitiesAccumulatedImage, vesselVelocitiesAccumulatedImage->GetLargestPossibleRegion());
   itk::ImageRegionIterator<DoubleImageType> deltaTIt(deltaTImage, deltaTImage->GetLargestPossibleRegion());

   for (accVelIt.GoToBegin(), deltaTIt.GoToBegin(), pathsIt.GoToBegin(), distIt.GoToBegin(); !distIt.IsAtEnd(); ++accVelIt, ++deltaTIt, ++pathsIt, ++distIt)
       if (pathsIt.Get() > 0)
       {
          if (accVelIt.Get() > 0)
             deltaTIt.Set( distIt.Get() / ( accVelIt.Get() * 10.0 / distIt.Get() ) );
          else
             deltaTIt.Set( 0 );
       }
       else
          deltaTIt.Set(0);

   double min, max;
   GetMinimumAndMaximum(deltaTImage, min, max);
   cout << min << " " << max << endl;
   for (deltaTIt.GoToBegin(); !deltaTIt.IsAtEnd(); ++deltaTIt)
      deltaTIt.Set( deltaTIt.Get() * 1700.0 / max );

   sprintf(path, "%sdeltaTImage.nii", outputDir);
   WriteImage(path,deltaTImage);
   memset(path, 0, sizeof(path));

   //4. Finally, combine everything using Okell's model
   for (int phaseTime = firstTimePoint; phaseTime <= lastTimePoint; phaseTime = phaseTime + imagingFrequencyInMs)
   {
      DoubleImageType::Pointer phaseImage;
      DuplicateDoubleImage(vesselPathsImage, phaseImage);
      itk::ImageRegionIterator<DoubleImageType> phaseImageIt(phaseImage, phaseImage->GetLargestPossibleRegion());

      for (pathsIt.GoToBegin(), AIt.GoToBegin(), sIt.GoToBegin(), pIt.GoToBegin(), deltaTIt.GoToBegin(),
           phaseImageIt.GoToBegin(); !pathsIt.IsAtEnd(); ++pathsIt, ++AIt, ++sIt, ++pIt, ++deltaTIt, ++phaseImageIt)
          if (pathsIt.Get() > 0)
             phaseImageIt.Set( AssignOkellValue(phaseTime, labelingTime * 1.0, flipAngle * 1.0, firstTimePoint * 1.0, TR, AIt.Get(), deltaTIt.Get(), (int)(sIt.Get()), (int)(pIt.Get())) );
          else
             phaseImageIt.Set(0);

      sprintf(path, "%st%d.nii", outputDir, phaseTime);
      WriteImage(path, phaseImage);
      memset(path, 0, sizeof(path));
   }
}

void DisplayHelpParameterImages()
{
    cout << "Usage: " << endl
         << "GenerateParametersAndImages -p separatedVesselsImagePath -d vesselDistancesImagePath -v vesselVelocitiesAccumulatedImagePath "
         << "-f flipAngle(degrees) -r imagingFrequencyInMs -l labelingTimeInMs -t firstTimePointInMs -e lastTimePointInMs -k TRInMs -o output directory" << endl
		 << "Note that output directory must end with a slash (/)." << endl;
}

bool ReadArgumentsParameterImages(
        int argc, char **argv, char **separatedVesselsImagePath, char **distancesImagePath, char **accVelocityImagePath,
        int & flipAngle, int & imagingFrequencyInMs, int & labelingTime, int & firstTimePoint, int & lastTimePoint, double & TR, char **outputDir)
{
   int option;

   if (argc < 21)
   {
      DisplayHelpParameterImages();
      return false;
   }

   while ((option = getopt(argc, argv, "p:d:v:f:r:l:t:e:k:o:")) != EOF)
   {
       switch (option)
       {
            case 'p' : *separatedVesselsImagePath = optarg;
                break;
            case 'v' : *distancesImagePath = optarg;
                break;
            case 'd' : *accVelocityImagePath = optarg;
                break;
            case 'f' : flipAngle = atoi(optarg);
                break;
            case 'r' : imagingFrequencyInMs = atoi(optarg);
                break;
            case 'l' : labelingTime = atoi(optarg);
                break;
            case 't' : firstTimePoint = atoi(optarg);
                break;
            case 'e' : lastTimePoint = atoi(optarg);
                break;
            case 'k' : TR = atof(optarg);
                break;
            case 'o' : *outputDir = optarg;
                break;
            default  : DisplayHelpParameterImages();
                       return false;
       }
   }

   return true;
}

int main(int argc, char *argv[])
{

    char *separatedVesselsImagePath, *distancesImagePath, *accVelocityImagePath, *outputDir;
    int flipAngle, imagingFrequencyInMs, labelingTime, firstTimePoint, lastTimePoint;
    double TR;

    if (!ReadArgumentsParameterImages(argc, argv, &separatedVesselsImagePath, &distancesImagePath, &accVelocityImagePath, 
        flipAngle, imagingFrequencyInMs, labelingTime, firstTimePoint, lastTimePoint, TR, &outputDir))
    {
       //Bad arguments.
       return 1;
    }

    GenerateTheParametersAndImages(separatedVesselsImagePath, distancesImagePath, accVelocityImagePath, flipAngle, imagingFrequencyInMs, 
                          labelingTime, firstTimePoint, lastTimePoint, TR, outputDir);
}

