#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation){
    assert(elemSize > 0);
    assert(initialAllocation >= 0);
    v->alloc_len = initialAllocation;
    if(initialAllocation == 0) v->alloc_len = 2;
    v->elem_size = elemSize;
    v->free_fn = freeFn;
    v->log_len = 0;
    v->elems = malloc(v->alloc_len * v->elem_size);
    assert(v->elems != NULL);
}

void VectorDispose(vector *v){
    if(v->free_fn != NULL){
        for(int i=0; i < v->log_len; i++){
            void* ptr = (char*)v->elems + i*v->elem_size;
            v->free_fn(ptr);
        }
    }
    free(v->elems);
}

int VectorLength(const vector *v){
    assert(v->elems != NULL); 
    return v->log_len;
}

// To Grow the allocated memory
void grow(vector * v){
    v->alloc_len *= 2;
    v->alloc_len++;
    v->elems = realloc(v->elems, v->elem_size * v->alloc_len);
    assert(v->elems != NULL);
}

void *VectorNth(const vector *v, int position){
    assert(position < v->log_len && position >= 0);
    void* ptr = (char*)v->elems + v->elem_size*position;
    return ptr;
}

void VectorReplace(vector *v, const void *elemAddr, int position){
    assert(position < v->log_len && position >= 0);
    void* ptr = (char*)v->elems + position*v->elem_size;
    if(v->free_fn != NULL) v->free_fn(ptr);
    memcpy(ptr,elemAddr,v->elem_size);
}

void VectorInsert(vector *v, const void *elemAddr, int position){
    assert(position <= v->log_len && position >= 0);
    if(v->alloc_len == v->log_len){
        grow(v);
    }
    void* dest = (char*)v->elems + (position+1) * v->elem_size;
    void* from = (char*)v->elems + (position) * v->elem_size;
    if(position != v->log_len){    
        int size = v->log_len - position;
        memmove(dest, from, size*v->elem_size); 
    }
    memcpy(from, elemAddr, v->elem_size);
    v->log_len++;
}

void VectorAppend(vector *v, const void *elemAddr){
    assert(v->elems != NULL);
    if(v->log_len == v->alloc_len) grow(v);
    void* ptr = (char*)v->elems + v->elem_size*v->log_len;
    memcpy(ptr,elemAddr,v->elem_size);
    v->log_len++;
}

void VectorDelete(vector *v, int position){
    assert(position >= 0 && position < v->log_len);
    void* ptr = (char*)v->elems + position*v->elem_size;
    if(v->free_fn != NULL) v->free_fn(ptr);
    if(position != v->log_len - 1){
        memmove(ptr,(char*)ptr + v->elem_size, (v->log_len - position)*v->elem_size);
    }
    v->log_len--;
}

void VectorSort(vector *v, VectorCompareFunction compare){
    assert(compare != NULL);
    qsort(v->elems,v->log_len,v->elem_size,compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData){
    assert(mapFn != NULL);
    for(int i=0 ; i < v->log_len; i++){
        void* ptr = (char*)v->elems + v->elem_size*i;
        mapFn(ptr,auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted){
    assert(startIndex >= 0 && startIndex < v->log_len);
    assert(searchFn != NULL);
    if(isSorted){
        void* from = (char*)v->elems + v->elem_size*startIndex;
        void* ptr = bsearch(key,from,v->log_len-startIndex,v->elem_size,searchFn);
        if(ptr != NULL){
            int pos = ((char*)ptr - (char*)v->elems)/v->elem_size;
            return pos;
        }
    }else{
        for(int i=startIndex; i < v->log_len; i++){
            void* ptr = (char*)v->elems + i*v->elem_size;
            if(searchFn(key,ptr) == 0) return i;
        }
    }
    return kNotFound; 
} 
