#ifndef IMAGE_MANAGEMENT_H
#define IMAGE_MANAGEMENT_H

//General image manipulation
#include "itkImage.h"

//Image I/O
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

//To iterate through images
#include "itkImageIterator.h"

//To calculate minimum and maximum intensity values
#include "itkMinimumMaximumImageCalculator.h"

//To apply a mask to an image and delete what is outside of that mask
#include "itkMaskImageFilter.h"

//To find connected components, remove the smaller ones and relabel them
#include "itkRelabelComponentImageFilter.h"
#include "itkConnectedComponentImageFilter.h"

//To cast double images as int images
#include "itkCastImageFilter.h"

//To binarize images
#include "itkBinaryThresholdImageFilter.h"

//To duplicate images
#include "itkImageDuplicator.h"

//To modify direction, origin and spacing of an image
#include "itkChangeInformationImageFilter.h"

//For morphological erotion and dilation
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryBallStructuringElement.h"

//To fill in holes of medium size (tens of pixels in radius).
#include "itkVotingBinaryIterativeHoleFillingImageFilter.h"

//To negate binary images
#include "itkNotImageFilter.h"

//To calculate distance transform
#include "itkDanielssonDistanceMapImageFilter.h"

//To skeletonize vessels
#include "itkBinaryThinningImageFilter3D.h"

//Mathematical functions, time and type limits
#include "math.h"
#include "limits.h"
#include "time.h"

//To generate random numbers
#include "random"

//To obtain process pid
#include "unistd.h"

//Definitions for 3-dimensional images
const int dimension3 = 3;

//Short integer image type, required for some algorithms
typedef short int ShortIntPixelType;
typedef itk::Image<ShortIntPixelType, dimension3> ShortIntImageType;

//Integer image type
typedef int IntPixelType;
typedef itk::Image<IntPixelType, dimension3> IntImageType;

//Double image type
typedef double DoublePixelType;
typedef itk::Image<DoublePixelType, dimension3> DoubleImageType;

//Read and write images
void ReadImage(char* path, DoubleImageType::Pointer & image);
void WriteImage(char* path, DoubleImageType::Pointer image);
void WriteShortIntImage(char* path, ShortIntImageType::Pointer image);

//Get the minimum and maximum intensity values of an image
void GetMinimumAndMaximum(DoubleImageType::Pointer inputImage, double & minimum, double & maximum);

//Mask an image and delete what is outside of the mask.
void MaskImage(DoubleImageType::Pointer image, DoubleImageType::Pointer mask, DoubleImageType::Pointer & maskedImage);

//This function allocates memory for an image with voxels of type double
void CreateDoubleImage(DoubleImageType::Pointer & doubleImage, DoubleImageType::IndexType start,
                       DoubleImageType::SizeType size);

//Cast double image as int image
void CastDoubleImageAsIntImage(DoubleImageType::Pointer inputImage, IntImageType::Pointer & outputImage);

//Cast int image as double image
void CastIntImageAsDoubleImage(IntImageType::Pointer inputImage, DoubleImageType::Pointer & outputImage);

//Cast double image as int image
void CastDoubleImageAsShortIntImage(DoubleImageType::Pointer inputImage, ShortIntImageType::Pointer & outputImage);

//Duplicate a double image
void DuplicateDoubleImage(DoubleImageType::Pointer inputImage, DoubleImageType::Pointer & outputImage);

//Copy direction, spacing and origin of one image to the other
void CopyDirectionOriginSpacing(DoubleImageType::Pointer inputImage1, DoubleImageType::Pointer inputImage2, DoubleImageType::Pointer & outputImage);

//Skeletonize and calculate voronoi of an image
void SkeletonizeAndShowVoronoiImage(DoubleImageType::Pointer inputImage, DoubleImageType::Pointer & simpleSkeleton, DoubleImageType::Pointer & numberedSkeleton, DoubleImageType::Pointer & voronoi);
#endif
