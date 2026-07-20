#include "../includes/Huffman.hpp"

bool Huffman::build_frequency_map()
{
    for (size_t i = 0; i < frequency.size(); ++i){
        frequency[i] = 0;
    }
    TYPE_CHAR next_byte;
    while(reader->next_byte(next_byte)){
        if(frequency[next_byte]==0){
            num_unique_chars++;
        }
        frequency[next_byte]++;
    }
    return true;
}

Node* Huffman::build_tree()
{
    for (size_t byte = 0; byte < frequency.size(); ++byte){
        if(frequency[byte]){
            Node *node = new Node(byte, frequency[byte]);
            min_heap.push(node);
        }
    }

    while(min_heap.size() > 1){
        Node *left = min_heap.top();
        min_heap.pop();
        Node *right = min_heap.top();
        min_heap.pop();

        min_heap.push(new Node(left->frequency + right->frequency, left, right));
    }

    Node* root = min_heap.top();
    min_heap.pop();

    return root;
}

void Huffman::build_huffman_codes(Node *root, ByteStream &code)
{
    if (root == nullptr)
        return;
    if (root->is_leaf())
    {
        huffman_codes[root->c] = code;
        return;
    }
    bool bit;
    code.push_bit(false);
    build_huffman_codes(root->left, code);
    code.pop_bit(bit);
    code.push_bit(true);
    build_huffman_codes(root->right, code);
    code.pop_bit(bit);
}


void Huffman::_encode(std::string in_file_name, std::string out_file_name)
{
    std::cout << "\n*****Starting Encoding*****\n" << std::endl;
    // check if the building frequency map is successful or not
    // if this is not successful then return immediately
    bool success = build_frequency_map();

    // build the huffman tree and get the root of the tree
    Node *root = build_tree();
    ByteStream code;
    build_huffman_codes(root, code);
    // show_frequency_map();

    enc->encode(in_file_name, out_file_name, frequency, huffman_codes, num_unique_chars);
}

void Huffman::encode(std::string in_file_name, std::string out_file_name){
    if(reader->open(in_file_name) && writer->open(out_file_name)){
        this->_encode(in_file_name, out_file_name);
    }
}