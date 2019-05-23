#include "ComputeSkeleton.h"

using namespace std;

void Skeletonize(char *input, char *output)
{
    DoubleImageType::Pointer inputImage, simpleSkeletonImage, numberedSkeletonImage, voronoiImage, inputImageMainComponent;
    ReadImage(input, inputImage);

    //Keep only the largest connected component.
    IntImageType::Pointer inputImageInt;
    CastDoubleImageAsIntImage(inputImage, inputImageInt);

    //This filters finds the connected components.
    typedef itk::ConnectedComponentImageFilter <IntImageType, IntImageType> ConnectedComponentImageFilterType;
    ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
    connected->SetInput(inputImageInt);
    connected->SetFullyConnected(1);
    connected->Update();

    //This filter orders the labels. The biggest object has label 1, the second biggest is number 2 ... 0 is background.
    typedef itk::RelabelComponentImageFilter<IntImageType, DoubleImageType>  RelabelFilterType;
    RelabelFilterType::Pointer relabeler = RelabelFilterType::New();
    relabeler->SetInput(connected->GetOutput());
    relabeler->SetMinimumObjectSize(1000);
    relabeler->Update();

    //Keep only component 1.
    inputImageMainComponent = relabeler->GetOutput();
    itk::ImageRegionIterator<DoubleImageType> imageIt(inputImageMainComponent, inputImageMainComponent->GetLargestPossibleRegion());
    for (imageIt.GoToBegin(); !imageIt.IsAtEnd(); ++imageIt)
        if (imageIt.Get() > 1.0)
            imageIt.Set(0);

    SkeletonizeAndShowVoronoiImage(inputImageMainComponent, simpleSkeletonImage, numberedSkeletonImage, voronoiImage);

    CopyDirectionOriginSpacing(inputImage, simpleSkeletonImage, simpleSkeletonImage);

    WriteImage(output, simpleSkeletonImage);
}

void DisplayHelpSkel()
{
    cout << "Usage:" << endl
         << "ComputeSkeleton -i input binary image -o output" << endl;
}

bool ReadArgumentsSkel(
        int argc, char **argv, char **input, char **output)
{
   int option;

   if (argc < 5)
   {
      DisplayHelpSkel();
      return false;
   }

   while ((option = getopt(argc, argv, "i:o:")) != EOF)
   {
       switch (option)
       {
            case 'i' : *input = optarg;
                break;
            case 'o' : *output = optarg;
                break;
            default  : DisplayHelpSkel();
                       return false;
       }
   }

   return true;
}

int main(int argc, char *argv[])
{

    char *input, *output;

    if (!ReadArgumentsSkel(argc, argv, &input, &output))
    {
       //Bad arguments.
       return 1;
    }

    Skeletonize(input, output);
}

