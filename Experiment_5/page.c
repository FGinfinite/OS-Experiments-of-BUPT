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
    pageTable->LRUmarks = NULL;
    pageTable->LFUmarks = NULL;
    pageTable->CLOCKmarks = NULL;
    pageTable->OPTmarks = NULL;
    pageTable->pages = (Page *)malloc(sizeof(Page) * pageTableSize);
    for (int i = 0; i < pageTableSize; ++i)
    {
        initPage(pageTable->pages + i, pageSize);
    }
}

void resizePageTable(PageTable *pageTable, int pageTableSize, int pageSize)
{
    free(pageTable->pages);
    // 重置页面置换信息
    pageTable->page_fault = 0;
    pageTable->page_hit = 0;
    pageTable->page_replacement = 0;
    pageTable->pageTableSize = pageTableSize;
    pageTable->pageSize = pageSize;
    switch (pageTable->policy)
    {
    case FIFO:
        pageTable->pageIndex = 0;
        break;
    case LRU:
        pageTable->LRUmarks == NULL ?: free(pageTable->LRUmarks);
        pageTable->LRUmarks = (int *)malloc(sizeof(int) * pageTableSize);
        memset(pageTable->LRUmarks, 0, sizeof(int) * pageTableSize);
        break;
    case LFU:
        pageTable->LFUmarks == NULL ?: free(pageTable->LFUmarks);
        pageTable->LFUmarks = (int *)malloc(sizeof(int) * pageTableSize);
        memset(pageTable->LFUmarks, 0, sizeof(int) * pageTableSize);
        break;
    case CLOCK:
        pageTable->CLOCKmarks == NULL ?: free(pageTable->CLOCKmarks);
        pageTable->CLOCKmarks = (int *)malloc(sizeof(int) * pageTableSize);
        memset(pageTable->CLOCKmarks, 0, sizeof(int) * pageTableSize);
        break;
    case OPT:
        pageTable->OPTmarks == NULL ?: free(pageTable->OPTmarks);
        pageTable->OPTmarks = (int *)malloc(sizeof(int) * pageTableSize);
        memset(pageTable->OPTmarks, 0, sizeof(int) * pageTableSize);
        break;
    }

    pageTable->pages = (Page *)malloc(sizeof(Page) * pageTableSize);
    for (int i = 0; i < pageTableSize; ++i)
    {
        resizePage(pageTable->pages + i, pageSize);
    }
}

void initPage(Page *page, int size)
{
    page->pageNumber = PAGE_NULL;
    page->data = (Instruction *)malloc(sizeof(Instruction) * size);
}

void resizePage(Page *page, int size)
{
    page->data = (Instruction *)malloc(sizeof(Instruction) * size);
    page->pageNumber = PAGE_NULL;
}

