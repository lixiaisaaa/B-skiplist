#include <iostream>
#include <vector>
#include <stack>
#include <random>
#include <time.h> 
#include <limits.h>

class Block;

class Node {
public:
    int value;
    Block* down; // Pointer to lower level block contains same value
    Node(int value, Block* down){
        this->value= value;
        this->down = down;
    }
};

class Block {
public:
    std::vector<Node*> vector; 
    Block* next; // Pointer to the next block at the same level
    Block( Node* node, Block* next){
        vector.push_back(node);
        //vector.resize(3); // minimum size of each block
        this->next = next;
    }

    Block(std::vector<Node*> vector, Block* next){
    this->vector = vector;
    //vector.resize(3); // minimum size of each block
    this->next = next;
    }

    void print(){
        for (unsigned int i = 0; i < vector.size(); i++){
            // if(vector[i])
            //     std::cout << vector[i]->value << " ";
            // else
            //     std::cout << "Null ";
            std::cout << vector[i]->value;
            if(vector[i]->down)
                std::cout<<"("<<vector[i]->down->vector[0]->value <<")";
            std::cout<< " ";
        }
        std::cout << "|";
    }
};

class BSkipList {
private:
    std::vector<Block*> levels; // Vector of head blocks from each level
public:
    int r = 2;
    BSkipList() {
        Block* block = new Block(new Node(INT_MIN,nullptr), nullptr); // negative infinity block
        levels.push_back(block);
    }

    ~BSkipList() {
        // Destructor to free memory
        // ... (cleanup logic here)
    }

    void insert(int value) {
        srand(time(NULL)); // initialize random seed
        Block* current = levels[levels.size()-1];  // starting from first block in higest level
        std::stack<Block*> blocks;      // place for value at each level
        Block* block;  // keep track the place for value
        Node* prev;
        while(current){
            bool found = false;
            // find a value greater than insert value
            for(unsigned int i = 0; i < current->vector.size(); i++) {
                if(value > current->vector[i]->value){ // go to next node
                    prev = current->vector[i];
                    block = current;
                }
                else{    // find the place
                    blocks.push(block);
                    current = prev->down;
                    found = true;
                    break;
                }
            }
            if(!found){
                // keep looking in next block
                if(current->next){
                    current = current->next;
                    // last in current block
                    if(value < current->vector[0]->value){
                        blocks.push(block);
                        current = prev->down;
                    }
                }
                else // last in this level
                    blocks.push(current);
                    current = prev->down;
            }
        }
        // building block from botton
        block = blocks.top();
        blocks.pop();
        for(unsigned int i = 0; i < block->vector.size(); i++) {
            if(block->vector[i]->value > value){ // in the middle of the vector
                if(r % 2 == 0){ // tail
                    r++;
                    block->vector.insert(block->vector.begin() + i, new Node(value,nullptr));
                }
                else{ // head
                    r++;
                    std::vector<Node*> right;
                    // split and shrink block
                    for(unsigned int j = i; j < block->vector.size(); j++)
                        right.push_back(block->vector[j]);
                    block->vector.resize(i);
                    Block* rightBlock = new Block(right,nullptr);
                    block->next = new Block(new Node(value,nullptr),rightBlock);
                    // new level
                    if(blocks.empty()){
                        Block* up = new Block(new Node(INT_MIN,block),nullptr);
                        up->vector.push_back(new Node(value,block->next));
                        levels.push_back(up);
                    }
                }
                return;
            }
        }
        // at the end of the vector
        if(r % 2 == 0){ // tail
            r++;
            block->vector.push_back(new Node(value,nullptr));
        }
        else{ //head
            r++;
            Block* newBlock = new Block(new Node(value,nullptr),block->next);
            block->next = newBlock;
            // new level
            if(blocks.empty()){
                Block* up = new Block(new Node(INT_MIN,block),nullptr);
                up->vector.push_back(new Node(value,newBlock));
                levels.push_back(up);
            }
            
        }
    }

    void print(){
        for(unsigned int i = 0; i <levels.size() ; i++) {
            Block* current = levels[i];
            while(current){
                current->print();
                current = current->next;
            }
            std::cout << std::endl;
        }
    }

