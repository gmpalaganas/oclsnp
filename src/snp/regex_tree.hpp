#include <sstream>

struct node{
    char value;
    node *left;
    node *right;

    node(char val, node* l, node* r);
    node(char val);

    bool is_leaf();

};

class RegexTree{
    public:
        RegexTree();
        ~RegexTree();

        void insert(char val, node* leaf, int pos);
        std::string decode(std::string encoded);
        void destroy_tree();
        node* get_root();

    private:
        void destroy_tree(node* leaf);
        node *root;
};


namespace regexTree{
    void construct_tree(RegexTree& tree);
};
