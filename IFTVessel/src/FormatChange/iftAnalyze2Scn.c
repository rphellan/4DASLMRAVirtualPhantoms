#include "ift.h"


int main(int argc, char *argv[]) 
{
  iftImage *image;

  /*--------------------------------------------------------*/

  void *trash = malloc(1);                 
  struct mallinfo info;   
  int MemDinInicial, MemDinFinal;
  free(trash); 
  info = mallinfo();
  MemDinInicial = info.uordblks;

  /*--------------------------------------------------------*/


  if (argc!=3){
    iftError("Usage: iftAnalyze2Scn <analyze file basename (name with no extensions)> <output scene file (*.scn)>","main");
  }

  image = iftAna2Scn(argv[1]);
  iftWriteImage(image, argv[2]);
  iftDestroyImage(&image);

  
  /* ---------------------------------------------------------- */

  info = mallinfo();
  MemDinFinal = info.uordblks;
  if (MemDinInicial!=MemDinFinal)
    printf("\n\nDinamic memory was not completely deallocated (%d, %d)\n",
	   MemDinInicial,MemDinFinal);   

  return(0);
}




