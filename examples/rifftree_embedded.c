// Example of using an embedded RIFF file

#define RIFF_IMPL
#include "../riff.h"
#include "testriff.h"
#include <string.h>

void indent(int num) {
	for (int i=0;i<num;++i) {
		printf("            ");
	}
}

void print_chunk(RIFF_Chunk *chunk, int numindent, int showSize) {
	indent(numindent);
	if (riff_is_container(chunk->type)) {
		printf("%.4s(%.4s)->",chunk->type,chunk->form);
		if (showSize) printf(" (%d Bytes)",chunk->size-4);
		printf("\n");
		RIFF_ChunkListItem *walker = chunk->contains.chunks;
		while (walker!=NULL) {
			print_chunk(walker->chunk,numindent+1,showSize);
			walker = walker->next;
		}
	} else {
		printf("%.4s;",chunk->type);
		if (showSize) printf(" (%d Bytes)",chunk->size);
		printf("\n");
	}
}

int main(int argc, char *argv[]) {
	int showSize = 0;
	int i=1;
	while (i<argc) {
		if (strcmp(argv[i],"-v")==0) {
			printf("rifftree revision 1337\n");
			printf("not using libgig\n");
			return 0;
		} else if (strcmp(argv[i],"-s")==0) {
			showSize = 1;
			++i;
		} else {
			++i;
		}
	}
	size_t offset = 0;
	RIFF_Chunk *chunk = riff_parse_chunk_from_data(testriff_bin,testriff_bin_len,&offset);
	if (chunk==NULL) {
		printf("Error loading RIFF file: %s\n",riff_get_error());
		return 1;
	}
	print_chunk(chunk,0,showSize);
	riff_free_chunk(chunk);
}
