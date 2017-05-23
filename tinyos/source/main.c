#define NVIC_INT_CTRL	 0xE000Ed04
#define NVIC_PENDSVSET	 0x10000000
#define NVIC_SYSPRI2 	 0xE000ED22
#define NVIC_OENDSV_PRI  0x000000FF

#define MEM32(addr) 	*(volatile unsigned long*)(addr)
#define MEM8(addr) 		*(volatile unsigned char*)(addr)
	
void triggerPendSVC(void)
{
	MEM8(NVIC_SYSPRI2) = NVIC_OENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

typedef struct _BlockType_t
{
	unsigned long *stackpPtr;
}BlockType_t;

BlockType_t *blockPtr;

void delay(int count)
{
	while(--count);
}

unsigned long stackBuffer[1024];
BlockType_t block;

int flag;
int main()
{
	block.stackpPtr = &stackBuffer[1024];
	blockPtr = &block;
	while(1)
	{
		flag = 0;
		delay(100);
		flag = 1;
		delay(100);
		triggerPendSVC();
	}
}
