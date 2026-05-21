# CPU亲和性相关知识点
## 什么是CPU亲和性？ 
CPU亲和性(affinity)就是进程要在某个给定的CPU上尽量长时间的运行而不被迁移到其他处理器的倾向性。linux内核进程调度器天生具有软CPU亲和性（affinity）的特性，这意味着进程通常不会在处理器之间频繁迁移。这种状态正是我们希望的，因为进程迁移的频率小就意味着产生的负载小。
Linux内核还包含一些机制，它让开发人员可以编程实现硬CPU亲和性（affinity）。着意味着应用程序可以显示的指定进程在那个（或那些）处理器上运行。  
查看CPU具有几个核的命令：  
`$ cat /proc/cpuinfo |grep processor | wc -l`或  
`$ nproc`  
## cpu集(cpu_set_t)
cpu_set_t用来描述CPU的集合 ，被sched_setaffinity等类似的函数使用。  
### 非动态分配cpu_set_t
常见的接口有：  
```
void CPU_ZERO(cpu_set_t *set);
void CPU_SET(int cpu, cpu_set_t *set);
```
### 动态分配cpu_set_t
常见的接口有：
```
void CPU_ZERO_S(size_t setsize, cpu_set_t *set);
void CPU_SET_S(int cpu, size_t setsize, cpu_set_t *set);
```
一般来说，用非_S系列函数就够了，即便是想动态分配内存，非_S也是可以的。  
**_S应用场景：**
想省空间。一般来说，CPU个数不会很大（远小于1024个），那么使用非_S就意味着，cpu_set_t要占用128B内存空间。如果想省空间，可以使用CPU_ALLOC来申请更精准大小的空间。  
你的代码要操控的CPU个数超过1024。  
**CPU亲和性只是一种倾向性，当绑定的CPU不存在或者存在但是被禁用了，任务会在其他的CPU上执行。**  
