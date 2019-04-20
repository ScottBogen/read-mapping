/* 

    Layout:

    tree: 
        - root
        - diplay(node* u): display u's children from left to right
        - constructor: sets root node
        - enumerate(node* u): display u's children in DFS traversal (l to r after visiting parent)
        - BWT(char* s): enumerate leaf node's id's from left to right (lexicographically smallest to largest)
            - BWT index is an array B of size n, given by B[i] = s[leaf(i)-1],
              where leaf(i) is the suffix id of the ith leaf in the lexicographical order

    node: 
        - int id 
        - int depth             parent->depth + edge label length 
        - node** children
        - node* parent 
        - node* SL
        - char* label;

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// end results
int matches = 0;
int mismatches = 0;

// direction used for traceback
typedef enum Direction {
    up,
    left,
    diagonal,
    done            // done only applies to t(0,0)
} direction;

typedef struct Cell {
    int S;
    int I;
    int D;              
    int i;              // i index 
    int j;              // j index 
    direction dir;      // traceback direction 
} cell;


int num_nodes = 0;
int max_depth = 0;
int max_length = 0;

int ma = 1;
int mi = -2;

int x = 0;
int m = 100;

int next_index = 0;

int lower_id = 1;   // stores ID of leaves (1 to strlen(S))
int upper_id;       // stores ID of internal nodes

int alphabet_length;
int line_iterator = 1;  // used to ensure 10 li

typedef struct Node {
    int id;
    int depth;          // how deep we are into the string "banana$". 2 = the first 'n'.
    int label_length;   // how much further we go, >= 1. 
                        // 2 = the second 'a'. Thus label = "na";
    struct Node** children;
    struct Node* parent;
    struct Node* SL;

    // for labels
    int start_index;
    int end_index;

    // for testing
    int map_start;
    int map_end;
} node;


typedef struct Tree {
    struct Node* root;
    struct Node* u;
} tree;


tree* init();    // create tree
node* createNode();

void insert(tree* t, char* S, int i);       // insert new node into tree from string S at offset i
                                            // ex: t, "banana$", "3" = insert("nana$")
void display(node* u);
void enumerate(node* n, char* S);
void BWT(tree* t);
void sortChildren(node* n, char* S);
node* findPath(tree* t, node* v, char* S, int offset);
void printNodeInfo(node* temp, char* S);
node* nodeHops(node* n, char* S, char* beta, int offset); 
node* nodeHops2(node* n, char* S, char* beta, int offset);
node* findLocSearch(node* v, char* read, char* S, int read_ptr);
int* findLoc(node* root, char* S, char* read);

int substitution(char a, char b) {
    if (a == b) {
        return ma;
    }
    else { 
        return mi;
    }
}


int findMax(int a, int b, int c) {
    int max = a;
    if (b > max) { max = b; }
    if (c > max) { max = c; }
    return max;
}

// does ctrl+c, ctrl+v count as code reuse? 
int findMaxLocal(int a, int b, int c, int d) {
    int max = a;
    if (b > max) { max = b; }
    if (c > max) { max = c; }
    if (d > max) { max = d; }
    return max;
}

// creates tree, sets root to null, sets SL to self
tree* init() {
    tree* t = (tree*) malloc(sizeof(struct Tree));

    node* r = createNode();
    r->children = NULL;
    r->parent = NULL;
    r->depth = 0;
    r->id = 0;
    r->SL = r;
    r->parent = r;
    r->start_index = -1;
    r->end_index = -1;

    t->root = r;
    t->u = r;

    return t;
}


// creates a node
node* createNode() {
    node* n = (node*) malloc(sizeof(struct Node));
    n->children = NULL;
    n->depth = 0;
    n->id = 0;
    n->label_length = 0;
    n->start_index = -1;
    n->end_index = -1;
    n->parent = NULL;
    n->SL = NULL;
    num_nodes++;
    return n;
}

// test node info (for me)
void printNodeInfo(node* temp, char* S) {
    printf("Node Info:\n");
    printf("\tid:\t\t%d\n", temp->id);
    //printf("\tparent id:\t%d\n", temp->parent->id);

    printf("\tparent id\t");
    if (temp->parent) { printf("%d\n", temp->parent->id); } else { printf("no parent\n"); }

    printf("\tchildren?\t");
    if (temp->children) { printf("yes\n"); } else { printf("no\n"); }

    printf("\tstart index:\t%d\n", temp->start_index);
    printf("\tend index:\t%d\n", temp->end_index);
    printf("\tdepth:\t\t%d\n", temp->depth);

    char output[64];

    for (int i = 0; i < 64; i++) { output[i] = 0; }

    strncpy(output, S+temp->start_index, (temp->end_index - temp->start_index + 1));

    printf("\tlabel:\t\t%s\n", output);
    printf("\tSL?\t\t");
    if (temp->SL) { printf("%d\n", temp->SL->id); } else {printf("no SL\n");} 
    printf("\n");
}

// test function (for me) to print children of node n
void searchTree(node* n, char* S) {
    
    //printf("in node id:%d\n", n->id);

    // search children, l thru r
    for (int i = 0; i < alphabet_length; i++) {
        if (n->children != NULL) {
            if (n->children[i] != NULL) {
                searchTree(n->children[i], S);
            }
        }
    }

    printNodeInfo(n,S);
}

void writeToFile(node* n, char* S) {
    FILE* fp;
    fp = fopen("output.txt", "w");
    searchTreeBST(n, S, fp);
    fclose(fp);
}

void searchTreeBST(node* n, char* S, FILE* fp) {
    
    //printf("in node id:%d\n", n->id);

    // search children, l thru r
    if (n->children != NULL) {
        for (int i = 0; i < alphabet_length; i++) {
            if (n->children[i] != NULL) {
                searchTreeBST(n->children[i], S, fp);
            }
        }
    }

    // when it has returned, print the value of n
    if (n->children == NULL) {
        if (n->id > 1) {
            fprintf(fp, "%c\n", S[n->id-2]);
        }
        else { 
            fprintf(fp, "$\n", n->id);
        }
    } 
}

void display(node* n) {
    printf("%-8d ", n->id);

    if (line_iterator % 10 == 0) { 
        printf("\n"); 
    }
    line_iterator++;

    if (n->children != NULL) {
        for (int j = 0; j < alphabet_length; j++) {
            if (n->children[j] != NULL) {
                display(n->children[j]);
            }
        }
    }
}


// DFS style of printing node depths
void enumerate(node* n, char* S) {
    
    if (n->depth > max_depth && n->children != NULL) {
        max_depth = n->depth;
    }
    
    printf("%-8d ", n->depth);

    if (line_iterator % 10 == 0) { 
        printf("\n"); 
    }
    line_iterator++;

    if (n->children != NULL) {
        for (int j = 0; j < alphabet_length; j++) {
            if (n->children[j] != NULL) {
                enumerate(n->children[j], S);
            }
        }
    }


}


// to maintain lexicographical structure
void sortChildren(node* n, char* S) {
    int i,j;

    int len = 0;
    while(n->children[len] != NULL) { len++; }

    if (len < 2) { return; }    // nothing to sort

    // lazy bubble sort
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            if (i == j) { continue; }
            int a = n->children[i]->start_index;
            int b = n->children[j]->start_index;

            if (S[a] < S[b]) {
                node* temp = n->children[i];
                n->children[i] = n->children[j];
                n->children[j] = temp;
            }
        }
    } 
}


// FindPath algorithm. Offset measures where in S we are. 
node* findPath(tree* t, node* v, char* S, int offset) {
    if(v->children != NULL) {
        for (int i = 0; i < alphabet_length; i++) {     // for each child 
            if (v->children[i] != NULL) {                   // if such a child does exist
                node* child = v->children[i];
                if (S[child->start_index] == S[offset]) {           // if the first index of the label matches with s[offset]

                    int length = child->end_index - child->start_index + 1;

                    if(strncmp(S+child->start_index, S+offset, length) == 0) {        // if they are one in the same, we will have to jump to it 
                        return findPath(t,child, S, offset+length);
                    }
                    else {                                  // if they are not one in the same, we must break here along the edge and create two children.

                        // path does not exhaust label -- we must create new nodes
                        int matches = 0;
                        int j = offset;
                        int k = child->start_index;

                        while (S[j++] == S[k++]) {
                            matches++;      // count # matches on the current label
                        }


                        /*              
                                        What we're doing:
                                              parent
                                                | 
                                            new_parent
                                            /       \
                                        child      new_child
                        */

                        node* new_parent = createNode();
                        new_parent->id = upper_id++; // should be upper id
                        new_parent->start_index = child->start_index;
                        new_parent->end_index = child->start_index + matches - 1;
                        new_parent->parent = v;
                        new_parent->SL = NULL;
                        new_parent->depth = v->depth + (new_parent->end_index - new_parent->start_index + 1);

                        node* new_child = createNode();
                        new_child->id = lower_id++;
                        new_child->start_index = offset+matches;
                        new_child->end_index = offset + matches + strlen(S+offset+matches) - 1;
                        new_child->parent = new_parent;
                        new_child->SL = NULL;
                        new_child->depth = new_parent->depth + (new_child->end_index - new_child->start_index + 1);

                        child->parent = new_parent;
                        child->start_index = child->parent->end_index + 1;
                        child->depth = new_parent->depth + (child->end_index - child->start_index + 1);

                        new_parent->children = (node**) malloc(alphabet_length * sizeof(struct Node));
                        new_parent->children[0] = new_child;
                        new_parent->children[1] = child;

                        sortChildren(new_parent, S);

                        t->u = new_parent;

                        v->children[i] = new_parent;
                        sortChildren(v, S);
                        return new_parent;
                    }
                } 
            }
        }
    }


    // if we reach here, node has no adequate children, so we need to make one
    // this node will be a LEAF
    node* temp = createNode();
    temp->id = lower_id++;
    temp->children = NULL;
    temp->start_index = offset;
    temp->end_index = strlen(S) - 1;    // this end index is correct because the node
                                        // is a LEAF, thus label goes to the end of S
    temp->SL = NULL;
    temp->parent = v;
    temp->children = NULL;
    temp->depth = v->depth + (temp->end_index - temp->start_index + 1);

    // insert temp 
    int i = 0;

    if (v->children != NULL) {
        while (v->children[i]) { 
            i++; 
        };
    } 
    else {
        v->children = (node**) malloc(alphabet_length * sizeof(struct Node));
    }

    v->children[i] = temp;
    t->u = v;

    sortChildren(v, S);       
    return v;    // return the parent of the leaf
}

