#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "comm/msgdefs.h"
#include "compiler/compiler.h"
#include "sandbox/sandbox.h"

char work_dir[PATH_MAX] = "work dir";
//const char out_file_path[] = "out/out.txt";

/**
 * @brief Types of judger execution result.
 */
typedef enum
{
    ePD = 0,    /*!< Pending */
    eCE = 1,    /*!< Compile Error */
    eAC = 2,    /*!< Okay */
	eWA = 3,    /*!< Wrong Answer */
    eRF = 4,    /*!< Restricted Function */
    eML = 5,    /*!< Memory Limit Exceed */
    eOL = 6,    /*!< Output Limit Exceed */
    eTL = 7,    /*!< Time Limit Exceed */
    eRT = 8,    /*!< Run Time Error (SIGSEGV, SIGFPE, ...) */
    eAT = 9,    /*!< Abnormal Termination */
    eIE = 10,   /*!< Internal Error */
} judge_res_t;


#define CEE_INFO_SIZE 4096
char cerr_info[CEE_INFO_SIZE];

int compare(int pid, const char *out_file_path)
{
	char ans_byte, out_byte;

	char answer_file_path[PATH_MAX];
	sprintf(answer_file_path, "%s%s%d.ans", work_dir, "answers/", pid);
	DBG("answer file path:%s", answer_file_path);
	DBG("output file path:%s", out_file_path);
	if(access(answer_file_path, 0) || access(out_file_path, 0))
	{
	    DBG("answer file or output file doesn't exist.");
	    return -2;
	}
	unsigned long file_size = 0;
	struct stat buf;
	if(stat(answer_file_path, &buf) < 0)
	{
		return -2;
	}
	file_size = (unsigned long)buf.st_size;
	if(stat(out_file_path, &buf) < 0)
	{
		return -2;
	}
	if(file_size != (unsigned long)buf.st_size)
	{
		return -1;
	}
	int answer_fd, out_fd;
	if((answer_fd = open(answer_file_path, O_RDONLY)) < 0) return -2;
	if((out_fd = open(out_file_path, O_RDONLY)) < 0) return -2;
	unsigned long i = 0;
	for(i = 0; i < file_size; i++)
	{
		if(read(answer_fd, &ans_byte, 1) != 1
			|| read(out_fd, &out_byte, 1) != 1)
			return -2;
		if(ans_byte != out_byte) return -1;
	}
	return 0;
}

