#define _GNU_SOURCE
#include <dlfcn.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <libgen.h>

//#define HOOK_SYSCALLS
#ifdef HOOK_SYSCALLS
#include <libsyscall_intercept_hook_point.h>
#include <syscall.h>
#endif // HOOK_SYSCALLS

//#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define	dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...) do { if (0) printf(__VA_ARGS__); } while (0)
#endif

#undef pthread_create

typedef int (*__pthread_create_fn)(pthread_t *thread,
		const pthread_attr_t *attr,
		void *(*start_routine) (void *),
		void *arg);

static __pthread_create_fn orig_pthread_create = 0;

char *addr_to_lib(void *addr, unsigned long *offset_in_lib)
{
	char maps_path[PATH_MAX];
	FILE * fp;
	void *start, *end;
	char perms[4];
	unsigned long offset;
	unsigned long dev[2];
	int inode;
	char path[PATH_MAX];

	sprintf(maps_path,"/proc/self/maps");
	fp = fopen(maps_path, "r");
	if (fp == NULL) {
		fprintf(stderr,"error: cannot open the memory maps, %s\n",
				strerror(errno));
		return NULL;
	}

	memset(path, 0, sizeof(path));
	while (fscanf(fp, "%012lx-%012lx %4s %lx %lx:%lx %d%[^\n]",
				(unsigned long *)&start,
				(unsigned long *)&end,
				perms, &offset, &dev[0], &dev[1], &inode, path) != EOF) {

		if (start <= addr && end > addr) {
			fclose(fp);
			if (offset_in_lib)
				*offset_in_lib = (unsigned long)(addr - start);
			return strlen(path) > 0 ?
				strdup(&path[strspn(path, " \t")]) : NULL;
		}

		memset(path, 0, sizeof(path));
	}

	fclose(fp);

	return NULL;
}

struct __thready_arg {
	void *(*start_routine)(void *);
	void *arg;
	int cpu;
};

void *__thready_start_routine(void *arg)
{
	char *lib = NULL;
	unsigned long offset = 1;
	struct __thready_arg *thready_arg = (struct __thready_arg *)arg;
	void *__arg = thready_arg->arg;
	void *(*__start_routine)(void *) = thready_arg->start_routine;

	lib = addr_to_lib(__start_routine, &offset);

	printf("[TID: %ld]: %p is in %s @ offset: %lu\n",
			syscall(__NR_gettid),
			__start_routine,
			lib ? basename(lib) : "(unknown)",
			offset);

	if (lib)
		free(lib);

	free(thready_arg);
	return __start_routine(__arg);
}

int pthread_create(pthread_t *thread,
		const pthread_attr_t *attr,
		void *(*start_routine) (void *),
		void *arg)
{
	int ret;
	struct __thready_arg *thready_arg;

	if (!orig_pthread_create) {
		orig_pthread_create =
			(__pthread_create_fn)dlsym(RTLD_NEXT, "pthread_create");
	}
	
	thready_arg = malloc(sizeof(*thready_arg));
	if (!thready_arg) {
		goto out;
	}

	thready_arg->start_routine = start_routine;
	thready_arg->arg = arg;
	thready_arg->cpu = -1;

	start_routine = __thready_start_routine;
	arg = thready_arg;

out:
	ret = orig_pthread_create(thread, attr, start_routine, arg);
	return ret;
}

#ifdef HOOK_SYSCALLS
static int hook(long syscall_number,
		long arg0, long arg1,
		long arg2, long arg3,
		long arg4, long arg5,
		long *result)
{
	if (syscall_number == SYS_clone) {
		printf("%s: SYS_clone: 0x%lx\n",
			__func__, arg0);
	}

	return 1;
}


static __attribute__((constructor)) void init(void)
{
	// Set up the callback function
	intercept_hook_point = hook;
}
#endif // HOOK_SYSCALLS
