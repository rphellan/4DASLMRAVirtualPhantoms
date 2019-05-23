#include "ImageManagement.h"

void ReadImage(char* path, DoubleImageType::Pointer & image)
{
    typedef itk::ImageFileReader<DoubleImageType> ImageReaderType;
    ImageReaderType::Pointer imageReader = ImageReaderType::New();
    imageReader->SetFileName(path);
    imageReader->Update();
    image = imageReader->GetOutput();
}

void WriteImage(char* path, DoubleImageType::Pointer image)
{
    typedef itk::ImageFileWriter<DoubleImageType> WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->SetFileName(path);
    imageWriter->SetInput(image);
    imageWriter->Update();
}

void WriteShortIntImage(char* path, ShortIntImageType::Pointer image)
{
    typedef itk::ImageFileWriter<ShortIntImageType> WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->SetFileName(path);
    imageWriter->SetInput(image);
    imageWriter->Update();
}

void GetMinimumAndMaximum(DoubleImageType::Pointer inputImage, double & minimum, double & maximum)
{
   typedef itk::MinimumMaximumImageCalculator <DoubleImageType> ImageCalculatorFilterType;
   ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New ();
   imageCalculatorFilter->SetImage(inputImage);
   imageCalculatorFilter->Compute();

   minimum = imageCalculatorFilter->GetMinimum();
   maximum = imageCalculatorFilter->GetMaximum();
}

void MaskImage(DoubleImageType::Pointer image, DoubleImageType::Pointer mask, DoubleImageType::Pointer & maskedImage)
{
   typedef itk::MaskImageFilter<DoubleImageType, DoubleImageType> MaskFilterType;
   MaskFilterType::Pointer maskFilter = MaskFilterType::New();
   maskFilter->SetInput(image);
   maskFilter->SetMaskImage(mask);
   maskFilter->Update();
   maskedImage = maskFilter->GetOutput();
}

void CreateDoubleImage(DoubleImageType::Pointer & doubleImage, DoubleImageType::IndexType start,
                       DoubleImageType::SizeType size)
{
   doubleImage = DoubleImageType::New();
   DoubleImageType::RegionType region;
   region.SetSize( size );
   region.SetIndex( start );
   doubleImage->SetRegions( region );
   doubleImage->Allocate();
}

void CastDoubleImageAsIntImage(DoubleImageType::Pointer inputImage, IntImageType::Pointer & outputImage)
{
   typedef itk::CastImageFilter< DoubleImageType, IntImageType > CastFilterType;
   CastFilterType::Pointer castFilter = CastFilterType::New();
   castFilter->SetInput(inputImage);
   castFilter->Update();
   outputImage = castFilter->GetOutput();
}

void CastIntImageAsDoubleImage(IntImageType::Pointer inputImage, DoubleImageType::Pointer & outputImage)
{
   typedef itk::CastImageFilter< IntImageType, DoubleImageType > CastFilterType;
   CastFilterType::Pointer castFilter = CastFilterType::New();
   castFilter->SetInput(inputImage);
   castFilter->Update();
   outputImage = castFilter->GetOutput();
}

void CastDoubleImageAsShortIntImage(DoubleImageType::Pointer inputImage, ShortIntImageType::Pointer & outputImage)
{
   typedef itk::CastImageFilter< DoubleImageType, ShortIntImageType > CastFilterType;
   CastFilterType::Pointer castFilter = CastFilterType::New();
   castFilter->SetInput(inputImage);
   castFilter->Update();
   outputImage = castFilter->GetOutput();
}

void DuplicateDoubleImage(DoubleImageType::Pointer inputImage, DoubleImageType::Pointer & outputImage)
{
    typedef itk::ImageDuplicator< DoubleImageType > DuplicatorType;
    DuplicatorType::Pointer duplicator = DuplicatorType::New();
    duplicator->SetInputImage(inputImage);
    duplicator->Update();
    outputImage = duplicator->GetOutput();
}

void CopyDirectionOriginSpacing(DoubleImageType::Pointer inputImage1, DoubleImageType::Pointer inputImage2, DoubleImageType::Pointer & outputImage)
{
    typedef itk::ChangeInformationImageFilter< DoubleImageType > FilterRespaceType;
    FilterRespaceType::Pointer respace = FilterRespaceType::New();

    respace->SetOutputDirection(inputImage1->GetDirection());
    respace->ChangeDirectionOn();
    respace->SetOutputOrigin(inputImage1->GetOrigin());
    respace->ChangeOriginOn();
    respace->SetOutputSpacing(inputImage1->GetSpacing());
    respace->ChangeSpacingOn();

    respace->SetInput(inputImage2);
    respace->Update();
    outputImage = respace->GetOutput();
}

void SkeletonizeAndShowVoronoiImage(DoubleImageType::Pointer inputImage, DoubleImageType::Pointer & simpleSkeleton, DoubleImageType::Pointer & numberedSkeletonImage, DoubleImageType::Pointer & voronoi)
{
    IntImageType::Pointer inputInteger, voronoiMap, numberedSkeleton;
    DoubleImageType::Pointer numberedSkeletonDouble;

    CastDoubleImageAsIntImage (inputImage, inputInteger);

    typedef itk::VotingBinaryIterativeHoleFillingImageFilter<IntImageType> HoleFillerType;
    HoleFillerType::Pointer holeFiller = HoleFillerType::New();
    holeFiller->SetInput(inputInteger);
    holeFiller->SetMaximumNumberOfIterations(20);
    holeFiller->SetBackgroundValue(0);
    holeFiller->SetForegroundValue(1);
    holeFiller->Update();

    typedef itk::BinaryThinningImageFilter3D< IntImageType, IntImageType > ThinningFilterType;
    ThinningFilterType::Pointer thinningFilter = ThinningFilterType::New();
    thinningFilter->SetInput( holeFiller->GetOutput() );
    thinningFilter->Update();

    numberedSkeleton = thinningFilter->GetOutput();
    CastIntImageAsDoubleImage(numberedSkeleton, numberedSkeletonDouble);
    DuplicateDoubleImage(numberedSkeletonDouble, simpleSkeleton);

    int count = 1;
    for (int x = 0; x < numberedSkeleton->GetLargestPossibleRegion().GetSize()[0]; x++)
       for (int y = 0; y < numberedSkeleton->GetLargestPossibleRegion().GetSize()[1]; y++)
          for (int z = 0; z < numberedSkeleton->GetLargestPossibleRegion().GetSize()[2]; z++)
          {
             IntImageType::IndexType indexInput;
             indexInput[0] = x; indexInput[1] = y; indexInput[2] = z;
             if (numberedSkeleton->GetPixel(indexInput) > 0.0)
             {
                numberedSkeleton->SetPixel(indexInput, count);
                count ++;
             }
          }

    CastIntImageAsDoubleImage(numberedSkeleton, numberedSkeletonImage);

    typedef itk::DanielssonDistanceMapImageFilter<IntImageType, DoubleImageType> DistanceMapFilterType;
    DistanceMapFilterType::Pointer distanceMap2 = DistanceMapFilterType::New();
    distanceMap2->SetInput(numberedSkeleton);
    distanceMap2->InputIsBinaryOff();
    distanceMap2->UseImageSpacingOn();
    distanceMap2->Update();

    voronoiMap = distanceMap2->GetVoronoiMap();

    CastIntImageAsDoubleImage(voronoiMap, voronoi);
}