judge_res_t run_judge(lang_t lang, int pid, char *exe_path,
			unsigned long time, unsigned long memory, stat_t *pru)
{
	char in_file_path[PATH_MAX], out_file_path[PATH_MAX];
	sprintf(in_file_path, "%s%s%d.in", work_dir, "inputs/", pid);
	sprintf(out_file_path, "%s%s", work_dir, "out/out.txt");
	DBG("input file path:%s", in_file_path);
	DBG("output file path:%s", out_file_path);
	if(access(in_file_path, F_OK))
	{
	    DBG("input file doesn't exist.");
	    return eIE;
    }
	if(access(out_file_path, F_OK))
	{
	    /*
		if(creat(out_file_path, 0666))
		{
		    DBG("create output file failed.");
			return eIE;
		}
		*/
	    DBG("output file doesn't exist.");
	    return eIE;
	}
	else
	{
		if(truncate(out_file_path, 0))    //clear the temp output file
		{
		    DBG("clear out.txt file failed.");
		    return eIE;
		}
    }
	int in_fd, out_fd;
	if((in_fd = open(in_file_path, O_RDONLY)) < 0) return eIE;
	if((out_fd = open(out_file_path, O_WRONLY)) < 0) return eIE;
	const char *argv[8] = {NULL};
	switch(lang)
	{
	case eC:
	case eCpp:
	case ePascal:
		argv[0] = exe_path;
		argv[1] = NULL;
		break;
	case eJava:
	{
		char class_dir[PATH_MAX];
		strcpy(class_dir, exe_path);
		char *p = class_dir + strlen(class_dir);
		while(*(--p) != '/');
		*p = '\0';
		argv[0] = JAVA_DIR;
		argv[1] = "-classpath";
		argv[2] = class_dir;
		argv[3] = "Main";
		argv[4] = NULL;
		break;
	}
	case ePython:
		argv[0] = "python";
		argv[1] = "main.pyc";
		argv[2] = NULL;
		break;
	default: break;
	}
	result_t run_res = S_RESULT_PD;
	judge_res_t judge_res = ePD;
	sandbox_t sbx;
	sandbox_init(&sbx, argv);
	sbx.task.ifd = in_fd;
	sbx.task.ofd = out_fd;
	sbx.task.quota[S_QUOTA_CPU] = time;
	sbx.task.quota[S_QUOTA_MEMORY] = memory;
	sbx.task.quota[S_QUOTA_DISK] = 2 * 1024 * 1024;
	sandbox_execute(&sbx);
	close(in_fd);
	close(out_fd);
	run_res = sbx.result;
	*pru = sbx.stat;
	sandbox_fini(&sbx);
	switch(run_res)
	{
	case S_RESULT_OK:
	{
		int compare_res = compare(pid, out_file_path);
		if(compare_res == 0)
			judge_res = eAC;
		else if(compare_res == -1)
			judge_res = eWA;
		else judge_res = eIE;
		break;
	}
	case S_RESULT_RF: judge_res = eRF; break;
	case S_RESULT_ML: judge_res = eML; break;
	case S_RESULT_OL: judge_res = eOL; break;
	case S_RESULT_TL: judge_res = eTL; break;
	case S_RESULT_RT: judge_res = eRT; break;
	case S_RESULT_AT: judge_res = eAT; break;
	case S_RESULT_IE: judge_res = eIE; break;
	default: judge_res = eIE;
	}
	return judge_res;
}

int save_src(lang_t lang, char *src, char *src_path)
{
	strcpy(src_path, work_dir);
	strcat(src_path, "src/");
	switch(lang)
	{
	case eC: strcat(src_path, "main.c"); break;
	case eCpp: strcat(src_path, "main.cpp"); break;
	case ePascal: strcat(src_path, "main.pascal"); break;
	case eJava: strcat(src_path, "Main.java"); break;
	case ePython: strcat(src_path, "main.py"); break;
	default: break;
	}    //[work_dir]/src/[src_file]
	if(access(src_path, F_OK) == -1)
	{
		if(creat(src_path, 0666) == -1)
			return -1;
	}
	else
	{
		/* clear the source file */
		if(truncate(src_path, 0))
			return -1;
	}
	int src_fd = open(src_path, O_WRONLY);
	if(src_fd < 0) return -1;
	int n = write(src_fd, src, strlen(src));
	close(src_fd);
	if(n != strlen(src)) return -1;
	return 0;
}

void test(int *ppid, lang_t *plang, char *src)
{

    *ppid = 1000;
	*plang = eC;
    strcpy(src,\
"#include <stdio.h> \n\
int main(void) \n\
{ \
    printf(\"Hello World!\");\n\
    return 0;\n\
}\n");

/*
    *ppid = 1000;
    *plang = eJava;
    strcpy(src,\
"import java.util.Scanner;\n   \
public class Main\n\
{\n\
public static void main(String args[])\n\
    {\n\
        Scanner cin=new Scanner(System.in);\n\
        int a;\n\
        while(cin.hasNextInt()) {\n\
            a = cin.nextInt();\n\
            System.out.print(a);\n\
        }\n\
    }\n\
}");
*/
}