char* readFile(char* file_name) {
    FILE* fp;
    char* line = NULL;
    size_t len;

    fp = fopen(file_name, "r");

    if (!fp) { printf("FASTA file %s not opened\n", file_name); exit(0); }
    printf("FASTA file %s opened\n", file_name);
    
    int string_length = 0;
    int i = 0;

    while(getline(&line, &len, fp) != -1) {
        if (line[0] != '>') {
            while (line[i] != '\0' && line[i] != '\n') {
                string_length++;
                i++;
            }
            i = 0;
        }
    }
    
    char* S = (char*) malloc(string_length+1);

    rewind(fp);
    
    line = NULL;

    i = 0;
    string_length = 0;

    while(getline(&line, &len, fp) != -1) {
        if (line[0] != '>') {
            while (line[i] != '\0' && line[i] != '\n') {
                S[string_length++] = line[i++];
            }
            i = 0;
        }
    } 
    S[string_length] = '$';

    close(fp);
    return S;
}


void readAlphabetFile(char* file){
    FILE* fp;
    char* line = NULL;
    size_t len;

    fp = fopen(file, "r");

    if (!fp) { printf("Alphabet file %s not opened\n", file); exit(0); }
    printf("Alphabet file %s opened\n", file);

    alphabet_length = 0;
    int i = 0;

    while(getline(&line, &len, fp) != -1) {
        if (line[0] != '>') {
            while (line[i] != '\0' && line[i] != '\n') {
                if (line[i] != ' ') {
                    alphabet_length++;
                }
                i++;
            }
            i = 0;
        }
    }
    alphabet_length++;  // for $, per the way I set it up

    fclose(fp);
}

