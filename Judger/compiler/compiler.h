#include <sys/fcntl.h>
#include "../sandbox/sandbox.h"

#define JAVAC_DIR "/home/shiwei/jdk1.6.0_23/bin/javac"
#define JAVA_DIR "/home/shiwei/jdk1.6.0_23/bin/java"
#define PYTHON_DIR ""

typedef enum
{
	eC,
	eCpp,
	ePascal,
	eJava,
	ePython,
} lang_t;

typedef enum
{
	eCOk,
	eCErr,
	eCIe,
} compile_res_t;

compile_res_t compile(lang_t lang, char *src_path, char *exe_path, char *cerr_info);
