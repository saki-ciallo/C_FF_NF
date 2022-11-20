#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdbool.h>

#define maxRam 1024
#define num_pro 10

typedef struct Block {
    int id;             //分区的序号
    int size;           //分区的大小
    int startAddr;      //分区的开始位置
    bool status;        //分区的状态：true为空闲，false为被占用
    int pid;            //如果该分区被占用，则存放占用进程的id; 否则为 - 1
    struct Block* prev;  //指向前面一块内存分区
    struct Block* next;   //指向后面一块内存分区
}block;

typedef struct PCB {
    int pid;            //进程的序号
    int neededMem;      //需要的内存分区大小
    int status;         //1: 内存分配成功；-1：分配失败
    int blockID;        //如果分配成功，保存占用分区的id,否则为 - 1
    struct PCB* next;   //指向下一个PCB
}Node;

block * create_block(){  // 仅创建初始区块
    block * p = NULL;
    block * temp = (block*)malloc(sizeof(block)); // 创建初始区块
    temp->id = 0;
    temp->size = 1024;
    temp->startAddr = 0;
    temp->status = true;  // 空闲
    temp->pid = -1;
    p = temp;  // 头指针指向初始区块
    temp->prev = p;  // 初始区块前驱设为头指针
    temp->next = NULL;
    return p;
}

Node * create_pcb(){
    Node * p = NULL;//创建头指针
    Node * temp = (Node*)malloc(sizeof(Node));//创建首元节点
    //首元节点先初始化
    temp->pid = 0;
    temp->blockID = -1;
    srand((unsigned)time(NULL));
    temp->neededMem = (rand() % 101) + 100;
    temp->next = NULL;
    temp->status = 0;
    p = temp;//头指针指向首元节点
    for (int i = 1; i < num_pro; i++) {  //从第二个节点开始创建
        //创建一个新节点并初始化
        Node *a = (Node*)malloc(sizeof(Node));
        a->pid = i;
        a->neededMem = (rand() % 101) + 100;
        a->next = NULL;
        a->status = 0;
        a->blockID = -1;
        //将temp节点与新建立的a节点建立逻辑关系
        temp->next = a;
        //指针temp每次都指向新链表的最后/新的一个节点
        temp = a;
    }
    //返回建立的节点，只返回头指针 p即可，通过头指针即可找到整个链表
    return p;
}

void display_pcb(Node *p){
    Node*temp = p;
    printf("进程ID\t需求空间\n");
    while(temp){
        printf("%d\t%d\n",temp->pid, temp->neededMem);
        temp = temp->next;
    }
    printf("——————————————————————————————————————————————————————————\n");
}

void display_block(block *p){
    block *temp = p;
    
    printf("区块ID\t 区块开始地址\t\t区块大小\n");
    while(temp){
        printf("%6d %14d \t\t%7d\t\tproc = %d\t\t status = %d\n", temp->id, temp->startAddr, temp->size, temp->pid, temp->status);
        temp = temp->next;
    }
    printf("——————————————————————————————————————————————————————————\n");
}

int count_block(block *p){
    block *temp = p;
    int i = 0;
    while(temp){
        i++;
        temp = temp->next;
    }
    return i;
}

