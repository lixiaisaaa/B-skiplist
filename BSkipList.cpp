#include <iostream>
#include <vector>
#include <stack>
#include <random>
#include <limits.h>
#include <time.h>
#include <deque>
#include <algorithm>
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

// FIX: duplication, leave special case
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
            for (unsigned int i = 0; i < current->pivots.size(); i++)
            {
                if (value > current->pivots[i]->value)
                { // go to next node
                    prev = current->pivots[i];
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
                    if (value < current->pivots[0]->value)
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

    // Sorted after add a node into a vector, return the prevous node
    Node *addToVector(Node *&node, std::vector<Node *> &vector)
    {
        Node *prev = nullptr;
        for (int i = 0; i < vector.size(); i++)
        {
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

    // flush current block
    void flush(Block *block)
    {
        // when buffer is full or have enough delete, flush(leaves don't need flush)
        if (block->height > 0 && (block->buffer.size() + block->pivots.size() > B || block->numberOfDeletedNode * 2 >= B))
        {
            // flush each node in buffer
            for (int i = 0; i < block->buffer.size(); i++)
            {
                Node *current = block->buffer[i];
                Block *down;
                // insert pivot
                if (current->opcode == 0 && current->height >= block->height)
                {
                    down = addToVector(current, block->pivots)->down;
                    if (current->value == 9)
                        cout << down->pivots[0]->value << endl;
                    Block *newDown = buildForm(current->value, current->height, down);
                    current->down = newDown;
                    break;
                }
                // delete pivot
                else if (current->opcode == 1)
                {
                    // FIX: delete algorithm
                }

                // find place for message to flush
                down = block->pivots[0]->down;
                for (int i = 0; i < block->pivots.size(); i++)
                {
                    if (block->pivots[i]->value <= current->value)
                        down = block->pivots[i]->down;
                }
                // normal case, add to lower buffer
                if (down->height != 0)
                {
                    down->buffer.push_back(current);
                    flush(down);
                }
                // flush to leave is a speacial case
                else
                {
                    if (current->opcode == 0)
                        addToVector(current, down->buffer);
                    else
                    {
                        // FIX delete algorithm
                    }
                }
            }
            // clean the buffer
            block->buffer.clear();
        }
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
        if (height > block->height)
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
            // FIX remove algorthim here
        }
        else
            // add new node to buffer
            block->buffer.push_back(new Node(value, nullptr, height, opcode));

        // trak delete messages
        if (opcode == 1)
            block->numberOfDeletedNode += 1;
        // try flush block
        flush(block);
    }

    void insert(int value)
    {
        int height = 0;
        // 随机数生成
        upsert(value, 0, height);
    }

    void remove(int value)
    {
        int height = 0;
        upsert(value, 1, height);
    }

    void remove2(int value)
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
                for (int j = 0; j < curr->pivots.size(); j++)
                {
                    if (curr->pivots[j]->value == value)
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

        int x = 0;
        while (!blocks.empty())
        {
            block = blocks.top();
            blocks.pop();

            for (unsigned int i = 0; i < block->pivots.size(); i++)
            {
                if (block->pivots[i]->value == value)
                {
                    Block *downBlock = block->pivots[i]->down;
                    block->pivots.erase(block->pivots.begin() + i);

                    while (downBlock != nullptr)
                    {
                        current = downBlock->pivots[0]->down;
                        downBlock->pivots.erase(downBlock->pivots.begin());
                        if (!downBlock->pivots.empty())
                        {
                            update[x]->pivots.insert(update[x]->pivots.end(), downBlock->pivots.begin(), downBlock->pivots.end());
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
        for (int i = levels.size() - 1; i >= 0; i--)
        {
            levels[i]->print();
            cout << endl;
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
            for (it = block->pivots.begin(); it != block->pivots.end(); ++it)
            {
                node = *it;
                if (node->value < key)
                {
                    prev_node = node;
                    if (node == *std::prev(block->pivots.end()))
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
            }
        }
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
    list.upsert(5, 0, 1);

    list.upsert(6, 0, 0);
    list.upsert(8, 0, 0);
    list.upsert(1, 0, 0);
    list.upsert(3, 0, 0);
    list.upsert(9, 0, 1);

    list.upsert(10, 0, 0);

    list.upsert(12, 0, 0);
    list.upsert(16, 0, 0);
    list.upsert(21, 0, 0);
    list.upsert(30, 0, 0);

    list.upsert(18, 0, 2);
    list.upsert(6, 0, 0);
    list.upsert(7, 0, 0);
    list.upsert(30, 0, 0);
    list.upsert(12, 0, 0);
    list.upsert(16, 0, 0);
    list.upsert(21, 0, 0);
    list.upsert(30, 0, 0);

    list.print_list();
    return 0;
}