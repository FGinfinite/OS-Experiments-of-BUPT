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
    LFR,
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
    int pageTableSize;
    int pageSize;
    int page_fault;
    int page_hit;
    int page_replacement;
    PageReplacementPolicy policy;

    // 页表置换信息
    int pageIndex; // FIFO中是队列的头
} PageTable;

typedef struct memory
{
    Instruction *data;
    int size;
} Memory;

Memory externalMemory;

// 生成指令序列
void generateInstructionSequence(int **instructionSequencePtr, int size);
// 初始化页
void initPage(Page *page, int size);
// 初始化页表
void initPageTable(PageTable *pageTable, int pageTableSize, int pageSize);
// 初始化内存
void initMemory();
// 调整页表大小
void resizePageTable(PageTable *pageTable, int size);
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