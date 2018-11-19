#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tree_node Node;
struct tree_node {
    int c;
    int freq;
    int key;
    unsigned int code[8];
    Node *left;
    Node *right;
};

typedef struct binary_heap Heap;
struct binary_heap {
    Node *array;
    int size;
};

void downMinHeap(Heap minHeap, int i){
    int k;
    Node temp;
    if(2*i+1<minHeap.size){
        if(minHeap.array[i].key > minHeap.array[k = 2*i+2 >= minHeap.size || minHeap.array[2*i+1].key <= minHeap.array[2*i+2].key ? 2*i+1 : 2*i+2].key){
            temp = minHeap.array[i];
            minHeap.array[i] = minHeap.array[k];
            minHeap.array[k] = temp;
            downMinHeap(minHeap, k);
        }
    }
}

void buildMinHeap(Heap minHeap){
    int i, b;
    for(i=minHeap.size/2-1;i>=0;i--){
        downMinHeap(minHeap, i);
    }
}

Node extractMinHeap(Heap *minHeap){
    Node temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[--minHeap->size];
    downMinHeap(*minHeap, 0);
    return temp;
}

void insertMinHeap(Heap *minHeap, Node x){
    int k, i = minHeap->size++;
    minHeap->array[i] = x;
    while(i>0 && minHeap->array[i].key<minHeap->array[k=(i-1)/2].key){
        minHeap->array[i] = minHeap->array[k];
        minHeap->array[k] = x;
        i = k;
    }
}

void lshift(unsigned int *code){
    int i;
    for(i=0;i<7;i++){
        code[i] <<= 1;
        code[i] |= code[i+1] >> 31;
    }
    code[7] <<=1;
}

int main(int argc, char *argv[]){
    int size, hsize, i, b;
    FILE *fd, *op;
    char fname[256];
    Node *hTree, *cursor, temp;
    Heap heap;
    unsigned int bus[8];
    fd  = fopen(argv[1], "rb");
    fread(&size, sizeof(int), 1, fd);
    fread(&hsize, sizeof(int), 1, fd);
    Node heapArr[heap.size = hsize];
    for(i=0;i<hsize;i++){
        heapArr[i].c = getc(fd);
        fread(&heapArr[i].freq, sizeof(int), 1, fd);
        heapArr[i].key = heapArr[i].freq;
        heapArr[i].left = heapArr[i].right = NULL;
    }
    heap.array = heapArr;
    buildMinHeap(heap);
    temp.c = temp.freq = -1;
    while(heap.size>1){
        temp.left = (Node *)malloc(sizeof(Node));
        *temp.left = extractMinHeap(&heap);
        temp.right = (Node *)malloc(sizeof(Node));
        *temp.right = extractMinHeap(&heap);
        temp.key = temp.left->key+temp.right->key;
        insertMinHeap(&heap, temp);
    }
    hTree = heap.array;
    cursor = hTree;
    strcpy(fname, argv[1]);
    fname[strlen(fname)-4]='\0';
    fname[0]='N';
    op = fopen(fname, "wb");
    while(fread(bus, sizeof(int), 8, fd)/8){
        for(i=0;i<32*8;i++){
            b = bus[0] & 0x80000000;
            lshift(bus);
            if(b==0)
                cursor = cursor->left;
            else
                cursor = cursor->right;
            if(cursor->left==NULL && cursor->right==NULL){
                putc(cursor->c, op);
                cursor = hTree;
                size--;
                if(size==0){
                    fclose(fd);
                    fclose(op);
                    return 0;
                }
            }
        }
    }
    fclose(fd);
    fclose(op);
}
