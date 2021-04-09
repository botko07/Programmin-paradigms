#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn){
			assert(numBuckets > 0);
			assert(elemSize > 0);
			assert(hashfn != NULL);
			assert(comparefn != NULL);
			
			h->size = 0;
			h->num_buckets = numBuckets;
			h->hash = hashfn;
			h->freefn = freefn;
			h->cmp = comparefn;
			h->ptr = malloc(numBuckets * sizeof(vector));
			assert(h->ptr != NULL);

			for(int i=0; i < numBuckets; i++){
				VectorNew(h->ptr + i,elemSize,freefn,4);
			}
}

void HashSetDispose(hashset *h){
	for(int i=0 ; i < h->num_buckets; i++){
		vector * ptr = &(h->ptr[i]);
		VectorDispose(ptr);
	}
	free(h->ptr);
}

int HashSetCount(const hashset *h){ 
	assert(h != NULL);
	return h->size;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData){
	assert(h->ptr != NULL);
	assert(mapfn != NULL);
	for(int i=0; i < h->num_buckets; i++){
		vector * current = h->ptr + i;
		VectorMap(current,mapfn,auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr){
	assert(h != NULL);
	int hash = h->hash(elemAddr,h->num_buckets);
	assert(hash >= 0 && hash < h->num_buckets);
	vector * current = &(h->ptr[hash]);
	for(int i=0; i < current->log_len; i++){
		void * ptr = (char*)current->elems + i*current->elem_size;
		if(h->cmp(elemAddr, ptr) == 0){
			VectorReplace(current,elemAddr, i);
			return;
		}
	}
	VectorAppend(current,elemAddr);
	h->size++;
}

void *HashSetLookup(const hashset *h, const void *elemAddr){
	assert(h != NULL);
	int hash = h->hash(elemAddr,h->num_buckets);
	assert(hash >= 0 && hash < h->num_buckets);
	vector* current = h->ptr + hash;
	for(int i=0; i < current->log_len; i++){
		void * ptr = (char*)current->elems + i*current->elem_size;
		if(h->cmp(elemAddr, ptr) == 0){
			return ptr;
		}
	}
	return NULL;
}
