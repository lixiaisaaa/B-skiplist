#include <iostream>
#include <vector>
#include <limits.h>
#include <random>
#include <cstdlib>
#include <time.h>
#include <unordered_map>
#include <chrono>
#include <math.h>
#include <thread>
#define B 6
using namespace std;
class Block;

class Node
{
public:
    int value;
    Block *down; // Pointer to lower level block contains same value
    int height;
    int opcode;
    Node(int value, Block *down, int height, int opcode)
    {
        this->value = value;
        this->down = down;
        this->height = height;
        this->opcode = opcode;
    }
};

class Block
{
public:
    std::vector<Node *> pivots;
    std::vector<Node *> buffer;
    Block *next; // Pointer to the next block at the same level
    int height;
    int numberOfDeletedNode;

    Block(Node *node, Block *next, int height)
    {
        // vector.resize(3); // minimum size of each block
        this->pivots.push_back(node);
        this->next = next;
        this->height = height;
        this->numberOfDeletedNode = 0;
    }

    void print()
    {
        Block *hasNext = this;
        while (hasNext)
        {
            std::cout << "Level " << hasNext->height << ": ";
            // pivots
            for (int i = 0; i < hasNext->pivots.size(); i++)
            {
                Node *current = hasNext->pivots[i];
                string value = (current->value == INT_MIN) ? "-i" : std::to_string(current->value);
                if (current->down)
                {
                    string downValue = (current->down->pivots[0]->value == INT_MIN) ? "-i" : std::to_string(current->down->pivots[0]->value);
                    cout << value << "(d:" << downValue << ",h:" << current->height << ") ";
                }
                else
                {
                    cout << value << "(h:" << current->height << ") ";
                }
            }
            // buffer
            std::cout << "[";
            for (int i = 0; i < hasNext->buffer.size(); i++)
            {
                Node *current = hasNext->buffer[i];
                cout << current->value << " ";
            }
            std::cout << "]\t";
            hasNext = hasNext->next;
        }
    }
};

class BSkipList
{
private:
    std::vector<Block *> levels; // Vector of head blocks from each level
    std::unordered_map<int, int> levelMap;
    const int MAX_LEVEL = 32;

    // try find value in a block
    bool find(int value, Block *block)
    {
        if (!block)
            return false;
        for (int i = block->buffer.size() - 1; i >= 0; i--)
        {
            Node *current = block->buffer[i];
            if (current->value == value)
            {
                if (current->opcode == 0)
                    return true;
                else
                    return false;
            }
        }
        Block *prev;

        for (int i = 0; i < block->pivots.size(); i++)
        {
            Node *current = block->pivots[i];
            if (current->value == value)
                return true;
            else if (value > current->value)
                prev = current->down;

            else
                break;
        }
        return find(value, prev);
    }

    // build from the block all the way down
    Block *buildForm(int value, int height, Block *block)
    {
        Block *newBlock;
        Block *lower = nullptr;
        Node *prev = block->pivots[0];
        for (int i = 0; i < block->pivots.size(); i++)
        {
            // in the middle of the pivots
            if (block->pivots[i]->value > value)
            {
                // build lower level
                if (prev->down)
                    lower = buildForm(value, height, prev->down);
                // split and shrink block
                std::vector<Node *> right;
                right.push_back(new Node(value, lower, height, 0));
                for (int j = i; j < block->pivots.size(); j++)
                    right.push_back(block->pivots[j]);
                block->pivots.resize(i);
                newBlock = new Block(NULL, block->next, block->height);
                newBlock->pivots = right;
                block->next = newBlock;
                // copy over buffer if needed
                for (int j = 0; j < block->buffer.size(); j++)
                {
                    if (block->buffer[j]->value > value)
                    {
                        newBlock->buffer.push_back(block->buffer[j]);
                        block->buffer.erase(block->buffer.begin() + j);
                    }
                }
                return newBlock;
            }
            prev = block->pivots[i];
        }
        // at the end of the vector
        // build lower level
        if (prev->down)
            lower = buildForm(value, height, prev->down);
        newBlock = new Block(new Node(value, lower, height, 0), block->next, block->height);
        block->next = newBlock;
        // copy over buffer if needed
        for (int j = 0; j < block->buffer.size(); j++)
        {
            if (block->buffer[j]->value > value)
            {
                newBlock->buffer.push_back(block->buffer[j]);
                block->buffer.erase(block->buffer.begin() + j);
            }
        }
        return newBlock;
    }

