#include <iostream>
#include <vector>
#include <stack>
#include <random>
#include <limits.h>
#include <time.h>
#include <chrono>
#include <numeric>   // For std::accumulate
#include <algorithm> // For std::min_element and std::max_element
using namespace std;
class Block;

class Node
{
public:
    int value;
    Block *down; // Pointer to lower level block contains same value
    Node(int value, Block *down)
    {
        this->value = value;
        this->down = down;
    }
};

class Block
{
public:
    std::vector<Node *> vector;
    Block *next; // Pointer to the next block at the same level
    Block(Node *node, Block *next)
    {
        vector.push_back(node);
        // vector.resize(3); // minimum size of each block
        this->next = next;
    }

    Block(std::vector<Node *> vector, Block *next)
    {
        this->vector = vector;
        // vector.resize(3); // minimum size of each block
        this->next = next;
    }

    void print()
    {
        for (unsigned int i = 0; i < vector.size(); i++)
        {
            std::cout << vector[i]->value;
            if (vector[i]->down)
                std::cout << "(" << vector[i]->down->vector[0]->value << ")";
            std::cout << " ";
        }
        std::cout << "| ";
    }
};

class BSkipList
{
private:
    std::vector<Block *> levels; // Vector of head blocks from each level
    std::stack<Block *> getBlockStack(int value)
    {
        int lvl = levels.size() - 1;
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
                    lvl--;
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
    const float P_FACTOR = 0.25;
    // const int MAX_LEVEL = 32;
    // const float P_FACTOR = 0.25;
    // static std::random_device rd; // obtain a random number from hardware
    // static std::mt19937 gen(rand()); // seed the generator
    // static std::uniform_real_distribution<> distr(0, 1); // define the range
    BSkipList()
    {
        Block *block = new Block(new Node(INT_MIN, nullptr), nullptr); // negative infinity block
        levels.push_back(block);
    }

    ~BSkipList()
    {
        // Destructor to free memory
        // ... (cleanup logic here)
    }

    void insert(int value)
    {
        srand(time(NULL)); // initialize random seed
        std::stack<Block *> blocks = getBlockStack(value);
        Block *lower = nullptr;
        // building block from botton
        while (!blocks.empty())
        {
            bool inserted = false;
            Block *block = blocks.top();
            blocks.pop();
            for (unsigned int i = 0; i < block->vector.size(); i++)
            {
                if (block->vector[i]->value > value)
                { // in the middle of the vector
                    // (static_cast<float>(rand()) / RAND_MAX) < P_FACTOR)
                    if (r % 2 != 0)
                    { // tail
                        r = r + rand();
                        block->vector.insert(block->vector.begin() + i, new Node(value, lower));
                        return;
                    }
                    else
                    { // head
                        r++;
                        // split and shrink block
                        std::vector<Node *> right;
                        right.push_back(new Node(value, lower));
                        for (unsigned int j = i; j < block->vector.size(); j++)
                            right.push_back(block->vector[j]);
                        block->vector.resize(i);
                        Block *rightBlock = new Block(right, block->next);
                        block->next = rightBlock;
                        // new level
                        if (blocks.empty())
                        {
                            Block *up = new Block(new Node(INT_MIN, block), nullptr);
                            up->vector.push_back(new Node(value, block->next));
                            levels.push_back(up);
                        }
                        inserted = true;
                        lower = block->next;
                        break;
                    }
                }
            }
            if (!inserted)
            {
                // at the end of the vector
                if (r % 2 != 0)
                { // tail
                    r = r + 1;
                    block->vector.push_back(new Node(value, lower));
                    return;
                }
                else
                { // head
                    r = r + rand();
                    Block *newBlock = new Block(new Node(value, lower), block->next);
                    block->next = newBlock;
                    // new level
                    if (blocks.empty())
                    {
                        Block *up = new Block(new Node(INT_MIN, block), nullptr);
                        up->vector.push_back(new Node(value, newBlock));
                        levels.push_back(up);
                    }
                    lower = newBlock;
                }
            }
        }
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
                curr = curr->next;
                cout << "|";
            }
            cout << " " << endl;
        }
    }

    std::vector<int> getAverageSize()
    {
        Block *curr;
        vector<int> sizes;
        for (int i = levels.size() - 1; i >= 0; i--)
        {
            Block *pre = nullptr;
            curr = levels[i];
            while (curr)
            {
                sizes.push_back(curr->vector.size());
                curr = curr->next;
            }
        }
        return sizes;
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

    int getHeight(){
        return levels.size();
    }
};

int main()
{
    BSkipList list;
    for (int i = 0; i < 100000; ++i)
    {
        list.insert(i);
    }
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i)
    {
        list.search(i);
    }

    // Stop timing
    auto end = std::chrono::high_resolution_clock::now();
    // Calculate duration
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken to add " << 1000000 << " elements: "
              << duration.count() << " seconds." << std::endl;

    // vector<int> vec = list.getAverageSize();

    // double average = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
    // int maxElement = *std::max_element(vec.begin(), vec.end());
    // int minElement = *std::min_element(vec.begin(), vec.end());

    // std::cout << "Average: " << average << std::endl;
    // std::cout << "Max: " << maxElement << std::endl;
    // std::cout << "Min: " << minElement << std::endl;
    // std::cout << "Height: " << list.getHeight() << std::endl;
    return 0;
}
