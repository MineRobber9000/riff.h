all: rifftree

rifftree: rifftree.c
	gcc -o $@ $^

clean:
	rm rifftree
