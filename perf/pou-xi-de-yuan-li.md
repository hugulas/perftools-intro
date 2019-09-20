# 剖析的原理

你好，我是hugulas。

在上一篇中，我带着你，一起用perf找到了求质数小程序中的性能瓶颈。在寻找性能瓶颈的过程中，我们不需要对应用程序结构有太多的了解，也不需要修改任何应用代码；仅仅是通过perf做了一次剖析（Profiling），程序的性能瓶颈就自然暴露在我们面前。

但是，有了剖析工具，知道怎么使用就够了么？~~有没有感觉很神奇呢？我想你一定很好奇，剖析工具是怎么做到这一点的？~~

~~今天，我们就来讲一讲剖析的原理。首先我们来聊聊，剖析是什么（Profiling），剖析的原理是什么样的，并且我会用一个具体的例子，帮助你更好地理解。~~

~~那么，我为什么要给你讲原理呢？~~

如果你尝试着去读一下perf或者OProfile的手册，你会发现每句话你都能看懂，但是实战中却不知道这些选项应该怎么用，产生的报告应该怎么解读。我当年刚入行时，也有过这样的困惑。我曾经问过组里其他的开发者：“我们自己写的工具，我看了文档都不知道怎么用，用户怎么会用？”

经过一段时间的工作，我找到了答案，这是因为性能剖析工具和其他软件工具的用户群不同，它的用户是资深程序员和性能专家。它是为这些大师们量身打造的屠龙刀。大师们用剖析工具可以在复杂系统中庖丁解牛般地快速找到性能瓶颈。~~但是初学者想要用好这把屠龙刀却并不容易，因为使用说明也是为这些大师们写的，很多~~

但是初学者想要用好这把屠龙刀，却并不容易。你只有了解它的原理，才能合理地使用它去采样，怎么选择采样频率，采样事件，怎么解决各种异常；只有了解它的原理，才能正确地解读它采样得到的结果。

今天，我们就来讲一讲剖析的原理。首先我们来聊聊，剖析是什么（Profiling），剖析的原理是什么样的，并且我会用一个具体的例子，帮助你更好地理解。

在未来的应用性能分析过程中，你必然会在复杂环境下遇到千奇百怪的问题吗，希望这些原理可以帮助你找到解决问题的线索。

## 什么是剖析（Profiling）

剖析，我们简单地理解，就是给要分析的对象拍照，通过照片帮助我们理解和分析对象的特点，形象一点说，比如我们去医院拍X光片，拍CT就是剖析的过程。

而在计算机领域，**剖析（Profiling）是指\*\***：**通过对系统状态固定频率的采样，并统计分析采样的结果。**【TODO：这里需要加一下剖析/采样又分为剖析法和追踪法两种】\*\*定期进行采样是剖析法（Profiling）和追踪法（Tracing）最大的不同。【TODO：剖析法是指……追踪法是指……】和追踪法相比，定期采样可以有效地减少性能数据采集带来的性能开销。

在磁盘IO或者网络领域，剖析采样可以帮助性能分析师，用比较低的性能影响获得粗粒度的性能报告。

处理器利用率分析是剖析法最常用、最有效的领域。剖析工具可以通过归纳总结被采集到的指令和调用栈栈信息（stack trace），帮助分析师了解到：哪些代码在消耗处理器资源。

## 剖析工具的工作原理是什么样的？

【TODO：说到剖析工具，那么我们工作中……，所以这里我主要以perf和OProfile工具为例，来说……】

perf或者OProfile等工具在进行采样时，每隔一个固定的时间间隙，采集系统上软硬件性能事件，获得不同性能事件的统计结果。

它们会记下来在这个采样间隔中执行了多少指令，发生了多少次CacheMiss等等。除了采集软硬件性能事件外，**还会采集当前正在执行的指令地址和进程编号\*\***（**PID**）**\*\*，**我们在这个采样间隙中，采集到的软硬件性能事件都会记到这条指令身上。

剖析工具首先找到指令地址和进程编号，通过地址到命名的转换过程，找到指令具体属于哪条进程（命令），哪个线程，哪个模块，哪个函数，甚至哪个代码行。

最后，剖析工具会统计每个进程/线程，每个模块，每个函数，每个代码行，每条指令有多少次采样和多少软硬件事件。性能分析师通过这些信息可以顺藤摸瓜，找到有性能问题的模块或者代码。

听起来比较抽象，我们不妨一起来看个具体的例子。

