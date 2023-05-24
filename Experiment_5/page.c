#include "page.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

void initMemory()
{
    externalMemory.size = MEMORY_SIZE;
    externalMemory.data = (Instruction *)malloc(sizeof(Instruction) * MEMORY_SIZE);
    for (int i = 0; i < MEMORY_SIZE; ++i)
    {
        externalMemory.data[i].address = i;
        // 数据域为随机数
        externalMemory.data[i].data = rand() % 1000;
    }
}

void generateInstructionSequence(int **instructionSequencePtr, int size)
{
    *instructionSequencePtr = (int *)malloc(sizeof(int) * size);
    int *instructionSequence = *instructionSequencePtr;
    srand(time(NULL));
    int count;
    for (count = 0; count < size / 4; ++count)
    {
        instructionSequence[count * 4] = rand() % size;
        instructionSequence[count * 4 + 1] = rand() % (instructionSequence[count * 4] == 0 ? 1 : instructionSequence[count * 4]);
        instructionSequence[count * 4 + 2] = instructionSequence[count * 4 + 1] + 1;
        instructionSequence[count * 4 + 3] = rand() % (size == instructionSequence[count * 4 + 2] ? 1 : size - instructionSequence[count * 4 + 2]) + instructionSequence[count * 4 + 2];
    }
    // 最后不满足4个一组的情况将直接随机生成
    while (count < size)
    {
        instructionSequence[count] = rand() % size;
        ++count;
    }
}

void initPageTable(PageTable *pageTable, int pageTableSize, int pageSize)
{
    pageTable->pageTableSize = pageTableSize;
    pageTable->pageSize = pageSize;
    pageTable->page_fault = 0;
    pageTable->page_hit = 0;
    pageTable->page_replacement = 0;
    pageTable->policy = FIFO;
    pageTable->pageIndex = 0;
    pageTable->pages = (Page *)malloc(sizeof(Page) * pageTableSize);
    for (int i = 0; i < pageTableSize; ++i)
    {
        initPage(pageTable->pages + i, pageSize);
    }
}

void resizePageTable(PageTable *pageTable, int size)
{
    free(pageTable->pages);
    pageTable->pageTableSize = size;
    pageTable->pages = (Page *)malloc(sizeof(Page) * size);
    for (int i = 0; i < size; ++i)
    {
        resizePage(pageTable->pages + i, size);
    }
}

void initPage(Page *page, int size)
{
    page->pageNumber = PAGE_NULL;
    page->data = (Instruction *)malloc(sizeof(Instruction) * size);
}

void resizePage(Page *page, int size)
{
    free(page->data);
    page->data = (Instruction *)malloc(sizeof(Instruction) * size);
}

int pageTableLookup(PageTable *pageTable, int instructionIndex)
{
    int pageNumber = instructionIndex / pageTable->pageSize;
    for (int i = 0; i < pageTable->pageTableSize; ++i)
    {
        if (pageTable->pages[i].pageNumber == pageNumber)
        {
            ++pageTable->page_hit;
            return i;
        }
    }
    ++pageTable->page_fault;
    return PAGE_FAULT;
}

void loadPage(PageTable *pageTable, int instructionIndex)
{
    int pageNumber = instructionIndex / pageTable->pageSize;
    for (int i = 0; i < pageTable->pageTableSize; ++i)
    {
        if (pageTable->pages[i].pageNumber == PAGE_NULL)
        {
            pageTable->pages[i].pageNumber = pageNumber;
            // 将内存中的数据复制到页表中
            for (int j = 0; j < pageTable->pageSize; ++j)
            {
                pageTable->pages[i].data[j] = externalMemory.data[pageNumber * pageTable->pageSize + j];
            }
            return;
        }
    }
    // 如果没有空闲页，则进行页面置换
    replacePage(pageTable, instructionIndex);
}

int readInstruction(PageTable *pageTable, int instructionIndex)
{
    int pageIndex = pageTableLookup(pageTable, instructionIndex);
    if (pageIndex == PAGE_FAULT)
    {
        loadPage(pageTable, instructionIndex);
        pageIndex = pageTableLookup(pageTable, instructionIndex);
    }
    // // 打印当前页表中的所有页号
    // int count = 0;
    // while (pageTable->pages[count].pageNumber != PAGE_NULL && count < pageTable->pageTableSize)
    // {
    //     printf("%d ", pageTable->pages[count].pageNumber);
    //     ++count;
    // }
    // printf("\n");
    return pageTable->pages[pageIndex].data[instructionIndex % pageTable->pageSize].data;
}

