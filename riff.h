// RIFF loader
// Only handles 32-bit RIFF; RF64 support is left as an exercise for the end-user
// Released under CC0

#ifndef _RIFF_H
#define _RIFF_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef char FourCC[4];

// Chunk
struct RIFF_Chunk {
	FourCC type;
	FourCC form;
	uint32_t size;
	union {
		uint8_t *data;
		struct RIFF_ChunkListItem *chunks;
	} contains;
};

// Linked list of RIFF_Chunks
struct RIFF_ChunkListItem {
	struct RIFF_Chunk *chunk;
	struct RIFF_ChunkListItem *next;
};

typedef struct RIFF_Chunk RIFF_Chunk;
typedef struct RIFF_ChunkListItem RIFF_ChunkListItem;

int riff_fourcc_equals(FourCC a, FourCC b);
int riff_is_container(FourCC type);
RIFF_Chunk *riff_parse_chunk_from_file(FILE *fp);
void riff_free_chunk(RIFF_Chunk *chunk);
char *riff_get_error();

#ifdef RIFF_IMPL

static int riff_error = 0;

int riff_fourcc_equals(FourCC a, FourCC b) {
	return ((uint32_t*)a)[0] == ((uint32_t*)b)[0];
}

FourCC _RIFF = {'R','I','F','F'};
FourCC _LIST = {'L','I','S','T'};

int riff_is_container(FourCC type) {
	return riff_fourcc_equals(type,_RIFF) || riff_fourcc_equals(type,_LIST);
}

void riff_copy_fourcc(FourCC from, FourCC to) {
	for (int i=0;i<4;++i) {
		to[i]=from[i];
	}
}

RIFF_Chunk *riff_parse_chunk_from_file(FILE *fp) {
	FourCC type;
	uint8_t _size[4];
	uint32_t size;

	if (fread(type,1,4,fp)!=4) {
		riff_error = 1;
		return NULL;
	}
	if (fread(_size,1,4,fp)!=4) {
		riff_error = 1;
		return NULL;
	}
	RIFF_Chunk *chunk = calloc(1,sizeof(RIFF_Chunk));
	if (chunk == NULL) {
		riff_error = 2;
		return NULL;
	}
	riff_copy_fourcc(type,chunk->type);
	size = (_size[3]<<24)|(_size[2]<<16)|(_size[1]<<8)|_size[0];
	chunk->size = size;
	if (size==0) return chunk;
	if (riff_is_container(type)) {
		long pos = ftell(fp);
		FourCC form;
		if (fread(form,1,4,fp)!=4) {
			free(chunk);
			riff_error = 1;
			return NULL;
		}
		riff_copy_fourcc(form,chunk->form);
		if (size==4) return chunk;
		uint32_t end = pos+size;
		while (1) {
			long cpos = ftell(fp);
			if (cpos>=end) return chunk;
			RIFF_Chunk *newchunk = riff_parse_chunk_from_file(fp);
			// bubble up errors
			if (newchunk == NULL) {
				riff_free_chunk(chunk);
				return NULL;
			}
			RIFF_ChunkListItem *listItem = calloc(1,sizeof(RIFF_ChunkListItem));
			if (listItem == NULL) {
				riff_free_chunk(chunk);
				riff_free_chunk(newchunk);
				free(listItem);
				riff_error = 2;
				return NULL;
			}
			listItem->chunk = newchunk;
			if (chunk->contains.chunks == NULL) {
				chunk->contains.chunks = listItem;
			} else {
				RIFF_ChunkListItem *walker = chunk->contains.chunks;
				while (walker->next!=NULL) walker = walker->next;
				walker->next = listItem;
			}
		}
	} else {
		uint8_t *data = calloc(size,1);
		if (data == NULL) {
			riff_error = 2;
			free(chunk);
			return NULL;
		}
		if (fread(data,1,size,fp)!=size) {
			free(data);
			free(chunk);
			riff_error = 1;
			return NULL;
		}
		chunk->contains.data = data;
		// ensure file is aligned
		if (ftell(fp)&1) fseek(fp,1,SEEK_CUR);
		return chunk;
	}
}

char *riff_get_error() {
	if (riff_error==0) return "No error.";
	if (riff_error==1) return "Unexpected EOF while reading RIFF file.";
	if (riff_error==2) return "Memory allocation failed -- out of memory?";
	return "missingno.";
}

void _riff_free_chunklist(RIFF_ChunkListItem *listItem) {
	if (listItem->next!=NULL) _riff_free_chunklist(listItem->next);
	riff_free_chunk(listItem->chunk);
	free(listItem);
}

void riff_free_chunk(RIFF_Chunk *chunk) {
	if (riff_is_container(chunk->type)) {
		if (chunk->contains.chunks!=NULL) _riff_free_chunklist(chunk->contains.chunks);
	} else {
		free(chunk->contains.data);
		free(chunk);
	}
}

#endif

#endif