int* prepareST(tree* t, int n) {
    int* A = (char*) malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        A[i] = -1;  // init content with -1 
    }
    
    DFS_PrepareST(t->root, A);

    for (int i = 0; i < n; i++) {
        printf("A[%d] = %d\n", i, A[i]);
    }

    return A;
}

void DFS_PrepareST(node* n, int A[]){
    if (n == NULL) { return; }  // shouldn't ever happen
    if (!n->children) {         // if n has no children 
        A[next_index] = n->id;   
        if (n->depth >= x) {
            n->map_start = next_index;
            n->map_end = next_index;
        }
        next_index++;
        return;
    }

    // case: T is an internal node
    int i = 0;
    while (n->children[i] != NULL) {
        DFS_PrepareST(n->children[i], A);
        i++;
    }

    if (n->depth >= x) {
        node* u_left = n->children[0];
        node* u_right = n->children[i-1];

        n->map_start = u_left->map_start;
        n->map_end = u_right->map_end;
    }
}

void mapReads(tree* t, char* S, int* A) {
    // suppose at read 1 (r[1...]), we matched 20 characters
    // on read 2 (r[2...]), we should at least match 19 characters

    // for (i = 1 to l) {
        //      start     node*
        //findPath(r[i...], root)
    //}

    // for every starting positin of our read (r_ptr = 0 to L)
    // find the longest path that matches against my input.
    // store this as a legth parameter and save it as the max if it is the max
    int x = 25;
    
    // for all of the reads
    for (int r_i = 0; r_i < 1; r_i++) {
        char* read = "A";
        printf("read = %s\n", read);

        int l = strlen(read);
        int* L_i = findLoc(t->root, S, read);

        // read has been made 
        int start, end;

        start = L_i[0];
        end = L_i[1];

        printf("MapReads: Start = %d\tEnd = %d\n", start, end);

        for (int i = start; i <= end; i++) {
            printf("A[%d] = %d\n", i, A[i]);
        }
        
        printf("Started alignment:\n");
        for (int i = start; i <= end; i++) {
            align(read, A[i]-1, S, l);
        }

        free(L_i);
    }
}

