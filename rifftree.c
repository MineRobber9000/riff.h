// Implementation of rifftree(1)
// Only supports little-endian

#define RIFF_IMPL
#include "riff.h"
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
	if (argc==1) {
		printf("rifftree - parses an arbitrary RIFF file and prints out the RIFF tree structure.\n\n");
		printf("Usage: rifftree [OPTIONS] FILE\n\n");
		printf("Options:\n\n");
		printf("  -v  Print version and exit.\n\n");
		printf("  -s  Print the size of each chunk.\n\n");
		printf("  --flat\n      First chunk of file is not a list (container) chunk.\n\n");
		printf("  --first-chunk-id CKID\n      32 bit chunk ID of very first chunk in file.\n\n");
		printf("  --big-endian\n      File is in big endian format.\n\n");
		printf("  --little-endian\n      File is in little endian format.\n\n");
		return 0;
	}
	char *file = NULL;
	int showSize = 0;
	char *chunkId = NULL;
	int i=1;
	while (i<argc) {
		if (strcmp(argv[i],"-v")==0) {
			printf("rifftree revision 1337\n");
			printf("not using libgig\n");
			return 0;
		} else if (strcmp(argv[i],"-s")==0) {
			showSize = 1;
			++i;
		} else if (strcmp(argv[i],"--flat")==0) {
			// we don't need to do anything; our print_chunk will work just fine
			++i;
		} else if (strcmp(argv[i],"--big-endian")==0) {
			printf("ERROR: we don't support big-endian\n");
			return 1;
		} else if (strcmp(argv[i],"--little-endian")==0) {
			// we do little-endian, so we're good
			++i;
		} else if (strcmp(argv[i],"--first-chunk-id")==0) {
			++i;
			if (i<argc) {
				chunkId = argv[i];
				++i;
			} else {
				chunkId = "asdfg";
			}
		} else {
			file = argv[i];
			++i;
		}
	}
	FourCC ckId;
	if (chunkId!=NULL) {
		if (strlen(chunkId)!=4) {
			printf("Argument after '--first-chunk-id' must be exactly 4 characters long.\n");
			return 1;
		}
		for (int i=0;i<4;++i) ckId[i]=chunkId[i];
	}
	if (file==NULL) {
		printf("You must provide a file argument.\n");
		return 1;
	}
	FILE *fp = fopen(file,"rb");
	if (fp==NULL) {
		printf("Invalid file argument!\n");
		return 1;
	}
	RIFF_Chunk *chunk = riff_parse_chunk_from_file(fp);
	if (chunk==NULL) {
		printf("Error loading RIFF file: %s\n",riff_get_error());
		return 1;
	}
	fclose(fp);
	if (chunkId!=NULL) {
		if (!(riff_fourcc_equals(chunk->type,ckId))) {
			printf("Invalid file header ID (expected '%.4s' but got '%.4s')\n",ckId,chunk->type);
			riff_free_chunk(chunk);
			return 1;
		}
	}
	print_chunk(chunk,0,showSize);
	riff_free_chunk(chunk);
}
