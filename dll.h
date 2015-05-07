#ifndef __DLL_H
#define __DLL_H

int dllAppend (void * list, void * element);
int dllPrepend (void * list, void * element);
int dllInsert (void * list, void * after, void * element);
int dllRemove (void * list, void * element);
int dllRemoveFirst (void * list);
int dllRemoveLast (void * list);

#endif
