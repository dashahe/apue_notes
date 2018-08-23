# 进程环境

## 进程终止
8种方式，5种正常终止，3种异常终止。但是，执行一个程序的方式只有一种：`exec()`函数。

正常终止：
- main函数返回
- 调用exit
- 调用_exit
- 最后一个线程从其启动例程返回
- 从最后一个线程调用pthread_exit

异常终止：
- 调用abort
- 接受到一个signal
- 最后一个线程对取消请求作出响应

![pic1]()

不同终止方式可以得到不同的终止码，可以用`echo $?`查看。

另外，每个进程可以登记32个函数，用来exit()自动调用。可以用`int atexit(void (*func)(void))`函数来设置这些函数。

## 环境表
每个程序都有一个environment table，用一个二维char指针存储。可以用`extern char **environ;`来使用。其中，都是`name=value`的形式的字符串。

![pic2]()

## 存储空间布局
.text, .data, .bss, .heap, .stack五个部分，加上上面一点点空间存储命令行参数和环境变量。data段存储全局变量，static等已经初始化了的值，而.bss则是没有初始化的，会由`exec()`函数来初始化为0。

可以用`size命令来获取程序的text, data, bss长度:
```
➜  code size /bin/ls 
   text	   data	    bss	    dec	    hex	filename
 120528	   4664	   4552	 129744	  1fad0	/bin/ls
```

![pic3]()

## 共享库
就是让可执行文件不需要包含所使用的公共库函数，在所有进程可使用的存储区中保存这种库例程的副本。这样做的代价是，程序第一次运行时或者第一次调用库函数时，用动态链接的方式来将程序和共享库函数链接起来。

这样做 **大大节省空间，可执行文件小，但是第一次慢一点**。

`gcc`编译是加上`-static`选项，可以阻止使用共享库。

```
➜  environment gcc jump.c          
➜  environment size
   text	   data	    bss	    dec	    hex	filename
   2002	    620	    456	   3078	    c06	a.out
➜  environment gcc jump.c --static
➜  environment size
   text	   data	    bss	    dec	    hex	filename
721770	   8284	   6464	 736518	  b3d06	a.out
```

## 存储空间分配
三个函数：`malloc`, `calloc`和`realloc`。
![]()

## setjmp和longjmp
`goto`只能够在同一个函数里面跳转，而`longjmp`和`setjmp`用来实现长跳转。因为需要在函数之间跳转，那么会涉及到跳转到上面的栈帧。
而在跳转过程中，register和自动变量不保证能够回滚到目的栈帧的版本，而static，全局和valtile变量保证一定不会回滚。
```c
#include <setjmp.h>
int setjump(jmp_buf env); //直接调用返回0，如果从longjmp中返回，返回值为val。类似于fork()函数返回值来区分不同情况
void longjmp(jmp_buf env, int val);
```

最后，`limit`命令可以用来获取程序的资源限制值。C语言中可以用`setrlimit()`和`getrlimit`函数来操作资源限制。
例如：
```
➜  ~ limit
cputime         unlimited
filesize        unlimited
datasize        unlimited
stacksize       8MB
coredumpsize    0kB
memoryuse       unlimited
maxproc         30412
descriptors     1024
memorylocked    64kB
addressspace    unlimited
maxfilelocks    unlimited
sigpending      30412
msgqueue        819200
nice            0
rt_priority     0
rt_time         unlimited

```
