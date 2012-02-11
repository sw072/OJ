#ifndef MSGDEFS_H
#define MSGDEFS_H

typedef struct _solution
{
	int pid;
	unsigned long time_lmt;
	unsigned long memo_lmt;
	unsigned long src_len;
	//char src[]
} solution;

typedef struct _judger_result
{
	int pid;
	int result;
	union _info
	{
	    struct
	    {
	        unsigned long A, B;
	    } __bitmap__;	
		struct
		{
			unsigned long time;
			unsigned long memory;
		}info_ac;
		struct
		{
			unsigned long signo;
			unsigned long padding;
		}info_rt;
		struct
		{
			unsigned long cerr_size;
			unsigned long padding;
		}
	}info;
	//char compile_error[]
}judger_result;

#endif
