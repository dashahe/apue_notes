# 线程控制

主要涉及：
- 线程属性和同步原语属性
- 多个线程如何保持数据私有性
- 进程的系统调用如何和线程交互

## 线程限制
使用`sysconf`函数可以查看线程的各种限制。

## 线程属性
线程可以设置不同的属性，来调整线程的行为。前面`pthread_create`函数里面就有一个`attr`参数，这个就是用来设置属性的。

线程属性用`pthread_attr_t`结构来表示，这个需要用`pthread_attr_init`函数来初始化，用`pthread_attr_destory`函数来销毁。

对于Linux来说，支持四个线程属性：
- detachstate，设置线程的分离状态
- stackaddr，设置线程栈的低地址
- stacksize，设置线程栈的大小
- guardsize，设置线程警戒缓冲区的大小

线程的detach状态分为两种，`PTHREAD_CREATE_DETACHED`和`PTHREAD_CREATE_JOINABLE`。如果使用前者，那么线程开始就处于分离状态，就不用调用`pthread_detach`函数了。`pthread_attr_getdetachstate`和`pthread_attr_setdetachstate`是接口。

至于`stackaddr`是当我们想要自己手动设置线程栈的时候，就需要用到。比如，我们先`malloc`一块内存，然后分配给线程栈。可以用`pthread_attr_getstack`和`pthread_attr_setstack`，来进行操作。后面这些属性的接口都是类似。

`guardsize`控制线程栈末尾用来避免栈溢出的扩展内存的大小。如果我们修改了`stackaddr`，那么这个就会默认为0，因为系统认为我们采用了自己管理栈的方式。

## 同步属性
#### 互斥量属性
主要讲三个属性：共享属性、健壮属性和类型属性。

首先，共享内存是一种机制，可以把同一块内存映射给多个进程的各自独立的地址空间，实现多个进程共享内存。和多线程一样，这个需要一种同步方式来进行同步。共享属性默认为`PTHREAD_PROCESS_PRIVATE`，可以设置为`PTHREAD_PROCESS_SHARED`。如果设置为共享的，那么互斥量就可以用来处理这些进程的同步。api是`pthread_mutexattr_setpshared`等。

类型属性控制互斥量的锁定特性。
- PTHREAD_MUTEX_NORMAL，不做特殊的错误检查或死锁检测
- PTHREAD_MUTEX_ERRORCHECK，做错误检查
- PTHREAD_MUTEX_RECURSIVE，允许同一线程在互斥量解锁前对互斥量多次加锁，类似于shared_ptr，会有一个计数
- PTHREAD_MUTEX_DEFAULT，默认

递归型互斥量很有用。因为一般情况下，我们对同一个线程加锁两次，就会导致死锁。但是，如果采用了递归型的，就不会，因为它会设置一个计数器，互斥量没有上锁的时候计数器的值为0，这个时候线程才能够获得锁，而只有这个获得了锁的线程才能够lock()，每次lock一下计数器加1，unlock一直计数器减1，最后回到计数器为0，其他线程才能够上锁。

```c
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex;
pthread_mutexattr_t attr;

int main() {
    pthread_t t;

    pthread_mutexattr_init(&attr);
     //设置为递归型互斥锁
    // pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_DEFAULT);

    pthread_mutex_init(&mutex, &attr);

    pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&mutex);

    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&mutex);
}
```

从这个例子可以看到，如果采用默认的互斥锁，这里会死锁，如果采用递归型就不会。

#### 读写锁属性
读写锁只有一个属性：进程共享属性。和互斥量的进程共享属性相同。`pthread_rwlockattr_getpshared`和`pthread_rwlockattr_setpshared`。

#### 条件变量属性
只有两个属性：进程共享属性和时钟属性。时钟属性用来控制计算`pthread_cond_timedwait`函数的超时参数时，采用哪一个时钟。

#### 屏障属性
同样只有进程共享属性。


**可以看到，这几种同步原语唯一都拥有的属性就是进程共享属性。该属性用来控制这些同步原语，是可以被多进程的线程使用（SHARED），还是只能被单进程的多线程使用（PRIVATE）。**

## 重入
线程安全：如果一个函数在相同的时间可以被多个线程安全调用。
异步信号安全：如果一个函数对异步信号处理程序的重入式安全的。

线程安全是对于多线程来说的，而异步信号安全是单线程。一般，如果一个函数是线程安全的，但是不一定是信号处理程序可重入的，但是反过来则不同。

如果一个函数中有全局变量，那么一定是不可重入的。

## 线程私有数据
主要是两个东西：`pthread_key_t`和`pthread_once_t`。