void replacePage(PageTable *pageTable, int instructionIndex)
{
    ++pageTable->page_replacement;
    switch (pageTable->policy)
    {
    case FIFO:
        // 当FIFO发生页面置换时，必定是队列已满，所以可以只用pageIndex来判断队头与队尾
        pageTable->pages[pageTable->pageIndex].pageNumber = instructionIndex / pageTable->pageSize;
        // 将内存中的数据复制到页表中
        for (int i = 0; i < pageTable->pageSize; ++i)
        {
            pageTable->pages[pageTable->pageIndex].data[i] = externalMemory.data[pageTable->pages[pageTable->pageIndex].pageNumber * pageTable->pageSize + i];
        }
        // 更新页表指针
        pageTable->pageIndex = (pageTable->pageIndex + 1) % pageTable->pageTableSize;
        break;
    case LRU:
        break;
    case LFR:
        break;
    case OPT:
        break;
    default:
        break;
    }
}

int main()
{
    initMemory();
    // 生成指令序列
    int LogicalAddress = INSTRUCTIONS_SIZE / 10;
    PageTable pageTable;
    initPageTable(&pageTable, PAGE_TABLE_INITIAL_SIZE, PAGE_INITIAL_SIZE);
    int *instructionSequence;
    generateInstructionSequence(&instructionSequence, INSTRUCTIONS_SIZE);

    // 按指令序列读取并打印指令320次
    /*打印样式：
        ---------------------------------------------------------------------------------------------------
        |    count    |    count    |    count    |    count    |    count    |    count    |    count    |
        | aa bb cc dd | aa bb cc dd | aa bb cc dd | aa bb cc dd | aa bb cc dd | aa bb cc dd | aa bb cc dd |
        |    addr     |    addr     |    addr     |    addr     |    addr     |    addr     |    addr     |
        |    data     |    data     |    data     |    data     |    data     |    data     |    data     |
        ---------------------------------------------------------------------------------------------------
        -count 循环次数
        -aa bb cc dd 当前页表中的所有页号
        -addr 逻辑地址
        -data 逻辑地址对应的数据
    */
    const char *divide = "===================================================================================================\n";
    char pageNumbersStr[128] = "";
    char countStr[128] = "";
    char addrStr[128] = "";
    char dataStr[128] = "";
    int count = 0;
    for (int j = 0; j < INSTRUCTIONS_SIZE; ++j, ++count)
    {
        sprintf(countStr, "%s|     %3d     ", countStr, j);
        sprintf(pageNumbersStr, "%s|", pageNumbersStr);
        int k;
        for (k = 0; pageTable.pages[k].pageNumber != PAGE_NULL && k < pageTable.pageTableSize; ++k)
        {
            sprintf(pageNumbersStr, "%s %2d", pageNumbersStr, pageTable.pages[k].pageNumber);
        }
        for(;k<4;++k){
            sprintf(pageNumbersStr, "%s   ", pageNumbersStr);
        }
        sprintf(pageNumbersStr, "%s ", pageNumbersStr);
        sprintf(addrStr, "%s|     %3d     ", addrStr, instructionSequence[j]);
        sprintf(dataStr, "%s|     %3d     ", dataStr, readInstruction(&pageTable, instructionSequence[j]));

        if (count == 6 || j == INSTRUCTIONS_SIZE - 1)
        {
            sprintf(pageNumbersStr, "%s|\n", pageNumbersStr);
            sprintf(countStr, "%s|\n", countStr);
            sprintf(addrStr, "%s|\n", addrStr);
            sprintf(dataStr, "%s|\n", dataStr);
            printf("%s%s%s%s%s", divide, countStr, pageNumbersStr, addrStr, dataStr);

            sprintf(pageNumbersStr, "");
            sprintf(countStr, "");
            sprintf(addrStr, "");
            sprintf(dataStr, "");

            count = -1;
        }
    }
    return 0;
}