void first_fit(block *p, Node *p1){
    check_point:
    srand((unsigned)time(NULL));  
    for (int i = 0; i < num_pro; i++){  // 循环程序个数次
        block *temp_block = p;  // 传入已生成的Block起始地址
        Node *temp_node = p1;  // 传入已生成的Node起始地址
        while(temp_node){  // 找到需要处理的进程
            if (temp_node->status == 0 && temp_node->blockID == -1){
                break;  // 找到既不是分配失败，也没有分配区块的进程
            }
            temp_node = temp_node->next;  // 保存之后要处理的进程的指针
        }
        display_block(temp_block);  // 先展示当前所以区块
        printf("当前搜索到要处理的进程 %d , 所需要的内存空间是 = %d\n", temp_node->pid, temp_node->neededMem);
        int cblock = count_block(temp_block);  // 统计区块大小，用来作为生成新区块ID的基础
        while (temp_block){  // 搜索有效区块
            if (temp_block->size > temp_node->neededMem && temp_block->status == true){  
                // 如果当前块的可用空间大于当前进程所申请的空间大小，并且块处于空闲状态
                break;  // 满足条件退出循环，可以得到满足条件的区块地址temp_block
            }
            temp_block = temp_block->next;  // 分区一个个往后查找，直到有满足条件的
            // 没有判断进程是否满足放入区块条件
        }
        // 判断当前进程是否能被放入区块
        if (temp_block==NULL && temp_node->status == 0){
            temp_node->status = -1;  // 将该进程设置为分配失败
            printf("进程 %d 分配失败\n", temp_node->pid);
            if (temp_node->pid == 9){
                printf("分配结束\n");
                break;  // 最后一个进程仍没被分配，则退出for循环，避免报错
            }
            goto check_point;  //没有满足条件，下一个
        }
        int rand_start = (rand() % (temp_block->size - temp_node->neededMem)) + temp_block->startAddr;  // 在区块的有效空间随机起始地址
        printf("随机分配到的开始地址 %d \n", rand_start);
        if(temp_block->size == temp_node->neededMem) {  
            // 找到恰好符合条件的，若空间大小刚好符合进程的内存需求大小，则可以将这分区设为占用
            temp_block->status=false;  // 分区被占用
            temp_block->pid = temp_node->pid;  // 分区记录进程的PID
            temp_node->status = 1;  // 进程被分配成功设置为1
            temp_node->blockID = temp_block->id;  // 进程记录分区序号
        }
        else{
            // 区块更大的情况，需要划分区块
            block * new_block = (block*)malloc(sizeof(block)); // 创建区块
            //(temp_block->prev)->next = new_block;  // 将上一区块的next指针指向新的区块
            printf("搜索到适合的区块id = %d\n", temp_block->id);
            // 其它信息未被修改，处理next新区块
            /*——————————被占用新区块——————————*/
            //temp_block->next = new_block;  DEBUG发现这句话写的位置不对
            new_block->id = cblock;  // 新区块ID在前一块的基础上+1
            new_block->size = temp_node->neededMem;  // 新区块大小即进程大小
            new_block->startAddr = rand_start;  // 新区块起始地址为随机生成的地址
            new_block->status = false;  // 区块设置为占用
            new_block->pid = temp_node->pid;  // 区块设置进程ID
            new_block->prev = temp_block;  // 分配区块prev指向原来被切割的区块
            temp_node->status = 1;  // 进程分配成功
            temp_node->blockID = new_block->id;  // 进程保存分区ID
            /*——————————未占用新区块——————————*/
            block * new_block_next = (block*)malloc(sizeof(block)); // 创建区块
            new_block->next = new_block_next;  // 被占用区块next指向新的未分配区块
            new_block_next->id = new_block->id + 1;
            new_block_next->startAddr = new_block->size + rand_start;
            new_block_next->size = (temp_block->startAddr + temp_block->size) - new_block_next->startAddr;
            new_block_next->status = true;
            new_block_next->pid = -1;
            new_block_next->prev = new_block;
            new_block_next->next = temp_block->next;
            /*——————————原来的区块——————————*/
            // 最后处理被划为三块的第一块的大小
            temp_block->next = new_block;
            temp_block->size = new_block->startAddr - temp_block->startAddr; 
            temp_block->status = true;
        } 
    }  // for end
}