    // promote node from block
    void promote(int value, int height, Block *block)
    {
        Block *up = new Block(new Node(INT_MIN, levels[block->height], block->height + 1, 0), nullptr, block->height + 1);
        levels.push_back(up);
        // need more promotion
        if (height > block->height + 1)
            promote(value, height, up);
        // top level
        else
        {
            Block *newBlock = buildForm(value, height, levels[height - 1]);
            up->pivots.push_back(new Node(value, newBlock, height, 0));
        }
    }

    // remove element from the whole list
    void removeElement(int value)
    {
        Block *current = levels[levels.size() - 1];
        Block *prev;
        while (current)
        {
            for (int i = 0; i < current->pivots.size(); i++)
            {
                if (value == current->pivots[i]->value)
                {
                    // remove at top level then all the way down
                    current->pivots.erase(current->pivots.begin() + i);
                    removeFrom(value, prev);
                    return;
                }
                prev = current->pivots[i]->down;
            }
            current = current->pivots[0]->down;
        }
    }

    // remove element from block all the way down
    void removeFrom(int value, Block *prev)
    {
        if (!prev)
        {
            return;
        }
        Block *block = prev->next;
        // leader
        if (block->pivots[0]->value == value)
        {
            for (int i = 0; i < block->buffer.size(); i++)
            {
                prev->buffer.push_back(block->buffer[i]);
            }
            prev->next = block->next;
            prev = prev->pivots[prev->pivots.size() - 1]->down;
            removeFrom(value, prev);
        }
        else
        {
            for (int i = 0; i < block->pivots.size(); i++)
            {
                if (value == block->pivots[i]->value)
                {
                    // remove element from level
                    block->pivots.erase(block->pivots.begin() + i);
                    removeFrom(value, prev);
                }
                prev = block->pivots[i]->down;
            }
        }
    }

    // Sorted after add a node into a vector, return the prevous node
    Node *addToVector(Node *&node, std::vector<Node *> &vector)
    {
        Node *prev = nullptr;
        for (int i = 0; i < vector.size(); i++)
        {
            // remove duplicate
            if (vector[i]->value == node->value)
                return prev;
            // in the middle
            if (vector[i]->value > node->value)
            {
                vector.insert(vector.begin() + i, node);
                return prev;
            }
            prev = vector[i];
        }
        // at the end
        vector.push_back(node);
        return prev;
    }

    // remove blocks only has -infinity as pivot
    void checkLevel()
    {
        while (true)
        {
            Block *current = levels[levels.size() - 1];
            if (current->pivots.size() == 1)
            {
                if (current->buffer.empty())
                {
                    levels.pop_back();
                }
                else
                {
                    flush(current, true);
                }
            }
            else
                break;
        }
    }

    // flush current block, return true if deleted top level
    bool flush(Block *block, bool flag)
    {
        bool deleted = false;
        // when buffer is full or have enough delete, flush(leaves don't need flush)
        if ((block->height > 0 && (block->buffer.size() + block->pivots.size() > B || block->numberOfDeletedNode * 2 >= B)) || flag)
        {
            // flush each node in buffer
            for (int i = 0; i < block->buffer.size(); i++)
            {
                Node *current = block->buffer[i];
                // remove duplicate
                Block *down;
                // insert pivot
                if (current->opcode == 0 && current->height >= block->height)
                {
                    bool has = false;
                    // remove duplicate
                    for (int p = 0; p < block->pivots.size(); p++)
                    {
                        if (block->pivots[p]->value == current->value)
                            has = true;
                    }
                    if (!has)
                    {
                        down = addToVector(current, block->pivots)->down;
                        Block *newDown = buildForm(current->value, current->height, down);
                        current->down = newDown;
                    }
                    continue;
                }
                // delete pivot
                else if (current->opcode == 1 && current->height >= block->height)
                {
                    removeElement(current->value);
                    if (current->height == levels.size() - 1)
                        deleted = true;
                    continue;
                }

                // find place for message to flush
                down = block->pivots[0]->down;
                for (int j = 0; j < block->pivots.size(); j++)
                {
                    if (block->pivots[j]->value <= current->value)
                        down = block->pivots[j]->down;
                }
                // normal case, add to lower buffer
                if (down->height > 0)
                {
                    down->buffer.push_back(current);
                    flush(down, false);
                }
                // flush to leaves are speacial case
                else if (down->height == 0)
                {
                    if (current->opcode == 0)
                    {
                        addToVector(current, down->buffer);
                    }
                    else
                    {
                        for (int j = 0; j < down->buffer.size(); j++)
                        {
                            if (down->buffer[j]->value == current->value)
                                down->buffer.erase(down->buffer.begin() + j);
                        }
                    }
                }
            }
            //  clean the buffer
            block->buffer.clear();
            block->numberOfDeletedNode = 0;
        }
        return deleted;
    }

public:
    BSkipList()
    {
    }