在上一篇中，我们使用了“perf record -F 999 ./sample 2000”这样一条命令来进行采样。我指定的采样频率是999HZ，也就是每秒999次采样。

我们每次采样时，都会采集到当前正在执行的指令的指令地址和进程编号，以及相应的事件计数器信息。比如某一次采样采集到了地址为0x004005c4的指令，它的进程编号是22258。剖析工具就需要对它进行指令地址到命名的转换，以便定位出这条指令属于哪条进程。这里的具体过程如下所示。

第一步，**剖析工具通过进程编号\*\***（PID）**22258，就可以**从进程列表获知进程对应的是./sample命令**。** \*\*通过进程编号，找到进程对应命令的办法多种多样，不同平台上不同工具用的方法不同。 比如在Linux系统/proc目录下，每个PID对应了一个目录，打开/proc//cmdline文件, 我们就能找到PID对应的命令行。

```text
[hugulas@hugulas sample1]$ cat /proc/22258/cmdline
./sample 100000
```

第二步， **剖析工具在22258的\*\***地址空间**\*\*中，找到地址0x004005c4对应的模块**。下面的列表是进程22258的内存映射文件（/proc/22258/maps文件，**列表第一列是内存映射区域的起始地址，最后一列是模块名**）， 我们可以看到0x004005c4属于sample模块（/home/hugulas/sample，开始地址00400000，结束地址00401000）。

**/proc/22258/maps文件：\*\***~~（列表第一列是内存映射区域的起始地址，最后一列是模块名）~~\*\*

```text
[hugulas@hugulas sample1]$ cat /proc/22258/maps
00400000-00401000 r-xp 00000000 fd:03 487 /home/hugulas/sample
00600000-00601000 r--p 00000000 fd:03 487 /home/hugulas/sample
00601000-00602000 rw-p 00001000 fd:03 487 /home/hugulas/sample
01135000-01156000 rw-p 00000000 00:00 0   [heap]
7fc686e19000-7fc686fdb000 r-xp 00000000 fd:00 33645705  /usr/lib64/libc-2.17.so
7fc686fdb000-7fc6871db000 ---p 001c2000 fd:00 33645705  /usr/lib64/libc-2.17.so
7fff7fe71000-7fff7fe92000 rw-p 00000000 00:00 0 [stack]
7fff7fff9000-7fff7fffb000 r-xp 00000000 00:00 0 [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0  [vsyscall]
```

第三步，通过用不同的工具进一步扫描内存映射区域，或者分析二进制模块文件（/home/hugulas/sample）, 可以得到sample模块中各函数的地址范围。下面的列表就是sample模块各函数的地址范围。**从各函数的地址范围，我们可以知道0x004005c4属于wasteTime函数（5ad-5db）\*\***。\*\*

**sample模块中各函数的地址范围：**

```text
/home/hugulas/sample
 438  4c0 g _init                                        
 470  480 g printf@plt                                                 480  490 g __libc_start_main@plt    
 490  4a0 g malloc@plt                                          
 4a0  4b0 g atoi@plt                  
 4c0  4f0 g _start                
 4f0  520 l deregister_tm_clones
 520  560 l register_tm_clones 
 560  580 l __do_global_dtors_aux                             
 580  5ad l frame_dummy             
 5ad  5db g wasteTime          
 5db  6d0 g number                  
 6d0  771 g main  
 780  7e5 g __libc_csu_init
 7f0  7f2 g __libc_csu_fini 
 7f4 2000 g _fini
```

第四步，**通过sample模块中的debug信息或者额外的debug模块，剖析工具可以把指令地址和C的源代码相关联。**perf可以通过objdump从sample模块中抽取出源代码信息。**指令0x004005c4对应的代码行\*\***是第17行：“testResult += i;”**\*\*。**

**objdump的输出：**（命令：objdump --source --line-numbers sample\_with\_source）