const char *s_judge_result_name(int res)
{
    static const char * table[] =
    {
        "PD", /* 0 */
        "CE", /* 1 */
        "AC", /* 2 */
        "WA", /* 3 */
        "RF", /* 4 */
        "ML", /* 5 */
        "OL", /* 6 */
        "TL", /* 7 */
        "RL", /* 8 */
        "AT", /* 9 */
        "IE", /* 10 */
    };
    return table[res];
}


void print_judger_result(judger_result *pjres)
{
    printf("result : %s\t", s_judge_result_name(pjres->result));
    if(pjres->result == (int)eCE)
    {
        printf("\n%s", pjres->compile_error);
    }
    if(pjres->result == (int)eAC)
    {
        printf("time : %ldms\tmemory : %ldKB", pjres->info.info_ac.time, pjres->info.info_ac.memory);
    }
    else if(pjres->result == (int)eRT)
    {
        printf("sig : %d", pjres->info.info_rt.signo);
    }
    printf("\n");
}

int start_recv_thread()
{
}

int main(int argc, char *argv[])
{

	int pid = -1;                 /* problem id */
	lang_t lang;                  /* language type */
	judge_res_t judge_res = ePD;  /* judge result */
	stat_t stat;                  /* staticstic */
    if(getcwd(work_dir, sizeof(work_dir)-2) == NULL)
    {
        DBG("get current directory failed.");
		return -1;
    }
	int tail = strlen(work_dir);
	if(work_dir[tail - 1] != '/')
	{
	    work_dir[tail] = '/';
		work_dir[++tail] = '\0';
	}
	DBG("%s", work_dir);
	char src[PATH_MAX] = "src code";
	char src_path[PATH_MAX] = "src file path";    //absolute path
	char exe_path[PATH_MAX] = "exe file path";    //absolute path

    test(&pid, &lang, src);
    printf("%s\n", src);

	//start_recv_thread();		//start source code receiver thread
	//while(/*recv src*/)
	{
		if(save_src(lang, src, src_path/*out*/))
		{
		    DBG("save src failed.");
		    return -1;
		}
		DBG("src_path:%s", src_path);
		compile_res_t compile_res = compile(lang, src_path, exe_path/*out*/, cerr_info/*out*/);
		DBG("compile result:%d", compile_res);
		judger_result *pjres;
		if(compile_res == eCIe)
		{
			judge_res = eIE;
			pjres = (judger_result *)malloc(sizeof(judger_result));
			pjres->result = judge_res;
		}
		else if(compile_res == eCErr)
		{
			judge_res = eCE;
			int cerr_size = strlen(cerr_info);
			pjres = (judger_result *)malloc(sizeof(judger_result) + cerr_size + 1);
			pjres->result = judge_res;
			strcpy(pjres->compile_error, cerr_info);
			DBG("compile error:%s", cerr_info);
		}
		else if(compile_res == eCOk)
		{
			/* run judge */
			judge_res = run_judge(lang, pid, exe_path, 20000, 1024 * 1024 * 1024, &stat);
			pjres = (judger_result *)malloc(sizeof(judger_result));
			pjres->result = judge_res;
			switch(judge_res)
			{
            case eAC:
            {
                pjres->info.info_ac.time = stat.ru.ru_utime.tv_sec + stat.ru.ru_utime.tv_usec / 1000
                                         + stat.ru.ru_stime.tv_sec + stat.ru.ru_stime.tv_usec / 1000;
                pjres->info.info_ac.memory = (unsigned long)stat.vsize_peak / 1000;
                break;
            }
            case eWA:
            case eRF:
            case eML:
            case eOL:
            case eTL:
            case eAT:
            case eIE: break;
            case eRT:
            {
                pjres->info.info_rt.signo = stat.signal;
                break;
            }
            default: break;
			}
			pjres->pid = pid;
			if(remove(exe_path))
			{
			    DBG("remove exe file failed.");
			    return -1;
			}
		}
		else
		{
			/* error */
			DBG("unknow compile return value.");
			return -1;
		}
		print_judger_result(pjres);
		free(pjres);
	}

	return 0;
}
