#ifndef IFT_FSET_H_
#define IFT_FSET_H_

#include "iftCommon.h"

typedef struct ift_fset {
  float elem;
  struct ift_fset *next;
} iftFSet;

void    iftInsertFSet(iftFSet **S, float elem);
float   iftRemoveFSet(iftFSet **S);
void    iftRemoveFSetElem(iftFSet **S, float elem);
void    iftDestroyFSet(iftFSet **S);

#endif

