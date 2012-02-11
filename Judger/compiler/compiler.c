#include "compiler.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


extern char work_dir[PATH_MAX];
static const char cerr_file_path[] = "./compiler/cerr.txt";

int run_compiler(lang_t lang, char *src_path, char *exe_path)
{
	int res = 0;
	const char *argv[8] = {NULL};
	//DBG("compile command:");
	switch(lang)
	{
	case eC:
		argv[0] = "gcc";
		argv[1] = "-w";
		argv[2] = src_path;
		argv[3] = "-o";
		argv[4] = exe_path;
		argv[5] = NULL;
		//DBG("%s %s %s %s %s", argv[0], argv[1], argv[2], argv[3], argv[4]);
		res = execv("/usr/bin/gcc", argv);
		break;
	case eCpp:
		argv[0] = "g++";
		argv[1] = "-w";
		argv[2] = src_path;
		argv[3] = "-o";
		argv[4] = exe_path;
		argv[5] = NULL;
		//DBG("%s %s %s %s %s", argv[0], argv[1], argv[2], argv[3], argv[4]);
		res = execv("/usr/bin/g++", argv);
		break;
	case ePascal:
		//res = execv("");
		break;
	case eJava:
	{
		char class_dir[PATH_MAX];
		strcpy(class_dir, exe_path);
		char *p = class_dir + strlen(class_dir);
		while(*(--p) != '/');
		*p = '\0';
		argv[0] = JAVAC_DIR;
		//argv[1] = "-g:none";
		argv[1] = "-nowarn";
		argv[2] = src_path;
		argv[3] = "-d";
		argv[4] = class_dir;
		argv[5] = NULL;
		//DBG("%s %s %s %s %s", argv[0], argv[1], argv[2], argv[3],
		//	argv[4]);
		res = execv(JAVAC_DIR, argv);
		break;
	}
	case ePython:
		//res = execv("python");
		break;
	default: res = -1;
	}
	return res;
}

compile_res_t compile(lang_t lang, char *src_path, char *exe_path, char *cerr_info)
{
	pid_t pid = 0;
	if(access(cerr_file_path, F_OK) == -1)
	{
		if(creat(cerr_file_path, 0666))
		{
		    DBG("create compile error file failed.");
			return eCIe;
		}
	}
	else
	{
		if(truncate(cerr_file_path, 0))    //clear the compile error information file
		{
		    DBG("clear compile error file failed.");
		    return eCIe;
		}
	}
	int cerr_fd = open(cerr_file_path, O_WRONLY);
	if(cerr_fd < 0)
	{
		DBG("open compile error file failed.");
		return eCIe;
	}
	strcpy(exe_path, work_dir);
	strcat(exe_path, "exe/");
	switch(lang)
	{
	case eC:
	case eCpp:
	case ePascal: strcat(exe_path, "main"); break;
	case eJava: strcat(exe_path, "Main.class"); break;
	case ePython: strcat(exe_path, "main.pyc"); break;
	default: break;
	}    //[work_dir]/exe/[exe_file]
	DBG("exe_path:%s", exe_path);
	if((pid = fork()) == 0) /* create compiler process */
	{
		dup2(cerr_fd, STDOUT_FILENO);
		dup2(cerr_fd, STDERR_FILENO);
		if(run_compiler(lang, src_path, exe_path))
		{
		    DBG("run_compiler error.");
			return eCIe;
		}
	}
	else if(pid < 0)
	{
	    DBG("create compiler process failed.");
	    return eCIe;
    }
	wait(pid);
	close(cerr_fd);

	unsigned long cerr_file_size = 0;
	struct stat buf;
	if(stat(cerr_file_path, &buf) < 0)
	{
		return eCIe;
	}
	cerr_file_size = (unsigned long)buf.st_size;
	if(!cerr_file_size)
	{
		if(access(exe_path, F_OK) == -1)
		{
		    DBG("create exe file failed.");
		    return eCIe;
		}
		return eCOk;
	}
	cerr_fd = open(cerr_file_path, O_RDONLY);
	int n = read(cerr_fd, cerr_info, cerr_file_size);
	close(cerr_fd);
	return eCErr;
}

//gcc -w /home/shiwei/libsandbox_test/Judger/bin/Debug/src/main.c -o /home/shiwei/libsandbox_test/Judger/bin/Debug/exe/main
