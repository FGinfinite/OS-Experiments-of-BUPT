#include <stdio.h>
#define PAGE_INITIAL_SIZE 10
#define INSTRUCTIONS_SIZE 320
#define PAGE_TABLE_INITIAL_SIZE 4
#define PAGE_NULL -1
#define MEMORY_SIZE 320
#define PAGE_FAULT -1

typedef enum PageReplacementPolicy
{
    // 先进先出
    FIFO,
    // 最近最久未使用
    LRU,
    // 最近最少使用
    LFU,
    // 时钟
    CLOCK,
    // 最佳置换
    OPT
} PageReplacementPolicy;

typedef struct instruction
{
    int address;
    int data;
} Instruction;

typedef struct page
{
    // 数据（被加载到内存中的数据）
    Instruction *data;
    // 页号
    int pageNumber;
} Page;

typedef struct pagetable
{
    Page *pages;
    int pageTableSize; // 页表大小
    int pageSize;      // 页面大小
    int page_fault;
    int page_hit;
    int page_replacement;
    PageReplacementPolicy policy;

    // 页表置换信息
    int pageIndex;   // FIFO中是队列的头
    int *LRUmarks;   // 标记LRU中每个页面的距离上次访问的时间
    int *LFUmarks;   // 标记LFU中每个页面的访问次数
    int *CLOCKmarks; // 标记CLOCK中每个页面是否被访问过
    int *OPTmarks;   // 标记OPT中每个页面距离最后一次访问的时间
} PageTable;

typedef struct memory
{
    Instruction *data;
    int size;
} Memory;

Memory externalMemory;
int *OPT_instructionSequence; // 仅供最佳置换算法使用的指令序列
FILE *fp;
int outerCount;

// 生成指令序列
void generateInstructionSequence(int **instructionSequencePtr, int size);
// 初始化页
void initPage(Page *page, int size);
// 初始化页表
void initPageTable(PageTable *pageTable, int pageTableSize, int pageSize);
// 初始化内存
void initMemory();
// 调整页表大小
void resizePageTable(PageTable *pageTable, int pageTableSize, int pageSize);
// 调整页面大小
void resizePage(Page *page, int size);
// 页表置换
void replacePage(PageTable *pageTable, int instructionIndex);
// 页表查找
int pageTableLookup(PageTable *pageTable, int instructionIndex);
// 页面加载
void loadPage(PageTable *pageTable, int instructionIndex);
// 读取指令
int readInstruction(PageTable *pageTable, int instructionIndex);
// 读取指令并打印(本函数只在页表大小为4时调用)
void printPageTable(PageTable *pageTable, int *instructionSequence, int size);
// 打印本次访问指令序列后的页表置换信息
void printPageTableReplacementInfo(PageTable *pageTable);