void next_fit(block *p, Node *p1){
    srand((unsigned)time(NULL));  
    block *temp_block = p;  // NF相较于FF 区块不用重头开始搜索
    block *show_all = p;
    check_point:  // goto要避免重新执行头指针
    for (int i = 0; i < num_pro; i++){  // 循环程序个数次
        Node *temp_node = p1;  // 这里NF的节点仍然采用循环查找
        while(temp_node){  // 找到需要处理的进程
            if (temp_node->status == 0 && temp_node->blockID == -1){
                break;  // 找到既不是分配失败，也没有分配区块的进程
            }
            temp_node = temp_node->next;  // 保存之后要处理的进程的指针
        }
        display_block(show_all);  // 先展示当前所以区块
        printf("当前搜索到要处理的进程 %d , 所需要的内存空间是 = %d\n", temp_node->pid, temp_node->neededMem);
        int cblock = count_block(show_all);  // 统计区块大小，用来作为生成新区块ID的基础
        while (temp_block){  // 搜索有效区块
            printf("temp block id = %d\n", temp_block->id);
            if (temp_block->size > temp_node->neededMem && temp_block->status == true){  
                // 如果当前块的可用空间大于当前进程所申请的空间大小，并且块处于空闲状态
                break;  // 满足条件退出循环，可以得到满足条件的区块地址temp_block
            }
            temp_block = temp_block->next;  // 分区一个个往后查找，直到有满足条件的
            // 没有判断进程是否满足放入区块条件
        }
        // 判断当前进程是否能被放入区块
        if (temp_block==NULL && temp_node->status == 0){
            temp_node->status = -1;  // 将该进程设置为分配失败
            printf("进程 %d 分配失败\n", temp_node->pid);
            if (temp_node->pid == 9){
                printf("分配结束\n");
                break;  // 最后一个进程仍没被分配，则退出for循环，避免报错
            }
            goto check_point;  //没有满足条件，下一个
        }
        int rand_start = (rand() % (temp_block->size - temp_node->neededMem)) + temp_block->startAddr;  // 在区块的有效空间随机起始地址
        printf("随机分配到的开始地址 %d \n", rand_start);
        if(temp_block->size == temp_node->neededMem) {  
            // 找到恰好符合条件的，若空间大小刚好符合进程的内存需求大小，则可以将这分区设为占用
            temp_block->status=false;  // 分区被占用
            temp_block->pid = temp_node->pid;  // 分区记录进程的PID
            temp_node->status = 1;  // 进程被分配成功设置为1
            temp_node->blockID = temp_block->id;  // 进程记录分区序号
            temp_block = temp_block->next;  // NF 从分配的下一块开始
        }
        else{
            // 区块更大的情况，需要划分区块
            block * new_block = (block*)malloc(sizeof(block)); // 创建区块
            //(temp_block->prev)->next = new_block;  // 将上一区块的next指针指向新的区块
            printf("搜索到适合的区块id = %d\n", temp_block->id);
            // 其它信息未被修改，处理next新区块
            /*——————————被占用新区块——————————*/
            //temp_block->next = new_block;  DEBUG发现这句话写的位置不对
            new_block->id = cblock;  // 新区块ID在前一块的基础上+1
            new_block->size = temp_node->neededMem;  // 新区块大小即进程大小
            new_block->startAddr = rand_start;  // 新区块起始地址为随机生成的地址
            new_block->status = false;  // 区块设置为占用
            new_block->pid = temp_node->pid;  // 区块设置进程ID
            new_block->prev = temp_block;  // 分配区块prev指向原来被切割的区块
            temp_node->status = 1;  // 进程分配成功
            temp_node->blockID = new_block->id;  // 进程保存分区ID
            /*——————————未占用新区块——————————*/
            block * new_block_next = (block*)malloc(sizeof(block)); // 创建区块
            new_block->next = new_block_next;  // 被占用区块next指向新的未分配区块
            new_block_next->id = new_block->id + 1;
            new_block_next->startAddr = new_block->size + rand_start;
            new_block_next->size = (temp_block->startAddr + temp_block->size) - new_block_next->startAddr;
            new_block_next->status = true;
            new_block_next->pid = -1;
            new_block_next->prev = new_block;
            new_block_next->next = temp_block->next;
            /*——————————原来的区块——————————*/
            // 最后处理被划为三块的第一块的大小
            temp_block->next = new_block;
            temp_block->size = new_block->startAddr - temp_block->startAddr; 
            temp_block->status = true;
            temp_block = new_block_next;  // 把下次区块的搜索开始设为最新区块的下一块开始
        } 
    }  // for end
}
void free_block(block *p){
    block *temp = p;
    // 无论如何先对第一块区块单独回收
    temp->status = true;
    temp->pid = -1;
    printf("————————————————————————回收前检查————————————————————————\n");
    display_block(temp);
    temp = temp->next;  // 下一块
    printf("————————————————————————回收开始————————————————————————\n");
    int i = 1;
    while(temp){
        // 根据题目要求，当前区块与前一个区块都为空闲区块时，与前一个区块合并。
        if (temp->status == true && temp->prev->status == true){
            // 当前区块与前一区块同为空闲状态时
            printf("当前处理的块是 %d ，以及前一块 %d \n", temp->id, temp->prev->id);
            temp->prev->size = temp->prev->size + temp->size;  // 前一区块的大小加上当前的
            temp->prev->next = temp->next;  // 前一块的后继指向当前区块的后继
            if (temp->next != NULL){  // 如果处理到最后一个，是没有下一区块的
                temp->next->prev =temp->prev; 
            }
            block *temp1 = p;
            display_block(temp1);
        }
        else{
            // 当前区块被占用，先释放，再与前一区块（确保了前一区块必为空闲状态）合并
            printf("当前处理的块是 %d ，与前一块合并 %d \n", temp->id, temp->prev->id);
            temp->status = true;  // 设置成允许释放
            temp->pid = -1;
            temp->prev->size = temp->prev->size + temp->size;
            printf("%6d %14d \t\t%7d\t\tproc = %d\t\t status = %d\n", temp->id, temp->startAddr, temp->size, temp->pid, temp->status);
            printf("完成初始化\n");
            temp->prev->next = temp->next;
            if (temp->next != NULL){  // 如果处理到最后一个，是没有下一区块的
                temp->next->prev =temp->prev; 
            }
            block *temp2 = p;
            display_block(temp2);
        }
        temp = temp->next;
    }
    printf("————————————————————————回收结束————————————————————————\n");
    block *temp1 = p;
    display_block(temp1);
}

int main() {
    block * p = NULL;
    Node * p1 = NULL;
    p = create_block();
    // display_block(p);
    p1 = create_pcb();
    display_pcb(p1);
    // first_fit(p, p1);
    next_fit(p,p1);
    // free_block(p);
    return 0;
}