void align(char* read, int j, char* S, int l) {

    printf("Align: j, l = %d, %d\n", j, l);

    // S[start, end]
    int start = j-l;    // (-1 for see below:)
    int end = j+l;
    int i;

    //          j 
    //      S: banana$
    //   L[i]: 123456
    //   S[i]: 0123456

    // fix out of bounds cases 
    if (start < 0) { 
        start = 0; 
    }
    if (end > strlen(S)-1) { 
        end = strlen(S)-1;      // -1 for $ 
    } 
    
    int G_len = end-start+1;
    char* G = (char*) malloc(sizeof(char) * G_len);

    int k = 0;
    for (i = start; i <= end && i < strlen(S)-1; i++) {
        G[k++] = S[i]; 
    }

    printf("align: G = %s\n", G);

    /* params setup:
        0: ma
        1: mi
        2: h
        3: g
    */ 

    int h = -5;
    int g = -1;

    /* 
        create read between read and S[start... end]

        perform SW on it (local alignment)
        calculate matches and alignlen either by forward walk or traceback
    */

    int n = G_len;
    int m = strlen(read);

    char* s1 = read;
    char* s2 = G;

    cell** table = (cell**) malloc(sizeof(cell*) * (n+1));         

    // put cols in table 
    for (i = 0; i <= n; i++) { 
        table[i] = (cell*) malloc(sizeof(cell) * (m+1));            
    }

    // init
    table[0][0].S = 0;
    table[0][0].D = 0;
    table[0][0].I = 0;
    table[0][0].i = 0;
    table[0][0].j = 0;
    table[0][0].dir = done;


    // @ cells (i,0)
    for (i = 1; i <= m; i++) {           // on each col of 0th row
        table[0][i].S = -100000;
        table[0][i].I = -100000;
        table[0][i].D = h + (i * g);
        table[0][i].i = i;
        table[0][i].j = 0;
        table[0][i].dir = left;
    }

    // @ cells (0,j) 
    for (j = 1; j <= n; j++) {           // on each row of 0th col
        table[j][0].S = -100000;
        table[j][0].D = -100000;
        table[j][0].I = h + (j * g);    
        table[j][0].i = 0;
        table[j][0].j = j;
        table[j][0].dir = up;
    }

    cell* temp;
    cell max_cell;

    max_cell.i = max_cell.j = 0;
    max_cell.S = max_cell.I = max_cell.D = -1;       // cell with highest score, used for local

    // forward computation of smith-waterman
    for (i = 1; i <= m; i++) { 
        for (j = 1; j <= n; j++) {
            table[j][i].i = i;
            table[j][i].j = j;

            // local:
            // S:
            temp = &table[j-1][i-1];
            table[j][i].S = findMaxLocal(   temp->S + substitution(s1[i-1], s2[j-1]), 
                                            temp->D + substitution(s1[i-1], s2[j-1]), 
                                            temp->I + substitution(s1[i-1], s2[j-1]),
                                            0   );
            // D:
            temp = &table[j][i-1];
            table[j][i].D = findMaxLocal(temp->S + h + g, temp->D + g, temp->I + h + g, 0);

        
            // I:
            temp = &table[j-1][i];
            table[j][i].I = findMaxLocal(temp->S + h + g, temp->D + h + g, temp->I + g, 0);
        

            // here we use findMax to find the max between S, D, and I
            int max_of_dirs = findMax(table[j][i].S, table[j][i].D, table[j][i].I);


            if (max_of_dirs > findMax(max_cell.D, max_cell.I, max_cell.S)) { 
                max_cell = table[j][i];
            }

            if (max_of_dirs == table[j][i].S) {
                table[j][i].dir = diagonal;
            }  
            else if (max_of_dirs == table[j][i].D) {
                table[j][i].dir = up;
            }
            else {
                table[j][i].dir = left;
            }
        }
    }


    // put cols in table 
    // for (int i = 0; i <= n; i++) { 
    //    table[i] = (cell*) malloc(sizeof(cell) * (m+1));            
    // } 
    

    free(table);
    free(G);
}


