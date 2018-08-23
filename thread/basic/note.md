# 线程基础知识

## 概念
一般的进程可以看作只有一个控制线程，一个进程某个时刻只做一件事情。但是，如果用线程，将会有这些优点：
- 每个时间类型分配单独的处理线程，简化处理异步事件的coding
- 多个进程间通信十分复杂，共享内存、文件描述符很麻烦，线程则是共享的
- 并行化地解决问题
- 交互程序可以多线程来改善响应时间

多线程的好处并不只有在多处理器的条件下才能体现，就算是单线程，要是某些线程阻塞时，还可以让其他线程去做事情。

每个线程单独的信息：线程id、一组寄存器值、栈、调度优先级、信号屏蔽字、errno变量、私有化数据。

一个进程的所有信息都对这个进程的所有线程共享：可执行代码、全局内存和堆内存、栈、文件描述符。

## 线程标识
每个线程都有一个线程id，但是线程id只在它所在的进程上下文中才有意义。

线程id是`pthread_t`类型，Linux里面是一个`unsigned long`类型。

`int pthread_equal(pthread_t t1, pthread_t t2)`、`int pthread_self()`用来比较和得到线程id。

## 线程创建
```c
int pthread_create(
    pthread_t *tidp,
    const pthread_attr_t *attr,
    void *(*start_rtn)(void *),
    void *args
);
// 函数用来创建线程，四个参数。
// 第一个用来存储新线程的tid，第二个和第四个一般为NULL，第三个参数用来设置线程创建后程序开始执行的地方
```

## 线程终止
如果一个进程中任何一个线程调用了exit、_Exit或者_exit，那么整个进程会终止。另外，如果任何一个线程受到了默认动作是终止进程的信号，整个进程也会终止。

单个线程退出的三种方式：
- 从启动例程中return了
- 被同一个进程中其他线程cancel了
- 调用pthread_exit

`void pthread_exit(void *rval_ptr)`中的参数会作为返回值，被其他线程读取。读取的方式是通过`pthread_join(pthread_t thread, void **rval_ptr)`函数。调用join后，调用线程会阻塞，直到join了的线程退出。

`pthread_cancel(pthread_t tid)`用来让其他线程退出，但只是发送了请求，不会等待那个线程退出。

线程还可以安排退出时的函数，叫做 **线程清理处理函数**。
```c
void pthread_cleanup_push(void (*rtn)(void *), void *args);
void pthread_cleanup_pop(int execute);
```

默认情况下，线程的终止状态会保存到该线程被pthread_join。但是，还有另外一种情况，就是线程被`pthread_detach(pthread_t tid)`分离出去，那么线程的底层存储资源可以在线程终止时被立即回收。

## 线程同步
如果多个线程，对同一个变量进行读写或者写写操作，那么就可能产生竞争，这个时候就需要同步线程。

#### 互斥量
可以用互斥量来同步。互斥量本质上是一把锁，在访问共享资源前堆互斥量加锁，访问结束后解锁。
`pthread_mutex_t`是互斥量的类型，如果动态分配互斥量，那么需要调用`pthread_mutex_init`函数来初始化，释放时需要`pthread_mutex_destory`来销毁。

`pthread_mutex_lock`、`pthread_mutex_unlock`和`pthread_mutex_trylock`都是用来解锁加锁的。不同的是，如果不希望线程被阻塞，那么就用`pthread_mutex_trylock`，因为如果已经加锁，那么调用这个函数的时候不会阻塞，会直接返回。

另外，还有一种锁，可以定时。`pthread_mutex_timelock`函数来加锁，如果能够在设定的绝对时间内加锁，也就是如果阻塞超过一段时间就会返回。这样可以用来避免永久阻塞的情况。

#### 读写锁
因为多个线程同时读一个变量是没有问题的，如果读写或者写写就会出现竞争。上面的`mutex`并行性较低，用读写锁可以更高并行。读写锁有两种加锁，`pthread_rwlock_rdlock`和`pthread_rwlock_wrlock`，解锁只有`pthread_rwlock_unlock`。如果已经加了写锁，那么任何锁都会被阻塞，但是如果是加了读锁，就只有写锁会被阻塞。一般来说，有读者优先，也有写者优先，而pthread采用的是写者优先策略。

同样读写锁也有trylock和timelock。

#### 条件变量
`pthread_cond_t`表示条件变量，它给多个变量提供了一个会合的场所。`pthread_cond_wait`会让所有线程都阻塞，只有使用了`pthread_cond_signal`或`pthread_cond_broadcast`后才会解除。

另外，一个条件变量需要用一个互斥量来保护，作为参数传入`pthread_cond_wait`函数就好了。

#### 自旋锁
和互斥量类似，但是自旋锁不是通过休眠来阻塞，而是在lock之前一直保持忙等阻塞状态，也就是自旋状态。优点是，不需要频繁的休眠线程，适用于锁被持有时间很短的情况。自旋锁一般用在底层，实现其他的锁。

对于非抢占时内核很有用，但是对于用户作用不大。

`pthread_spin_lock`，接口类似于互斥量。

#### 屏障
barrier就真的像一个屏障，把线程都堵在`pthread_barrier_wait`的地方，除非阻塞的线程数量到了count，而这个count在`pthread_barrier_init`里面设置。其实，`pthread_join`就是一种屏障，允许一个线程等待，直到另一个线程退出。

从直观上来看，barrier更像是大坝，把线程都阻塞起来，同步一下。

这里有一个[多线程的排序](./multi_thread_sort.cc)，用到了屏障来同步线程。虽然速度并没有单线程快...可能是STL太快了。