    ~BSkipList()
    {
        // Destructor to free memory
        // ... (cleanup logic here)
    }

    void upsert(int value, int opcode, int height)
    {
        // empty list, no need to delete
        if (levels.size() == 0)
        {
            if (opcode == 1)
                return;
            // add a new block
            else
            {
                Block *block = new Block(new Node(INT_MIN, nullptr, 0, 0), nullptr, levels.size());
                levels.push_back(block);
            }
        }
        // top block
        Block *block = levels[levels.size() - 1];
        // promote new node if needed
        if (height > block->height && opcode == 0)
        {
            promote(value, height, block);
            return;
        }

        // leaf node
        if (block->height == 0)
        {
            if (opcode == 0)
            {
                Node *newNode = new Node(value, nullptr, height, opcode);
                addToVector(newNode, block->buffer);
            }
            else
            {
                for (int i = 0; i < block->buffer.size(); i++)
                {
                    if (block->buffer[i]->value == value)
                        block->buffer.erase(block->buffer.begin() + i);
                }
                // if list is empty
                if (block->buffer.empty())
                    levels.clear();
            }
        }
        else
            // add new node to buffer
            block->buffer.push_back(new Node(value, nullptr, height, opcode));

        // trak delete messages
        if (opcode == 1)
            block->numberOfDeletedNode += 1;
        // try flush block
        if (flush(block, false))
            checkLevel();
    }

    void insert(int value)
    {
        int height = random_level(value);

        upsert(value, 0, height);
    }

    void remove(int value)
    {
        int height = random_level(value);
        upsert(value, 1, height);
    }

    void print_list()
    {
        for (int i = levels.size() - 1; i >= 0; i--)
        {
            levels[i]->print();
            cout << endl;
        }
    }

    bool query(int value)
    {
        return find(value, levels[levels.size() - 1]);
    }

    std::vector<int> range_query(int start, int end)
    {
        std::vector<int> output;
        for (int key = start; key < end; key++)
        {
            if (query(key))
                output.push_back(key);
        }
        return output;
    }
    int random_level(int element)
    {
        float epi = 0.5;
        // Check if e is already in the map
        auto it = levelMap.find(element);
        if (it != levelMap.end())
        {
            // If e is found in the map, return the stored level
            return it->second;
        }
        int random_level = 0;
        int seed = time(NULL);
        static default_random_engine e(seed);
        static uniform_real_distribution<float> u(0.0, 1.0);
        float P = 1.0 / pow(B, 1 - epi);
        while (u(e) < P && random_level < MAX_LEVEL)
        {
            P = 1.0 / pow(B, epi);
            random_level++;
        }
        levelMap[element] = random_level;
        return random_level;
    }
};


int main()
{
    BSkipList list;

    // cout << list.random_level(2) << "random" << endl;

    int testDataSize = 1000000;
    for (int i = 0; i < testDataSize; ++i)
    {
        list.insert(i);
    }
    int test_duration = 10;
    // Start the timer thread
    
    // Start timing
    auto start = std::chrono::high_resolution_clock::now();
    int total_operations = 0;
    // Perform add operations
    int seed = time(NULL);
        static default_random_engine e(seed);
    for (int i = 0; i < testDataSize; ++i)
    {   
        static uniform_int_distribution<int> u(0, 100000);
        list.query(u(e));
        
    }

    // Calculate the throughput: total operations divided by the test duration
    

    // Stop timing
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate duration
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken to add " << testDataSize << " elements: "
              << duration.count() << " seconds." << std::endl;

    return 0;
}