    bool search(int key) {
        std::vector<Node*>::iterator it;
        Node* node;
        Node* prev_node;

        // for (int i = levels.size() - 1; i >= 0; i--) { 
        // Block* block = levels[i];
        Block* block = levels[levels.size() - 1];
        while(block) {
            for (it = block->vector.begin(); it != block->vector.end(); ++it) {
                node = *it;
                if(node->value < key){
                    prev_node = node; 
                    if(node==*std::prev(block->vector.end())){
                        block = block->next;
                        break;
                    }
                    else{continue;}
                }
                else if(node->value == key) {return true;}
                else if (key < node->value){
                    block = prev_node->down; 
                    break;
                } 
                // else if (i == 0) {return false;}
            }
        }
        // }
        return false;
    }

    // std::vector<bool> range_query(int _start_key, int _end_key) {
    //     int start_key = _start_key;
    //     int end_key = _end_key;

    //     std::vector<Node*>::iterator it;
    //     std::vector<bool> output;
    //     bool value = false;
        
    //     Node* node;
    //     Node* prev_node;

    //     //first find the start key value
    //     Block* block = levels[levels.size() - 1];

    //     while(true){
    //         while(block) {
    //             if(value){break;}
                
    //             for (it = block->vector.begin(); it != block->vector.end(); ++it) {
    //                 node = *it;
    //                 if(node->value < start_key){
    //                     prev_node = node; 
    //                     if(node==*std::prev(block->vector.end())){
    //                         block = block->next;
    //                         break;
    //                     }
    //                     else{continue;}
    //                 }
    //                 else if(node->value == start_key) { 
    //                     value = true;
    //                     break;
    //                 }
    //                 else if (start_key < node->value){
    //                     block = prev_node->down; 
    //                     break;
    //                 } 
    //             }
    //         }
    //         output.push_back(value);
    //         start_key+=1;
            
    //         if(value){++it; break;}
    //         //prevent when the first key is not found
    //         //if the first key is not found set the next key is first key
    //         else if(start_key == end_key){break;}
    //     }

    //     //propagates next node until the key is below than end_key
    //     //if there is no more blocks the break the loop.
    //     int cur_key = start_key;

    //     while(block){ 
    //         if(cur_key >= end_key) break;

    //         if(it == block->vector.end()) {
    //             block = block->next;
    //             it = block->vector.begin();
    //         }

    //         node = *it;
    //         if(node->value == cur_key){
    //             value = true;
    //             ++cur_key;
    //             ++it;
    //         }
    //         else if(node->value > cur_key){
    //             value = false;
    //             ++cur_key;
    //         }

    //         else{
    //             value = false;
    //             ++it;
    //         }

    //         output.push_back(value);
    //     }
    //     return output;
    // }

    std::vector<bool> range_query(int start_key, int end_key) {
        std::vector<bool> output;
        for (int key = start_key; key < end_key; key++) {
            int value = search(key);
            if (value != -1) {
                output.push_back(value);
            }
        }
        return output;
    }
};


void test_search(BSkipList list){
    //Test Search
    std::cout << "==========================" << std::endl;
    std::cout << "Test for search" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << "Search 1: " << std::boolalpha << list.search(1) << std::endl;
    std::cout << "Search 3: " << std::boolalpha << list.search(3) << std::endl;
    std::cout << "Search 4: " << std::boolalpha << list.search(4) << std::endl;
    std::cout << "Search 10: " << std::boolalpha << list.search(10) << std::endl;
    std::cout << "Search 5: " << std::boolalpha << list.search(5) << std::endl;
    std::cout << "Search 14: " << std::boolalpha << list.search(5) << std::endl;
    std::cout << "Search 2: " << std::boolalpha << list.search(5) << std::endl;
}

void test_range_query(BSkipList list, int start, int end){    
    //Test Range Query
    std::vector<bool> rq_output = list.range_query(start, end);
    std::vector<bool>::iterator it;
    int i;

    std::cout << "==========================" << std::endl;
    std::cout << "Test for range search" << std::endl;
    std::cout << "==========================" << std::endl;

    for(it=rq_output.begin(), i = start; it!=rq_output.end() && i < end; it++, i++){
        std::cout << "Search " << i << ": " << std::boolalpha << *it << std::endl;
    }
}

int main() {
    BSkipList list;
    list.insert(1);
    list.insert(10);
    list.insert(3);
    list.insert(4);
    list.print();

    //Test Search
    test_search(list);
    //Test Range Search
    test_range_query(list, 1, 12);
    // test_range_query(list, 3, 11);
    // test_range_query(list, 5, 15);
    
    return 0;
}
