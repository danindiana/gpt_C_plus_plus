#include <stdio.h>
#include <stdlib.h>

#define MAX_NODES 100

// Structure to represent a tree node
typedef struct TreeNode {
    int data;
    struct TreeNode* firstChild;
    struct TreeNode* nextSibling;
} TreeNode;

// Structure to represent an Euler tour
typedef struct EulerTour {
    int node; // Node data
    int depth; // Depth of the node in the tree
} EulerTour;

// Global variables
EulerTour tour[2 * MAX_NODES]; // Array to store the Euler tour
int tourIndex = 0; // Index for the Euler tour array

// Function to create a new tree node
TreeNode* createNode(int data) {
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    newNode->data = data;
    newNode->firstChild = NULL;
    newNode->nextSibling = NULL;
    return newNode;
}

// Function to add a child to a tree node
void addChild(TreeNode* parent, int childData) {
    TreeNode* child = createNode(childData);
    if (parent->firstChild == NULL) {
        parent->firstChild = child;
    } else {
        TreeNode* sibling = parent->firstChild;
        while (sibling->nextSibling != NULL) {
            sibling = sibling->nextSibling;
        }
        sibling->nextSibling = child;
    }
}

// Function to perform an Euler tour of the tree
void eulerTour(TreeNode* root, int depth) {
    if (root == NULL) return;

    // Record the current node and its depth
    tour[tourIndex].node = root->data;
    tour[tourIndex].depth = depth;
    tourIndex++;

    // Traverse all children
    TreeNode* child = root->firstChild;
    while (child != NULL) {
        eulerTour(child, depth + 1);
        child = child->nextSibling;

        // Record the current node again after returning from a child
        tour[tourIndex].node = root->data;
        tour[tourIndex].depth = depth;
        tourIndex++;
    }
}

// Function to print the Euler tour
void printEulerTour() {
    printf("Euler Tour:\n");
    for (int i = 0; i < tourIndex; i++) {
        printf("Node: %d, Depth: %d\n", tour[i].node, tour[i].depth);
    }
}

// Function to process subtree queries (example: count nodes in a subtree)
int countNodesInSubtree(int start, int end) {
    int count = 0;
    for (int i = start; i <= end; i++) {
        if (tour[i].node == tour[start].node) {
            count++;
        }
    }
    return count;
}

int main() {
    // Create a sample tree
    TreeNode* root = createNode(1);
    addChild(root, 2);
    addChild(root, 3);
    addChild(root->firstChild, 4);
    addChild(root->firstChild, 5);
    addChild(root->firstChild->nextSibling, 6);

    /*
    The tree looks like this:
        1
       /|\
      2 3 6
      / \
     4   5
    */

    // Perform Euler tour starting from the root
    eulerTour(root, 0);

    // Print the Euler tour
    printEulerTour();

    // Example: Count nodes in the subtree rooted at node 2
    int start = 0, end = 0;
    for (int i = 0; i < tourIndex; i++) {
        if (tour[i].node == 2) {
            start = i;
            break;
        }
    }
    for (int i = start; i < tourIndex; i++) {
        if (tour[i].node == 2 && tour[i].depth == tour[start].depth) {
            end = i;
        }
    }
    int subtreeNodeCount = countNodesInSubtree(start, end);
    printf("Number of nodes in the subtree rooted at node 2: %d\n", subtreeNodeCount);

    return 0;
}