int pageTableLookup(PageTable *pageTable, int instructionIndex)
{
    if (pageTable->policy == LRU)
    {
        for (int i = 0; pageTable->pages[i].pageNumber != PAGE_NULL && i < pageTable->pageTableSize; ++i)
        {
            ++pageTable->LRUmarks[i];
        }
    }
    int pageNumber = instructionIndex / pageTable->pageSize;
    for (int i = 0; pageTable->pages[i].pageNumber != PAGE_NULL && i < pageTable->pageTableSize; ++i)
    {
        if (pageTable->pages[i].pageNumber == pageNumber)
        {
            ++pageTable->page_hit;
            if (pageTable->policy == LRU)
            {
                pageTable->LRUmarks[i] = 0;
            }
            else if (pageTable->policy == LFU)
            {
                ++pageTable->LFUmarks[i];
            }
            else if (pageTable->policy == CLOCK)
            {
                pageTable->CLOCKmarks[i] = 1;
            }
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
            if (pageTable->policy == LRU)
            {
                pageTable->LRUmarks[i] = 0;
            }
            else if (pageTable->policy == LFU)
            {
                pageTable->LFUmarks[i] = 0;
            }
            else if (pageTable->policy == CLOCK)
            {
                pageTable->CLOCKmarks[i] = 0;
            }
            else if (pageTable->policy == OPT)
            {
                // 寻找当前页号的下一次访问时间
                int nextTime = 0;
                int j;
                for (j = outerCount + 1; j < INSTRUCTIONS_SIZE; ++j)
                {
                    if (OPT_instructionSequence[j] / pageTable->pageSize == pageTable->pages[i].pageNumber)
                    {
                        break;
                    }
                }
                nextTime = j - outerCount;
                pageTable->OPTmarks[i] = nextTime;
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

    // 将OPTmarks中的数据更新
    if (pageTable->policy == OPT)
    {
        for (int i = 0; pageTable->pages[i].pageNumber != PAGE_NULL && i < pageTable->pageTableSize; ++i)
        {
            if ((--pageTable->OPTmarks[i]) < 0)
            {
                // 重新计算其下一次访问时间
                int nextTime = 0;
                int j;
                for (j = outerCount + 1; j < INSTRUCTIONS_SIZE; ++j)
                {
                    if (OPT_instructionSequence[j] / pageTable->pageSize == pageTable->pages[i].pageNumber)
                    {
                        break;
                    }
                }
                nextTime = j - outerCount;
                pageTable->OPTmarks[i] = nextTime;
            }
        }
    }

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
        // 利用LRUmarks数组来判断最近最久未使用的页面
        int max = 0;
        int maxIndex = 0;
        for (int i = 0; pageTable->pages[i].pageNumber != PAGE_NULL && i < pageTable->pageTableSize; ++i)
        {
            if (pageTable->LRUmarks[i] > max)
            {
                max = pageTable->LRUmarks[i];
                maxIndex = i;
            }
        }
        pageTable->pages[maxIndex].pageNumber = instructionIndex / pageTable->pageSize;
        // 将内存中的数据复制到页表中
        for (int i = 0; i < pageTable->pageSize; ++i)
        {
            pageTable->pages[maxIndex].data[i] = externalMemory.data[pageTable->pages[maxIndex].pageNumber * pageTable->pageSize + i];
        }
        pageTable->LRUmarks[maxIndex] = 0;
        break;
    case LFU:
        // 利用LFUmarks数组来判断最近最少使用的页面
        int min = 0;
        int minIndex = 0;
        for (int i = 0; pageTable->pages[i].pageNumber != PAGE_NULL && i < pageTable->pageTableSize; ++i)
        {
            if (pageTable->LFUmarks[i] < min)
            {
                min = pageTable->LFUmarks[i];
                minIndex = i;
            }
        }
        pageTable->pages[minIndex].pageNumber = instructionIndex / pageTable->pageSize;
        // 将内存中的数据复制到页表中
        for (int i = 0; i < pageTable->pageSize; ++i)
        {
            pageTable->pages[minIndex].data[i] = externalMemory.data[pageTable->pages[minIndex].pageNumber * pageTable->pageSize + i];
        }
        pageTable->LFUmarks[minIndex] = 0;
        break;
    case CLOCK:
        // 利用CLOCKmarks数组来判断最近最久未使用的页面
        while (pageTable->CLOCKmarks[pageTable->pageIndex] == 1)
        {
            pageTable->CLOCKmarks[pageTable->pageIndex] = 0;
            pageTable->pageIndex = (pageTable->pageIndex + 1) % pageTable->pageTableSize;
        }
        pageTable->pages[pageTable->pageIndex].pageNumber = instructionIndex / pageTable->pageSize;
        // 将内存中的数据复制到页表中
        for (int i = 0; i < pageTable->pageSize; ++i)
        {
            pageTable->pages[pageTable->pageIndex].data[i] = externalMemory.data[pageTable->pages[pageTable->pageIndex].pageNumber * pageTable->pageSize + i];
        }
        pageTable->CLOCKmarks[pageTable->pageIndex] = 0;
        pageTable->pageIndex = (pageTable->pageIndex + 1) % pageTable->pageTableSize;
        break;
    case OPT:
        // 寻找不再会被访问或最长时间内不再被访问的页面
        int maxTime = 0;
        int maxTimeIndex = 0;
        for (int i = 0; pageTable->pages[i].pageNumber != PAGE_NULL && i < pageTable->pageTableSize; ++i)
        {
            if (pageTable->OPTmarks[i] > maxTime)
            {
                maxTime = pageTable->OPTmarks[i];
                maxTimeIndex = i;
            }
        }
        pageTable->pages[maxTimeIndex].pageNumber = instructionIndex / pageTable->pageSize;
        // 将内存中的数据复制到页表中
        for (int i = 0; i < pageTable->pageSize; ++i)
        {
            pageTable->pages[maxTimeIndex].data[i] = externalMemory.data[pageTable->pages[maxTimeIndex].pageNumber * pageTable->pageSize + i];
        }
        // 寻找当前页号的下一次访问时间
        int nextTime;
        int j;
        for (j = instructionIndex + 1; j < INSTRUCTIONS_SIZE; ++j)
        {
            if (OPT_instructionSequence[j] / pageTable->pageSize == pageTable->pages[maxTimeIndex].pageNumber)
            {
                break;
            }
        }
        nextTime = j - outerCount;
        pageTable->OPTmarks[maxTimeIndex] = nextTime;
        break;
    default:
        break;
    }
}

void printPageTableReplacementInfo(PageTable *pageTable)
{
    // 打印当前策略，页表大小，页大小，命中率，置换率
    printf("当前策略：%s\t页表大小：%d\t页大小：%d\t命中率：%.2f%%\t置换率：%.2f%%\n", pageTable->policy == FIFO ? "FIFO" : pageTable->policy == LRU ? "LRU"
                                                                                                                        : pageTable->policy == LFU   ? "LFU"
                                                                                                                        : pageTable->policy == CLOCK ? "CLOCK"
                                                                                                                                                     : "OPT",
           pageTable->pageTableSize, pageTable->pageSize, (double)(INSTRUCTIONS_SIZE - pageTable->page_fault) / (double)INSTRUCTIONS_SIZE * 100, (double)pageTable->page_replacement / (double)INSTRUCTIONS_SIZE * 100);
    // 将结果输出到result.txt文件中
    fprintf(fp, "%s %d %.2f%% %.2f%%\n", pageTable->policy == FIFO ? "FIFO" : pageTable->policy == LRU ? "LRU"
                                                                          : pageTable->policy == LFU   ? "LFU"
                                                                          : pageTable->policy == CLOCK ? "CLOCK"
                                                                                                       : "OPT",
            pageTable->pageTableSize, (double)(INSTRUCTIONS_SIZE - pageTable->page_fault) / (double)INSTRUCTIONS_SIZE * 100, (double)pageTable->page_replacement / (double)INSTRUCTIONS_SIZE * 100);
}

int main()
{
    // 初始化内存
    initMemory();
    // 生成指令序列
    int LogicalAddress = INSTRUCTIONS_SIZE / 10;
    PageTable pageTable;
    initPageTable(&pageTable, PAGE_TABLE_INITIAL_SIZE, PAGE_INITIAL_SIZE);
    srand(time(NULL));
    // 反复测试十次，并且输出之result_0.txt～result_9.txt中
    for (int file_count = 0; file_count < 10; ++file_count)
    {
        char *filename = (char *)malloc(sizeof(char) * 30);
        sprintf(filename, "./result/result_%d.txt", file_count);
        fp = fopen(filename, "w");
        int *instructionSequence;
        generateInstructionSequence(&instructionSequence, INSTRUCTIONS_SIZE);
        OPT_instructionSequence = instructionSequence;
        // 测试五个策略
        for (int k = 0; k < 5; ++k)
        {
            pageTable.policy = k;
            // 按指令序列读取并打印指令
            for (int i = 4; i <= 32; ++i)
            {
                resizePageTable(&pageTable, i, PAGE_INITIAL_SIZE);
                for (int j = 0; j < INSTRUCTIONS_SIZE - 1; ++j)
                {
                    outerCount = j;
                    readInstruction(&pageTable, instructionSequence[j]);
                }
                printPageTableReplacementInfo(&pageTable);
            }
        }
        // 关闭文件
        fclose(fp);
    }
    return 0;
}