#include "regex_tree.hpp"

/*
 * Based on Huffman Tree
 * Used to decode encoded regex in binary file
 */

using namespace std;

node::node(char val, node* l, node* r): value(val), left(l), right(r) { }

node::node(char val): value(val) {
    left = NULL;
    right = NULL;
}

RegexTree::RegexTree(){
    root = new node('_');
}

RegexTree::~RegexTree(){
    destroy_tree();
}

void RegexTree::destroy_tree(node *leaf){
    if(leaf != NULL){
        destroy_tree(leaf->left);
        destroy_tree(leaf->right); delete leaf;
    }
}

void RegexTree::destroy_tree(){
    destroy_tree(root);
}

void RegexTree::insert(char value, node* leaf, int pos){
    if(leaf != NULL){
        if(pos == 0)
            leaf->left = new node(value);
        else
            leaf->right = new node(value);
    }
}

node* RegexTree::get_root(){
    return root;
}

string RegexTree::decode(string encoded){
    stringstream ss;    

    node* cur_node = root;
    for(size_t i = 0; i < encoded.length(); i++){

        if(cur_node->left == NULL && cur_node->right == NULL)
            return ss.str(); 

        if(encoded[i] == '0')
            cur_node = cur_node->left;
        else
            cur_node = cur_node->right;

        if(cur_node->value != '_'){
            ss << cur_node->value;
            cur_node = root;
        }

    }

    return ss.str();
}


void regexTree::construct_tree(RegexTree& tree){

    tree.insert('_', tree.get_root(), 0); 
    tree.insert('_', tree.get_root(), 1); 

    // Process left half of the tree
    node *cur_node = tree.get_root()->left;

    cur_node->left = new node('_');
    cur_node->left->left = new node('2');
    cur_node->left->right = new node('1');

    cur_node->right = new node('_');
    cur_node->right->left = new node('+');

    cur_node = cur_node->right;
    cur_node->right = new node('_');

    cur_node = cur_node->right;
    cur_node->left = new node('6');
    cur_node->right = new node('_');

    cur_node = cur_node->right;
    cur_node->left = new node('9');
    cur_node->right = new node('_');

    cur_node = cur_node->right;
    cur_node->right = new node('0');
    cur_node->left = new node('_');

    cur_node = cur_node->left;
    cur_node->left = new node('(');
    cur_node->right = new node(')');

    //Process right half of the tree
    cur_node = tree.get_root()->right;

    cur_node->left = new node('_');
    cur_node->right = new node('_');

    cur_node->left->left = new node('*');
    cur_node->left->right = new node('a');

    cur_node = cur_node->right;
    cur_node->left = new node('_');
    cur_node->right = new node('_');

    cur_node->left->left = new node('5');
    cur_node->left->right = new node('4');

    cur_node = cur_node->right;
    cur_node->right = new node('3');
    cur_node->left = new node('_');

    cur_node = cur_node->left;
    cur_node->right = new node('8');
    cur_node->left = new node('7');

}