// findloc returns all starting positions of the LCS in a read
int* findLoc(node* root, char* S, char* read) {
    int read_ptr;       
    int l = strlen(read);
    int max_length_index = -1;

    // 0: abcde
    // 1:  bcde
    // 2:   cde
    //      ...

    node* deepest_node = NULL;
    for (int i = 0; i < l; i++) {
        read_ptr = 0;
        node* temp = findLocSearch(root, read+i, S, read_ptr);
        if (temp != NULL) {
            deepest_node = temp;
        }
    }

    //printf("findLoc says max_len = %d, deepestNode = %d\n", max_length, deepest_node->id);
    //printf("start=%d, end=%d\n", deepest_node->map_start, deepest_node->map_end);

    int* ids = (int*) malloc(sizeof(int) * 2);

    ids[0] = deepest_node->map_start;
    ids[1] = deepest_node->map_end;

    printf("FindLoc: Deepest node: %d\n", deepest_node->id);
    
    max_length = 0;

    return ids;
}

// recursive read-only findpath used for finding the longest read_ptr of a read compared to the genome. 
node* findLocSearch(node* v, char* read, char* S, int read_ptr) {

    // case A: broke at edge
    if (read_ptr < strlen(read)) {
        for (int i = 0; i < alphabet_length; i++) {     // for each child 
            if (v->children[i] != NULL) {                   // if such a child does exist    
                node* child = v->children[i];
                if (S[child->start_index] == read[read_ptr]) {           // if the first index of the label matches with s[offset]
                    int length = child->end_index - child->start_index + 1;     // get length of the label 
                    if (strncmp(S+child->start_index, read+read_ptr, length) == 0) {        // if they are one in the same, we will have to jump to it 
                        node* temp = findLocSearch(child, read, S, read_ptr+length);     // read_ptr + length of label probably 
                        return temp;
                    }
                    
                    // case B: label breaks between this one and its child, so count the # of matches
                    else { 

                        // let u be the internal node visisted last -- this one!
                        int j = child->start_index;

                        // add to read_ptr the # of matches along the child label 
                        while (S[j] == read[read_ptr]) {
                            read_ptr++;
                            j++;
                        }

                        // allow to fall through to bottom
                    }
                    break;
                } 
            }
        }
    }
    
    // if we've fallen through, we've either failed the child test or we've returned from our recursive window. so return;
    
    // case A: breaks along path: return the last visited node (this one, since we didn't move on)
    // case B: breaks at node: return this one (since we can't take child paths)

    if (read_ptr > max_length) {
        max_length = read_ptr;
        return v;
    }
    else {
        return NULL;
    }
}


//  $ <test executable> <input file containing sequence s> <input alphabet file> 
int main(int argc, char** argv) {

    if (argc < 3) {
        printf("Example functionality: ./a.out <s1.fasta> <alphabet.txt>\n");
        exit(0);
    }

    char* sequence_file = argv[1];
    char* alphabet_file = argv[2];

    char* S = readFile(sequence_file);
    readAlphabetFile(alphabet_file);

    printf("S length = %d\t alphabet length = %d\n", strlen(S), alphabet_length);
    
    upper_id = strlen(S) + 1;       // upper_id is used for internal nodes -- it'll start at 1 + the number of leaves
    printf("upper id = %d\n", upper_id);

    int n = strlen(S);

    // create tree struct
    tree* t = init();

    struct timeval start, stop;

    printf("Press ENTER to build tree\n");
    //getchar();
    printf("\n");
     
    gettimeofday(&start, NULL);
    
    // loop to insert new nodes into tree
    for (int i = 0; i < n; i++) {
        // uncomment the two below lines to see if/when the program breaks   
        printf("--- ITERATION #%d, CHARACTER=%c ---\n", i, S[i]);
        
        findPath(t, t->root, S, i);
    }
    
    gettimeofday(&stop, NULL);
    printf("\n");

    int* A = prepareST(t, n);
     
    printf("\n");
    //writeToFile(t->root, S);

    //searchTree(t->root, S);

    printf("--mapreads:--\n");
    mapReads(t, S, A);
    printf("-- -- -- -- --\n");

    printf("McCreight's Algorithm program finished.\n");
    
    free(t);
    return 0;
}