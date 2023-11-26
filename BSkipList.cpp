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
    std::vector<Node *> vector;
    std::vector<Node *> buffer;
    Block *next; // Pointer to the next block at the same level
    int numberOfDeletedNode;
    Block(Node *node, Block *next)
    {
        vector.push_back(node);
        // vector.resize(3); // minimum size of each block
        this->next = next;
    }

    Block(std::vector<Node *> vector, Block *next, std::vector<Node *> buffer, int numberOfDeletedNode)
    {
        this->vector = vector;
        // vector.resize(3); // minimum size of each block
        this->next = next;
        this->buffer = buffer;
        this->numberOfDeletedNode = numberOfDeletedNode;
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
    // const int MAX_LEVEL = 32;
    // const float P_FACTOR = 0.25;
    // static std::random_device rd; // obtain a random number from hardware
    // static std::mt19937 gen(rand()); // seed the generator
    // static std::uniform_real_distribution<> distr(0, 1); // define the range
    BSkipList()
    {
        Block *block = new Block(new Node(INT_MIN, nullptr, 0, 0), nullptr); // negative infinity block
        levels.push_back(block);
    }

    ~BSkipList()
    {
        // Destructor to free memory
        // ... (cleanup logic here)
    }
    // 先写flush操作作为helper，如果超了，那么就发送到下一个block中，如果block满了，标记并加入一个queue， 然后循环处理直到把queue处理完毕(BFS)
    void upsert(int value, int opcode, int lvl)
    {   
        //如果比当前level高，直接变成pivot
        if (opcode == 0 && lvl > levels.size())
        {
            insert(value);
        }
        //插入buffer等flush
        else
        {
            Block *block = levels[levels.size() - 1];
            //需要down几次，然后变成pivot
            lvl = (levels.size() - 1) - lvl;

            //删除值处理
            if (opcode == 1)
            {
                block->numberOfDeletedNode += 1;
            }
            // Insert the new Node 
            block->buffer.push_back(new Node(value, nullptr, lvl, opcode));

            // flush if full or deleted massage more than half so flush
            // 循环处理需要flush的blk
            if (block->buffer.size() + block->vector.size() > B || block->numberOfDeletedNode * 2 >= B)
            {
                std::deque<Block *> queue;
                queue.push_back(block);


                // cout<<queue.size()<<endl;
                while (!queue.empty())
                {
                    Block *blk = queue.front();
                    queue.pop_front();
                    flush(blk);
                    blk->buffer.clear();
                    blk->numberOfDeletedNode = 0;
                    // check down block
                    for (int i = 0; i < blk->vector.size(); i++)
                    {
                        Block *downBlock = blk->vector[i]->down;
                        if (downBlock != nullptr)
                        {
                            if ((downBlock->buffer.size() + downBlock->vector.size() > B) || downBlock->numberOfDeletedNode * 2 >= B)
                            {
                                queue.push_back(downBlock);
                            }
                        }
                    }
                }
            }
        }
    }

    void add(int value)
    {
        int lvl = 0;
        //随机数生成
        upsert(value, 0, lvl);
    }

    void deleted(int value)
    {
        int lvl = 0;
        upsert(value, 1, lvl);
    }

    //flush本身，flush to correct position
    void flush(Block *curr)
    {
        int sz = curr->vector.size();
        //每个buffer的值和pivot比较
        for (int j = 0; j < curr->buffer.size(); j++)
        {
            int value = curr->buffer[j]->value;
            bool flag = false;
            cout << value << endl;
            cout << curr->buffer[j]->height << "h"<<endl;
            //loop每个pivot
            for (int i = 0; i < sz - 1; i++)
            {   
                //找到flush的位置
                // 待fixed
                //到指定的lvl，他需要变成pivot，但是没变
                //需要处理pivot
                if (curr->vector[i + 1]->value > value)
                {
                    if (curr->vector[i]->down != nullptr)
                    {
                        // cout << curr->buffer[j]->height << "height be" << endl;
                        // curr->buffer[j]->height = curr->buffer[j]->height - 1;
                        // cout << curr->buffer[j]->height << "height af" << endl;
                        // if (curr->buffer[j]->height <= 0)
                        // {
                        //     if (curr->buffer[j]->opcode == 0)
                        //     {
                        //         // 待优化
                        //         insert(curr->buffer[j]->value);
                        //     }
                        //     else if (curr->buffer[j]->opcode == 1)
                        //     {
                        //         remove(curr->buffer[j]->value);
                        //     }

                        // }
                        // else
                        // {
                        //     curr->vector[i]->down->buffer.push_back(curr->buffer[j]);
                        //     flag = true;
                        //     break;
                        // }
                        if(curr->buffer[j]->opcode == 1){
                            curr->vector[i]->down->numberOfDeletedNode+=1;
                        }
                        curr->vector[i]->down->buffer.push_back(curr->buffer[j]);
                        flag = true;
                        break;
                    }
                }
            }
            //走最后一个，成功flush
            //这个换成最新的insert应该就没问题
            if (!flag)
            {
                if (curr->vector[sz - 1]->down != nullptr)
                {
                    curr->buffer[j]->height = curr->buffer[j]->height - 1;
                    if (curr->buffer[j]->height <= 0)
                    {
                        // 待优化
                        // 0 是插入
                        // 1 是remove
                        if (curr->buffer[j]->opcode == 0)
                        {   
                            //我们已知height，根据height来建造pivot
                            insert(curr->buffer[j]->value);
                        }
                        else if (curr->buffer[j]->opcode == 1)
                        {
                            remove(curr->buffer[j]->value);
                        }
                    }
                    else
                    {
                        if (curr->buffer[j]->opcode == 1)
                        {
                            curr->vector[sz - 1]->down->numberOfDeletedNode += 1;
                        }
                        curr->vector[sz - 1]->down->buffer.push_back(curr->buffer[j]);
                    }
                }
            }
        }
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
                    if (r % 2 == 0)
                    { // tail
                        r = r + rand();
                        block->vector.insert(block->vector.begin() + i, new Node(value, lower, 0, 0));
                        return;
                    }
                    else
                    { // head
                        r++;
                        // split and shrink block
                        std::vector<Node *> right;
                        std::vector<Node *> buffer;
                        right.push_back(new Node(value, lower, 0, 0));
                        for (unsigned int j = i; j < block->vector.size(); j++)
                            right.push_back(block->vector[j]);
                        block->vector.resize(i);
                        Block *rightBlock = new Block(right, block->next, buffer, 0);
                        block->next = rightBlock;
                        // new level
                        if (blocks.empty())
                        {
                            Block *up = new Block(new Node(INT_MIN, block, 0, 0), nullptr);
                            up->vector.push_back(new Node(value, block->next, 0, 0));
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
                if (r % 2 == 0)
                { // tail
                    r = r + 1;
                    block->vector.push_back(new Node(value, lower, 0, 0));
                    return;
                }
                else
                { // head
                    r = r + rand();
                    Block *newBlock = new Block(new Node(value, lower, 0, 0), block->next);
                    block->next = newBlock;
                    // new level
                    if (blocks.empty())
                    {
                        Block *up = new Block(new Node(INT_MIN, block, 0, 0), nullptr);
                        up->vector.push_back(new Node(value, newBlock, 0, 0));
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
                }
                for (int j = 0; j < curr->buffer.size(); j++)
                {
                    cout << "[" << curr->buffer[j]->value << "]";
                }
                curr = curr->next;

                cout << "|";
            }
            cout << " " << endl;
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

void test_search(BSkipList list)
{
    // Test Search
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

void test_range_query(BSkipList list, int start, int end)
{
    // Test Range Query
    std::vector<bool> rq_output = list.range_query(start, end);
    std::vector<bool>::iterator it;
    int i;

    std::cout << "==========================" << std::endl;
    std::cout << "Test for range search from " << start << " to " << end << std::endl;
    std::cout << "==========================" << std::endl;

    for (it = rq_output.begin(), i = start; it != rq_output.end() && i < end; it++, i++)
    {
        std::cout << "Search " << i << ": " << std::boolalpha << *it << std::endl;
    }
}

int main()
{
    BSkipList list;
    list.insert(1);
    list.insert(10);
    list.insert(3);
    list.insert(2);
    list.insert(6);
    list.print_list();
    cout << "===================" << endl;
    list.remove(3);
    list.remove(1);
    list.remove(10);
    // list.upsert(1, 0, 0);
    // list.upsert(1, 0, 0);
    // list.upsert(4, 0, 0);
    // list.upsert(4, 0, 0);
    // list.upsert(4, 0, 0);
    // list.upsert(7, 0, 0);
    //list.upsert(7, 0, 0);
    // list.upsert(23, 0, 99);
    list.print_list();
    // cout<<"==================="<<endl;
    //  list.remove(7);
    //  list.remove(-2);
    //  std::cout << list.search(-1) << std::endl;
    //  std::cout << list.search(-2) << std::endl;
    //  std::cout << list.search(11) << std::endl;
    //  list.print_list();
    return 0;
}