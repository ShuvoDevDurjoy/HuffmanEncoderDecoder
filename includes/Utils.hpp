#pragma once

#include <array>

#include "FileReader.hpp"
#include "Node.hpp"
#include "../Utils/Type.hpp"

struct Compare{
    bool operator()(Node* a, Node* b){
        return a->frequency > b->frequency;
    }
};

namespace Utils{

    int byte_count(int num){
        int count = 0;
        while(num > 0){
            num /= 256;
            count++;
        }
        return count;
    }

    void build_frequency_map(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, uint16_t &num_unique_chars, FileReader* reader){
        // set initial frequency of all byte to 0
        for (size_t i = 0; i < freq.size(); ++i){
            freq[i] = 0;
        }
        //variable to load next byte in
        TYPE_CHAR next_byte;
        //reader reads a file till end to build frequency map
        while(reader->next_byte(next_byte)){
            if(freq[next_byte]==0){
                num_unique_chars++;
            }
            freq[next_byte]++;
        }
    }

    Node* build_huffman_tree(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq)
    {
        std::priority_queue<Node*, std::vector<Node*>, Compare> min_heap;
        //initialize the min heap with leaf nodes
        for (size_t byte = 0; byte < freq.size(); ++byte){
            //check if the frequency of the byte is not 0
            if(freq[byte]){
                Node *node = new Node(byte, freq[byte]);
                min_heap.push(node);
            }
        }
    
        // untill the min heap is of size 1, means only the root node of the tree is left
        //retrieve two smaller frequency nodes and create a new node with frequency equal to their sum
        //and push that into the mean heap
        while(min_heap.size() > 1){
            //get the left node
            Node *left = min_heap.top();
            min_heap.pop();
            //get the right node
            Node *right = min_heap.top();
            min_heap.pop();
    
            //insert a new node of frequency equals sum of left and right node
            min_heap.push(new Node(left->frequency + right->frequency, left, right));
        }
    
        //At this point top value of min heap is the root node of the huffman tree
        Node* root = min_heap.top();
        min_heap.pop();
    
        //return the root node
        return root;
    }

    // performs dfs on the huffman tree until a leaf node is found
    //when a leaf node is found then set the code for the byte
    //it is guaranted that leaf nodes are byte node by initialization of priority queue
    void build_huffman_codes(Node *root, ByteStream &code, std::array<ByteStream, BYTE_SIZE> &huffman_codes)
    {
        //check if root is null or not
        //this is just a guard against malicious input
        if (root == nullptr)
            return;
        if (root->is_leaf())
        {
            huffman_codes[root->c] = code;
            return;
        }
        bool bit;

        //when we will go left then bit 0 will be inserted into code
        //and when we will go right then bit 1 will be inserted into code
        //after going left when we will go right then bit 0 will popped out and 
        //same for after doing right traversal(pop bit 1)

        //go left
        code.push_bit(false);
        build_huffman_codes(root->left, code, huffman_codes);
        //back from left
        code.pop_bit(bit);
        //go right
        code.push_bit(true);
        build_huffman_codes(root->right, code, huffman_codes);
        //back from right
        code.pop_bit(bit);
    }
}