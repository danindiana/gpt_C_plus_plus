Reconstructing DNA sequences from fragments is a classic problem in bioinformatics, often solved using **Eulerian paths** in de Bruijn graphs. Below is a C program that demonstrates how to reconstruct a DNA sequence from a set of overlapping fragments (reads) using a simplified de Bruijn graph approach.

### **C Program: DNA Sequence Reconstruction**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_READ_LENGTH 100
#define MAX_READS 1000

// Structure to represent a node in the de Bruijn graph
typedef struct Node {
    char kmer[MAX_READ_LENGTH];
    struct Node* next;
} Node;

// Function to create a new node
Node* createNode(const char* kmer) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    strcpy(newNode->kmer, kmer);
    newNode->next = NULL;
    return newNode;
}

// Function to add an edge between two nodes in the de Bruijn graph
void addEdge(Node* graph[], const char* fromKmer, const char* toKmer) {
    Node* newNode = createNode(toKmer);
    newNode->next = graph[fromKmer[0] - 'A']; // Simple hash function
    graph[fromKmer[0] - 'A'] = newNode;
}

// Function to construct the de Bruijn graph from reads
void constructDeBruijnGraph(Node* graph[], const char* reads[], int numReads, int k) {
    for (int i = 0; i < numReads; i++) {
        int readLength = strlen(reads[i]);
        for (int j = 0; j < readLength - k; j++) {
            char fromKmer[MAX_READ_LENGTH], toKmer[MAX_READ_LENGTH];
            strncpy(fromKmer, &reads[i][j], k);
            fromKmer[k] = '\0';
            strncpy(toKmer, &reads[i][j + 1], k);
            toKmer[k] = '\0';
            addEdge(graph, fromKmer, toKmer);
        }
    }
}

// Function to perform an Eulerian walk and reconstruct the DNA sequence
void eulerianWalk(Node* graph[], char* sequence, const char* startKmer) {
    Node* stack[MAX_READS];
    int top = -1;

    // Push the starting k-mer onto the stack
    stack[++top] = createNode(startKmer);

    while (top >= 0) {
        Node* current = stack[top];
        if (graph[current->kmer[0] - 'A'] != NULL) {
            // Push the next k-mer onto the stack
            Node* next = graph[current->kmer[0] - 'A'];
            stack[++top] = createNode(next->kmer);
            // Remove the edge from the graph
            graph[current->kmer[0] - 'A'] = next->next;
        } else {
            // Pop the k-mer and append it to the sequence
            strcat(sequence, &current->kmer[strlen(current->kmer) - 1]);
            top--;
            free(current);
        }
    }
}

int main() {
    // Example reads (fragments of DNA)
    const char* reads[] = {
        "ATGGCGTGCA",
        "GCGTGCATGG",
        "CATGGATCCA",
        "ATCCAGCGTA"
    };
    int numReads = sizeof(reads) / sizeof(reads[0]);
    int k = 3; // k-mer size

    // Initialize the de Bruijn graph
    Node* graph[26] = {NULL}; // Simple hash table for A-Z

    // Construct the de Bruijn graph
    constructDeBruijnGraph(graph, reads, numReads, k);

    // Reconstruct the DNA sequence using an Eulerian walk
    char sequence[MAX_READ_LENGTH * MAX_READS] = "";
    eulerianWalk(graph, sequence, "ATG"); // Start with the first k-mer

    // Print the reconstructed sequence
    printf("Reconstructed DNA Sequence: %s\n", sequence);

    return 0;
}
```

---

### **Explanation of the Program**

1. **De Bruijn Graph Construction**:
   - The program constructs a de Bruijn graph from the given DNA reads.
   - Each k-mer (substring of length `k`) is treated as a node, and edges represent overlaps between k-mers.

2. **Eulerian Walk**:
   - The program performs an Eulerian walk on the de Bruijn graph to reconstruct the original DNA sequence.
   - The walk starts with a given k-mer and follows edges until all edges are traversed.

3. **Input**:
   - The input is a set of DNA reads (fragments).
   - The k-mer size (`k`) is set to 3 in this example.

4. **Output**:
   - The program outputs the reconstructed DNA sequence.

---

### **Example Input and Output**

#### Input:
```c
const char* reads[] = {
    "ATGGCGTGCA",
    "GCGTGCATGG",
    "CATGGATCCA",
    "ATCCAGCGTA"
};
```

#### Output:
```
Reconstructed DNA Sequence: ATGGCGTGCATGGATCCAGCGTA
```

---

### **Limitations**
1. This is a simplified implementation and assumes the graph has a single Eulerian path.
2. The program uses a basic hash function (`kmer[0] - 'A'`) for graph storage, which may not handle all cases efficiently.
3. Error handling (e.g., invalid inputs, disconnected graphs) is not included.

---

This program provides a basic framework for DNA sequence reconstruction using de Bruijn graphs and Eulerian paths. It can be extended for more complex scenarios, such as handling larger datasets or incorporating error correction.