```text
00000000004005ad <wasteTime>:
wasteTime():
/home/hugulas/code/perftools-intro/linux-perf/sample1/sample.c:12
    int count;
    struct PrimeNumberNode *next;
};


int wasteTime(int factor)
{
  4005ad:       55                      push   %rbp
  4005ae:       48 89 e5                mov    %rsp,%rbp
  4005b1:       89 7d ec                mov    %edi,-0x14(%rbp)
/home/hugulas/code/perftools-intro/linux-perf/sample1/sample.c:13
    int i = 0;
  4005b4:       c7 45 fc 00 00 00 00    movl   $0x0,-0x4(%rbp)
/home/hugulas/code/perftools-intro/linux-perf/sample1/sample.c:15
    int testResult;
    for (i = 0; i < factor; i++)
  4005bb:       c7 45 fc 00 00 00 00    movl   $0x0,-0x4(%rbp)
  4005c2:       eb 0a                   jmp    4005ce <wasteTime+0x21>
/home/hugulas/code/perftools-intro/linux-perf/sample1/sample.c:17 (discriminator 2)
    {
        testResult += i;
  4005c4:       8b 45 fc                mov    -0x4(%rbp),%eax
  4005c7:       01 45 f8                add    %eax,-0x8(%rbp)
/home/hugulas/code/perftools-intro/linux-perf/sample1/sample.c:15 (discriminator 2)
    for (i = 0; i < factor; i++)
  4005ca:       83 45 fc 01             addl   $0x1,-0x4(%rbp)
/home/hugulas/code/perftools-intro/linux-perf/sample1/sample.c:15 (discriminator 1)
  4005ce:       8b 45 fc                mov    -0x4(%rbp),%eax
  4005d1:       3b 45 ec                cmp    -0x14(%rbp),%eax
  4005d4:       7c ee                   jl     4005c4 <wasteTime+0x17>
/home/hugulas/code/perftools-intro/linux-perf/sample1/sample.c:20
        testResult = testResult * 1;
    }
    return testResult;
  4005d6:       8b 45 f8                mov    -0x8(%rbp),%eax
/home/hugulas/code/perftools-intro/linux-perf/sample1/sample.c:21
}
  4005d9:       5d                      pop    %rbp
  4005da:       c3                      retq
```

剖析工具依次处理每一条采样记录，把这些采样统计到进程、线程、模块、函数、代码行、指令行。性能分析师就能通过剖析工具知道采样在不同层面的分布，比如，如以下列表所示，采样大部分集中在waste time和number两个函数上。

**采样在函数层面的分布：**

```text
Samples: 4K of event 'cycles:u', Event count (approx.): 13437837470
Overhead  Command  Shared Object     Symbol                              
  99.04%  sample   sample            [.] wasteTime                   ▒
   0.94%  sample   sample            [.] number                        
   0.02%  sample   [kernel]          [k] 0xffffffff94d75c90            
   0.00%  sample   ld-2.17.so        [.] dl_main                     ▒
   0.00%  sample   [kernel]          [k] 0xffffffff94d6b730            
   0.00%  sample   ld-2.17.so        [.] _start
```

从剖析采样的原理可以看到，处理器利用率分析所需要的信息有以下几个来源： 1. 剖析采样； 2. 进程列表； 3. 进程的地址空间映射； 4. 二进制模块文件。

这些信息是系统和应用本来就有的，不需要应用改变自身的行为或者逻辑来提供，这也是剖析分析的一个优势。

根据剖析采样的原理，我们在分析和采样时有以下几点需要注意。

1. **剖析的结果是采样统计归纳的结果，只具有统计学上的意义。**只有当函数或者指令采样数量足够大的时候，不同函数和指令上的采样数量比较才有意义。比如指令A有2个采样，而指令B有1个采样，并不意味着指令A就一定比指令B花的时间更多。但是如果指令A有2000个采样，而指令B有1000个采样，我们可以认为指令A就一定比指令B花的处理器时间更多。建议：通过控制采样时间和采样频率，保证要分析的关键的函数和指令至少有两位数以上的采样。
2. **采样频率除了会影想到剖析的精度，也会增加剖析的性能开销和剖析的处理时间**。【TODO：因为……】采样频率越高，剖析的精度越高，但是性能开销也会越大，极端情况有可能下会干扰到应用的行为特征， 同时采样文件也越大。反之，采样频率越小，虽然性能开销变小，对系统的干扰也变小，但是采样的精度也相应下降。建议：如果是第一次对应用或者系统采样，可以尝试不同采样频率，并和不采样的性能做比较，选择性能开销可以接受的采样频率。
3. **剖析工具把采样进行函数和代码级别的归纳时，需要模块文件提供符号表和debug信息。**因此，我建议你尽量不要通过strip命令砍掉模块的符号表信息，这是很多人在XX情况下会执行的操作。对于要进行代码级分析的模块，可以通过重新编译提供debug信息。

   **总结**

【TODO这里还是总结一下今天讲的内容哈。加强用户的记忆】

在介绍完剖析的原理后，我有一个小问题留给你：如果我们要把perf.data文件从服务器拷贝到笔记本上来分析，需要注意什么问题？

