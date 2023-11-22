#include <iostream>
#include <vector>
#include <stack>
#include <random>
#include <limits.h>
#include <time.h>
#define B 6
using namespace std;
class Block;

class Node
{
public:
    int value;
    Block *down; // Pointer to lower level block contains same value
    int opcode;
    int height;
    Node(int value, Block *down, int opcode, int height)
    {
        this->value = value;
        this->down = down;
        this->opcode = opcode;
        this->height = height;
    }
};

class Block
{
public:
    std::vector<Node *> vector;
    Block *next; // Pointer to the next block at the same level
    std::vector<Node *> buffer;
    Block(Node *node, Block *next)
    {
        vector.push_back(node);
        // vector.resize(3); // minimum size of each block
        this->next = next;
    }

    Block(std::vector<Node *> vector, Block *next, std::vector<Node *> buffer)
    {
        this->vector = vector;
        this->buffer = buffer;
        // vector.resize(3); // minimum size of each block
        this->next = next;
    }
};

class BSkipList
{
private:
    std::vector<Block *> levels; // Vector of head blocks from each level
    std::stack<Block *> getBlockStack(int value)
    {

        Block *current = levels[levels.size() - 1]; // starting from first block in higest level
        std::stack<Block *> blocks;                 // store the path
        Block *block = current;                     // keep track the place for value
        Node *prev;
        while (current)
        {
            bool found = false;
            // find a value greater than insert value
            for (unsigned int i = 0; i < current->vector.size(); i++)
            {
                if (value > current->vector[i]->value)
                { // go to next node
                    prev = current->vector[i];
                }
                else
                { // find the place
                    blocks.push((block));
                    current = prev->down;
                    block = current;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                // keep looking in next block
                if (current->next)
                {
                    current = current->next;
                    // last in current block
                    if (value < current->vector[0]->value)
                    {
                        blocks.push(block);
                        current = prev->down;
                    }
                }
                else // last in this level
                    blocks.push(current);
                current = prev->down;
            }
            block = current;
        }
        return blocks;
    }

public:
    int r = 1;

    const int MAX_LEVEL = 32;
    const float P_FACTOR = 0.25;
    static constexpr int P = 4;
    static constexpr int S = 0xFFFF;
    static constexpr int PS = S / 4;


    // static std::random_device rd; // obtain a random number from hardware
    // static std::mt19937 gen(rand()); // seed the generator
    // static std::uniform_real_distribution<> distr(0, 1); // define the range
    BSkipList()
    {
        Block *block = new Block(new Node(INT_MIN, nullptr, 0, -1), nullptr); // negative infinity block
        levels.push_back(block);
    }

    ~BSkipList()
    {
        // Destructor to free memory
        // ... (cleanup logic here)
    }
    int randomLevel() {
        int lv = 1;
        
        while (lv < MAX_LEVEL && (rand() & S) < PS) ++lv;
        return lv;
    }

    void upsert(int value, int opcode,int lv)
    {
        
        //levels[levels.size()-1]->buffer.push_back(new Node(value, nullptr, opcode, lv));
        //如果层数比当前最高lvl高的情况
        if(lv >= levels.size()){
            //向上
            levels[levels.size()-1]->next = new Block(new Node(value, nullptr, 0, lv), nullptr);
            for(int i = levels.size(); i <= lv; i++){
                
                if(i==lv){
                    levels.push_back(new Block(new Node(INT_MIN, nullptr, 0, -1), nullptr));
                    levels[i]->vector.push_back(new Node(value, nullptr, 0, lv));
                }else{
                    levels.push_back(new Block(new Node(INT_MIN, nullptr, 0, -1), nullptr));
                    levels[i]->next = new Block(new Node(value, nullptr, 0, lv), nullptr);

                }
            }
            //向下
            int sz = levels[levels.size()-1]->vector.size();
            Node* cur = levels[levels.size()-1]->vector[sz-1];
            while(cur->down){
                
            }
        }else{
            levels[levels.size()-1]->buffer.push_back(new Node(value, nullptr, opcode, lv));
        }

        
        
        
        

        // for (int i = levels.size() - 1; i >= 0; i--)
        // {
        //     Block* curr = levels[i];
        // }
    }

    void insert(int value)
    {
        // upsert(value, 0);
        //  srand(time(NULL)); // initialize random seed
        //  std::stack<Block *> blocks = getBlockStack(value);
        //  Block *lower = nullptr;
        //  // building block from botton
        //  while (!blocks.empty())
        //  {
        //      bool inserted = false;
        //      Block *block = blocks.top();
        //      blocks.pop();
        //      for (unsigned int i = 0; i < block->vector.size(); i++)
        //      {
        //          if (block->vector[i]->value > value)
        //          { // in the middle of the vector
        //              // (static_cast<float>(rand()) / RAND_MAX) < P_FACTOR)
        //              if (r%2==0)
        //              { // tail
        //                  //r = r + rand();
        //                  block->vector.insert(block->vector.begin() + i, new Node(value, lower));
        //                  return;
        //              }
        //              else
        //              { // head
        //                  r++;
        //                  // split and shrink block
        //                  std::vector<Node *> right;
        //                  std::vector<Node *> buffer;
        //                  right.push_back(new Node(value, lower));
        //                  for (unsigned int j = i; j < block->vector.size(); j++)
        //                      right.push_back(block->vector[j]);
        //                  block->vector.resize(i);
        //                  Block *rightBlock = new Block(right, block->next, buffer);
        //                  block->next = rightBlock;
        //                  // new level
        //                  if (blocks.empty())
        //                  {
        //                      Block *up = new Block(new Node(INT_MIN, block), nullptr);
        //                      up->vector.push_back(new Node(value, block->next));
        //                      levels.push_back(up);
        //                  }
        //                  inserted = true;
        //                  lower = block->next;
        //                  break;
        //              }
        //          }
        //      }
        //      if (!inserted)
        //      {
        //          // at the end of the vector
        //          if (r%2==0)
        //          { // tail
        //              r = r + 1;
        //              block->vector.push_back(new Node(value, lower));
        //              return;
        //          }
        //          else
        //          { // head
        //              r = r + rand();
        //              Block *newBlock = new Block(new Node(value, lower), block->next);
        //              block->next = newBlock;
        //              // new level
        //              if (blocks.empty())
        //              {
        //                  Block *up = new Block(new Node(INT_MIN, block), nullptr);
        //                  up->vector.push_back(new Node(value, newBlock));
        //                  levels.push_back(up);
        //              }
        //              lower = newBlock;
        //          }
        //      }
        //  }
    }

    void remove(int value)
    {
        std::stack<Block *> blocks = getBlockStack(value);
        Block *current;
        Block *block;
        vector<Block *> update;
        Block *curr = nullptr;
        bool flag = false;
        for (int i = levels.size() - 1; i >= 0; i--)
        {
            Block *pre = nullptr;
            curr = levels[i];
            while (curr)
            {
                for (int j = 0; j < curr->vector.size(); j++)
                {
                    if (curr->vector[j]->value == value)
                    {
                        if (pre)
                        {
                            flag = true;
                            update.push_back(pre);
                            // cout << pre->vector[0]->value << "pre" << endl;
                        }
                        break;
                    }
                }
                if (flag)
                {
                    flag = false;
                    break;
                }

                pre = curr;
                curr = curr->next;
            }
        }

        // for (int i = 0; i < update.size(); i++)
        // {
        //     cout << update[i]->vector[0]->value << "update" << endl;
        //     if (update[i]->next)
        //     {
        //         cout << update[i]->next->vector[0]->value << "update next" << endl;
        //         if(update[i]->next->vector.size() > 1){
        //             cout << update[i]->next->vector[1]->value << "test" << endl;
        //         }
        //     }
        // }
        int x = 0;
        while (!blocks.empty())
        {
            block = blocks.top();
            blocks.pop();

            for (unsigned int i = 0; i < block->vector.size(); i++)
            {
                if (block->vector[i]->value == value)
                {
                    Block *downBlock = block->vector[i]->down;
                    block->vector.erase(block->vector.begin() + i);

                    while (downBlock != nullptr)
                    {
                        current = downBlock->vector[0]->down;
                        downBlock->vector.erase(downBlock->vector.begin());
                        if (!downBlock->vector.empty())
                        {
                            update[x]->vector.insert(update[x]->vector.end(), downBlock->vector.begin(), downBlock->vector.end());
                            update[x]->next = update[x]->next->next;
                            x++;
                        }
                        else
                        {
                            update[x]->next = update[x]->next->next;
                            x++;
                        }

                        downBlock = current;
                    }
                }
            }
        }
    }

    void print_list()
    {
        Block *curr;
        for (int i = levels.size() - 1; i >= 0; i--)
        {
            Block *pre = nullptr;
            curr = levels[i];
            while (curr)
            {
                for (int j = 0; j < curr->vector.size(); j++)
                {

                    cout << curr->vector[j]->value << " ";
                    if (curr->vector[j]->down)
                    {
                        cout << "(" << curr->vector[j]->down->vector[0]->value << ")";
                    }
                    
                }

                for(int k = 0; k < curr->buffer.size();k++){
                    cout << "["<<curr->buffer[k]->value << "]";
                }
                curr = curr->next;
                cout << "|";
            }
            cout << "" << endl;
        }
    }

    bool search(int key)
    {
        std::vector<Node *>::iterator it;
        Node *node;
        Node *prev_node;
        Block *block = levels[levels.size() - 1];

        while (block)
        {
            for (it = block->vector.begin(); it != block->vector.end(); ++it)
            {
                node = *it;
                if (node->value < key)
                {
                    prev_node = node;
                    if (node == *std::prev(block->vector.end()))
                    {
                        block = block->next;
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (node->value == key)
                {
                    return true;
                }
                else if (key < node->value)
                {
                    block = prev_node->down;
                    break;
                }
                // else if (i == 0) {return false;}
            }
        }
        // }
        return false;
    }

    std::vector<bool> range_query(int start_key, int end_key)
    {
        std::vector<bool> output;
        for (int key = start_key; key < end_key; key++)
        {
            int value = search(key);
            if (value != -1)
            {
                output.push_back(value);
            }
        }
        return output;
    }
};

int main()
{
    BSkipList list;
    list.upsert(1,0,3);
    list.upsert(2,0,3);
    // list.insert(10);
    // list.insert(3);
    // list.insert(2);
    // list.insert(6);
    // list.insert(11);
    // list.insert(7);
    // list.print_list();
    // list.remove(7);
    
    // list.insert(8);
    // list.insert(-1);
    // std::cout << list.search(-1) << std::endl;
    // std::cout << list.search(-2) << std::endl;
    // std::cout << list.search(11) << std::endl;
    list.print_list();
    return 0;
}