有时候，我们需要每个线程拥有自己的私有数据，而不是共享，这就需要`pthread_key_t`来实现这个目的。首先，用`pthread_key_create`函数来创建key，这个函数可以设置key的析构函数，这个函数在线程exit的时候执行来回收资源。`pthread_key_getspecific`用来查找key关联的资源，`pthread_key_setspecific`用来创建关联。而`pthread_key_delete`用来解除关联。

如果多个线程都调用了同一个函数，那么这个函数里面的代码会被重复执行很多次。但是，有时候有些代码只需要执行一次，就需要用到`pthread_once_t`。注意，这个`pthread_once_t`一定要是全局或者晶态变量。

例子：
```
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define MAXSTRINGSZ 4096

static pthread_key_t key;
static pthread_once_t init_done = PTHREAD_ONCE_INIT;
pthread_mutex_t env_mutex = PTHREAD_MUTEX_INITIALIZER;

extern char **environ; //环境表

//创建key，只应该调用一次
static void thread_init(void) {
    pthread_key_create(&key, free);
}

char *getenv(const char *name) {
    int i, len;
    char *envbuf;

    pthread_once(&init_done, thread_init);

    pthread_mutex_lock(&env_mutex);
    envbuf = (char*)pthread_getspecific(key);

    //给每个线程设置自己的私有数据
    if (envbuf == NULL) {
        envbuf = malloc(MAXSTRINGSZ);
        if (envbuf == NULL) {
            pthread_mutex_unlock(&env_mutex);
            return NULL;
        }
        pthread_setspecific(key, envbuf);
    }

    len = strlen(name);
    for (i = 0; environ[i] != NULL; ++i) {
        if ((strcmp(name, environ[i], len) == 0) && (environ[i][len] == '=')) {
            strncpy(envbuf, &envbuf[i][len+1], MAXSTRINGSZ-1);
            pthread_mutex_unlock(&env_mutex);
            return envbuf;
        }
    }
    pthread_mutex_unlock(&env_mutex);
    return NULL;
}
```

## 取消选项
两个线程属性：可取消状态和可取消类型。
可取消状态可以设置为`PTHREAD_CANCEL_ENABLE`或者`PTHREAD_CANCEL_DISABLE`。
可取消类型可以设置为`PTHREADCANCEL_DEFERRED`或者`PTHREAD_CANCEL_ASYNCHRONOUS`。

之前提到过`pthread_cancel`函数，这个函数会对待取消的线程发起取消请求，但是不会等到那个线程真正取消。如果那个线程的可取消状态设置成了不可取消，那么`pthread_calcel`函数发送的这个请求就没有用。

如果可以取消，一般会等到某个取消点才会取消。更改取消类型可以改编这个，如果将取消类型设置为异步的，那么随时可以取消，不用等到下个取消点。

## 线程和信号
每个线程都有自己的信号屏蔽字，但是信号的处理函数是所有线程共享的。

进程的信号只传递给一个线程，一般是任意选的，除非是硬件故障。

线程用`pthread_sigmask`来设置屏蔽字，`sigwait`来等待信号出现，`pthread_kill`来发送信号给线程。

## 线程和fork
子进程会copy整个地址空间，所有互斥量、读写锁、条件变量的状态也会被继承。子进程内部，只有一个线程，就是父进程调用fork的那个线程的副本。

但是这样就会有问题，因为子进程并没有占用锁的那个线程的副本，就会导致死锁。所以，fork需要由一些处理函数，把一些锁都给关掉。

清除锁的状态的函数是`pthread_atfork`，该函数会在调用fork的时候自动调用设置的三个清理函数。这个三个清理函数分别是`prepare`、`parent`和`child`，都是`pthread_atfork`函数的参数。

- prepare会在fork创建子进程之前调用，用来获取所有父进程定义的锁。
- parent会在fork完了之后，返回给父进程的时候调用，会清除父进程所有的锁。
- child会在fork完了之后，返回给子进程的时候调用，会清除子进程的所有锁。
  
可以用`pthread_atfork`调用多次来设置多套fork处理程序，那么prepare的调用顺序和设置顺序相反，另外两个和注册的顺序相同。

即使这样，仍然有很多fork调用的问题。比如：
- 不能处理较复杂的同步对象，如条件变量、屏障。
- 某些错误检查的互斥量实现会问题
- 递归互斥量可能出问题
等等等等。

详细看[test4.c](./test4.c)

## 线程和IO
`pread`和`pwrite`函数可以满足多线程正常使用，而普通的`read`就可能导致错误。