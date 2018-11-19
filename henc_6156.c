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
    int i;
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

void minHeapSort(Heap minHeap){
    int i, j;
    Node temp;
    i = j = minHeap.size;
    while(i-->0){
        minHeap.array[i] = extractMinHeap(&minHeap);
    }
    while(++i<--j){
        temp = minHeap.array[i];
        minHeap.array[i] = minHeap.array[j];
        minHeap.array[j] = temp;
    }
}

Node binarySearch(Node *tab, int l, int r, int c){
    int m = (r-l)/2+l;
    if(tab[m].c==c)
        return tab[m];
    else if(l>=r){
        printf("! search warning\n");
        return tab[m];
    }
    else if(tab[m].c>c)
        return binarySearch(tab, l, m-1, c);
    else
        return binarySearch(tab, m+1, r, c);
}

Node *count(Node *root, int c){
    if(root==NULL){
        //Node temp = {c, 1, 0, NULL, NULL, NULL};
        //root = &temp;
        root = (Node *)malloc(sizeof(Node));
        root->c = c;
        root->freq = 1;
        root->key = 0;
        root->left = NULL;
        root->right = NULL;
    }
    else if(c<root->c)
        root->left = count(root->left, c);
    else if(c>root->c)
        root->right = count(root->right, c);
    else
        root->freq++;
    return root;
}

void lshift(unsigned int *code){
    int i;
    for(i=0;i<7;i++){
        code[i] <<= 1;
        code[i] |= code[i+1] >> 31;
    }
    code[7] <<=1;
}

void rshift(unsigned int *code){
    int i;
    for(i=7;i>0;i--){
        code[i] >>= 1;
        code[i] |= code[i-1] << 31;
    }
    code[0] >>=1;
}

int feed(unsigned int *bus, unsigned int *data, int i, FILE *op){
    int j, k = 255, l = i;
    unsigned int copy[8] = {0};
    for(j=0;j<8;j++)
        copy[j] = data[j];
    while(data[0]>>31==0){
        lshift(data);
        k--;
    }
    lshift(data);
    while(i>0){
        rshift(data);
        i--;
    }
    for(j=0;j<8;j++)
        bus[j] |= data[j];
    l += k;
    //printf("%8x,%8x,%8x,%8x,%8x,%8x,%8x,%8x\n",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
    if(l-256>=0){
        fwrite(bus, sizeof(int), 8, op);
        for(j=0;j<8;j++)
            bus[j] = 0;
        i = 512-l;
        while(i>0){
            lshift(copy);
            i--;
        }
        for(j=0;j<8;j++)
            bus[j] |= copy[j];
        l -= 256;
        printf("%8x,%8x,%8x,%8x,%8x,%8x,%8x,%8x-* r%d\n",copy[0],copy[1],copy[2],copy[3],copy[4],copy[5],copy[6],copy[7],l);
    }
    return l;
}

int ravel(Node *root, Node *array, int i){
    if(root!=NULL){
        i = ravel(root->left, array, i);
        i = ravel(root->right, array, i);
        if(array!=NULL){
            array[i] = *root;
            array[i].key = array[i].freq;
            array[i].left = NULL;
            array[i].right = NULL;
        }
        i++;
    }
    return i;
}

int pick(Node *root, Node *array, unsigned int *code, int i){
    int k;
    if(root!=NULL){
        if(root->left==NULL && root->right==NULL){
            if(array!=NULL){
                array[i] = *root;
                array[i].key = array[i].c;
                array[i].left = NULL;
                array[i].right = NULL;
                for(k=0;k<8;k++)
                    array[i].code[k] = code[k];
            }
            i++;
        }
        else{
            lshift(code);
            i = pick(root->left, array, code, i);
            code[7] |= 1;
            i = pick(root->right, array, code, i);
            rshift(code);
        }
    }
    return i;
}

int main(int argc, char *argv[]){
    int i, c;
    FILE *fd, *op;
    char fname[256];
    Node *bsTree = NULL, *hTree, temp;
    Heap heap, dict;
    unsigned int size=0;
    unsigned int bus[8] = {0}, buffer[8] = {0, 0, 0, 0, 0, 0, 0, 1};

    /*-
     This program is divided into five parts:
     I.   parse the input file and build a Binary Search Tree to store distinct characters and their frequencies of occurrence;
     II.  build a Binary Min Heap to implement a Priority Queue from the tree;
     III. build the Huffman Tree from the heap;
     IV.  implement the encoder with an array of characters with the corresbonding codes sorted by a HeapSort;
     V.   compress and write the file.
    -*/
    
    /* I */
    fd = fopen(argv[1], "rb");
    while((c=getc(fd))!=EOF){
        bsTree = count(bsTree, c);
        size++;
    }
    fclose(fd);
    
    /* II */
    Node heapArr[heap.size = ravel(bsTree, NULL, 0)];
    ravel(bsTree, heapArr, 0);
    strcpy(fname, argv[1]);
    op = fopen(strcat(fname, ".huf"), "wb");
    fwrite(&size, sizeof(int), 1, op);
    fwrite(&heap.size, sizeof(int), 1, op);
    for(i=0;i<heap.size;i++){
        putc(heapArr[i].c, op);
        fwrite(&heapArr[i].freq, sizeof(int), 1, op);
    }
    heap.array = heapArr;
    buildMinHeap(heap);
    
    
    /* III */
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
    
    /* IV */
    Node dictArr[dict.size = pick(hTree, NULL, buffer, 0)];
    pick(hTree, dictArr, buffer, 0);
    dict.array = dictArr;
    buildMinHeap(dict);
    minHeapSort(dict);
    
    /* V */
    fd = fopen(argv[1], "rb");
    i = 0;
    while((c=getc(fd))!=EOF){
        temp = binarySearch(dict.array, 0, dict.size-1, c);
        i = feed(bus, temp.code, i, op);
    }
    fclose(fd);
    if(i>0)
        fwrite(bus, sizeof(int), 8, op);
    fclose(op);
}
