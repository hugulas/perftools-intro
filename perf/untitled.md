# 用Perf寻找程序中的性能热点

你好，我是hugulas chen。

作为程序员，在软件开发过程中，我们有的时候会遇到一些棘手的性能问题。比如下面的两种情况：

* 运维报告说：新上线的应用变慢了，处理器利用率也变高了。
* 用户对我们抱怨说：安装新应用后，跑到某个环节就卡住，系统都不响应了。

这个时候，你该怎么办呢？

在刚学编程的时候，我会去猜哪一段代码有性能问题，然后挨个给每个函数、每个循环加上计数器，通过计数器的值判断程序到底慢在哪里。但是过去的经验告诉我，这样的做法非常低效。

我花了大量的时间去做代码静态分析，调试计数器代码和反复跑慢得快宕机的代码，也不一定能猜到正确答案。而且要注意的是，这些插入到函数和循环中的计数器可能会带来非常大的性能负面影响，这些负面影响也会误导我们。

**既然上述操作行不通，这里\*\***我**来**告诉你一个更好的办法：采样。\*\*

采样是我们遇到“处理器消耗大，程序运行慢”时，首先应该想到的办法，它最快、最高效、负面影响也相对较小。你不需要改动应用代码，仅需对正在运行的应用做一次或者两次的采样，就能迅速找到性能的热点。

不同的平台提供了不同的采样工具。Linux平台上最著名的采样工具是perf和OProfile，它们都是Linux平台上轻量化的采样工具。采样给系统和应用带来的性能开销是可控的，比向每个函数每个循环插入计数器要小的多。

perf诞生在2009年，诞生在OProfile之后。当时，OProfile已经流行起来。但是，OProfile是一个独立的工具，它的更新相对来说比较慢，跟不上Linux的发展脚步，而且开发流程也和Linux内核团队不同。所以，林纳斯（Linus）就引入了perf这样一个工具。perf的代码和Linux内核代码放在一起，是内核级的工具。

perf的开发比OProfile要活跃得多，紧紧跟着Linux内核发布的节奏，对Linux内核的支持要比OProfile好得多。而且在功能上，perf也更强大，可以对众多的软硬件事件采样，还能采集出跟踪点（trace points）的信息（比如系统调用、TCP/IP事件和文件系统操作。和我一起的很多小伙伴都转向了使用perf）。所以，我觉得perf是在Linux上做剖析分析的首选工具。

那么，今天我们就通过解决“程序CPU占用高”这一问题，来看看perf是如何帮我们解决性能问题的。

### perf初体验

首先，我们看一看怎么用perf这把“牛刀”找到代码中的性能热点。

为了让你更容易理解，我用C写了一个求质数的小程序。你运行了之后，一定会偷偷问我：“你这程序怎么这么慢啊？一跑起来CPU利用率就是100%。”

别着急，我会在接下来的时间里带着你一起，用Perf找到这个小程序慢的根源。（你可以在GitHub上下载到这个程序。）[https://github.com/hugulas/perftools-intro/blob/master/linux-perf/sample1/sample.c](https://github.com/hugulas/perftools-intro/blob/master/linux-perf/sample1/sample.c)

#### 第一步：在Linux上安装perf

第一步当然是安装perf。有些比较早期的介绍perf的文档，会建议用户通过编译perf源代码来安装，但现在绝大部分主流的Linux发行版都提供了perf的支持，不管是Redhat系列，SUSE系列还是Debian系列，安装perf都只是一条命令的事了。

我在这里总结了不同Linux发行版上安装perf的命令，需要的同学可以加到笔记。

* Cent OS/RHEL：yum install perf
* Fedora：dnf install perf
* SUSE：zypper install perf
* Ubuntu：apt install linux-tools-common

和OProfile相比，安装简单是一个巨大的优势，90%以上的用户再也不需要为了获得内核部分的采样，而被迫编译相关代码了。在perf出现以前，我不止一次卡在OProfile内核支持的编译上。（我猜愤怒的林纳斯或许也有过这种不堪回首的经历。）

#### 第二步：通过perf stat判断程序是不是真的CPU占用率很高

我们在做性能分析时，有一点要牢记，所有的结论都应该由数据来支持，而不是直接依赖他人的判断。所以，我们首先需要**判断这个程序是不是真的很慢，是不是真的非常消耗处理器资源。**

我们可以通过执行下面这条命令，来采集程序的运行时间和CPU开销：

```text
perf stat ./sample 2000
```

我先来解释一下这条命令，它由三部分组成。

* perf：perf命令，perf命令的风格和版本控制工具git很像，它提供了非常多的子命令，比如stat、top、record、annotate等。通过调用这些不同的子命令，用户可以完成性能采样、性能分析、可视化等不同任务。
* stat：stat子命令，它汇总了程序执行过程中处理器和系统的一些计数器信息。它就好像程序的验血单，通过这张验血单，性能分析师就能判断出应用到底有没有生病，问题可能在哪里。
* ./sample 2000：被采样的命令，./sample是我写的小程序，2000是它的参数，表示寻找2000以下的所有质数。通过这一部分，我们告诉了perf怎么执行被采样的应用。

接着，我们再来看下刚才那条perf stat命令的输出。在文章中，我贴出来这条命令的输出：

```text
# 运行perf命令 
bash# perf stat ./sample 2000
# Performance counter stats for './sample 2000':


       5462.107517      task-clock (msec)         #    0.998 CPUs utilized
                91      context-switches          #    0.017 K/sec
                 0      cpu-migrations            #    0.000 K/sec
                45      page-faults               #    0.008 K/sec
   <not supported>      cycles
   <not supported>      stalled-cycles-frontend
   <not supported>      stalled-cycles-backend
   <not supported>      instructions
   <not supported>      branches
   <not supported>      branch-misses


       5.474573155 seconds time elapsed
```

输出的最后一行显示：求质数的命令执行了5.47秒。我的小程序运行了5.47秒，才找到所有2000以下的质数，的确是够慢的。

输出中还显示了task-clock \(msec\)是5462毫秒，也就是5.462秒，这是这个任务在处理器上消耗的时间。这个单线程程序的处理器时间5.462秒和总运行时间5.47秒几乎一样，处理器利用率是99.8%，这个处理器占用率非常高。

那么，是什么原因让它这么慢，这么消耗CPU的呢？要了解这个问题，我们接下来要对它做剖析，如果你看的是英文资料，对应的术语是Profiling， 顾名思义也就是给应用拍个X光片。

#### **第三步：剖析采样**

剖析采样可以帮助我们采集到程序运行的特征，而且剖析精度非常高，可以定位到具体的代码行和指令块。关于剖析的原理和注意事项，我会在下一篇文章中详细介绍。

我执行了下面这条perf record命令，来采集程序的剖析信息。record子命令会对系统进行采样，并把采样结果保存在perf.data文件中。

我通过“-F 999”选项，我把采样频率设置为999Hz，每秒采样999次。对运行5秒的程序，大约会有5000次采样。你不妨思考一下，为什么我要用999而不是1000呢？

```text
perf record -F 999 ./sample 2000
```

我把采样频率设置为999而不是整数，是一个实践中得出的小技巧。因为系统或者应用可能会有一些周期性的行为，一般是准点执行的。如果我使用1000等整数频率，可能会经常遇到某些周期性行为，而破坏了采样的随机性，让结果失真。

另外，采样时，我们要注意的是采样频率设置得越高，采样行为给系统带来的性能干扰越大；而如果设置得过小，那么采样的精确性就无法保证。所以，我一般会尽量保证我关心的函数和代码行有二位数以上的采样点，否则我就会调整采样频率重新采样。

perf有如下两行输出结果。从perf的输出，我们知道采样结果被写到了“perf.data”文件中。

```text
[ perf record: Woken up 1 times to write data ]
[ perf record: Captured and wrote 0.217 MB perf.data (5520 samples) ]
```

#### **第四步：通过\*\***p**erf** r**\*\*eport快速定位性能瓶颈**

采样结束后，性能分析师就可以通过perf report命令寻找采样中的性能瓶颈了。

下面是perf report的输出。

```text
Samples: 5K of event 'cpu-clock', Event count (approx.): 5525525520
Overhead  Command  Shared Object      Symbol
  99.06%  sample   sample             [.] wasteTime
   0.69%  sample   sample             [.] number
   0.07%  sample   [kernel.kallsyms]  [k] __do_softirq
   0.04%  sample   [kernel.kallsyms]  [k] _raw_spin_unlock_irqrestore
   0.04%  sample   [kernel.kallsyms]  [k] run_timer_softirq
   0.04%  sample   libc-2.12.so       [.] malloc
   0.02%  sample   [kernel.kallsyms]  [k] _raw_spin_lock_irq
   0.02%  sample   [kernel.kallsyms]  [k] cursor_timer_handler
   0.02%  sample   [kernel.kallsyms]  [k] finish_task_switch
   0.02%  sample   libc-2.12.so       [.] _int_malloc
```

perf report的输出是一个表格，依次有Overhead, Command, Shared Object, Symbol五列。

* Overhead：指出了该Symbol采样在总采样中所占的百分比。在当前场景下，表示了该Symbol消耗的CPU时间占总CPU时间的百分比
* Command：进程名
* Shared Object：模块名， 比如具体哪个共享库，哪个可执行程序。
* Symbol：二进制模块中的符号名，如果是高级语言，比如C语言编写的程序，等价于函数名。

从perf report的输出中，我们可以看到，在采样的5秒多时间内，sample模块的wasteTime函数花费了最多的处理器时间，99.06%的采样都落在了这个函数上。毫无疑问，这个函数是性能瓶颈。就像它的名字wasteTime一样，是我预先埋下的，用来浪费处理器时间的函数，你优化它就可以大大提高应用性能。

但是只定位到函数还不够好，perf工具还能帮我们定位到更细的粒度，这样我们就不用去猜函数中哪一段代码出了问题。如果我们通过键盘上下键把光标移动到wasteTime函数上，然后敲击Enter键，perf给出了一些选项。通过这些选项，我们可以进一步分析这个函数。

我们选中第一个选项“Annotate wasteTime”，我们敲击Enter键就可以对函数做进一步分析了。

**点击WasteTime后显示了下面的菜单：**

```text
Annotate wasteTime                --- 分析wasteTime函数中指令或者代码的性能
Zoom into sample(32477) thread    --- 聚焦到线程 sample(32477)
Zoom into sample DSO              --- 聚焦到动态共享对象sample(32477) 
Browse map details                --- 查看map
Run scripts for samples of thread [sample]--- 针对sample线程的采样运行脚本
Run scripts for samples of symbol [wasteTime] --- 针对函数的采样运行脚本   
Run scripts for all samples       --- 针对所有采样运行脚步
Switch to another data file in PWD --- 切换到当前目录中另一个数据文件
Exit
```

这里的Annotate是perf另外一个子命令，可以用于分析指定函数内（更精确地说应该是Symbol内）指令或者代码行的处理器开销。

下面是“Annotate wasteTime"的结果，Annotate输出是一个表格。整个表格被竖线分成左右两栏，竖线左边是每条指令的CPU-Clock占总体的百分比，也就是每条指令的热度。竖线的右边是指令的源代码行，反汇编代码等信息。

**wasteTime的Annotate的输出：**

```text
       │    Disassembly of section .text:                              
       │                                                              
       │    08048424 <wasteTime>:                                      
       │    wasteTime():                                              
  0.04 │      push   %ebp                                              
       │      mov    %esp,%ebp                                        
  0.02 │      sub    $0x10,%esp                                        
       │      movl   $0x0,-0x8(%ebp)                                  
       │      movl   $0x0,-0x8(%ebp)                                  
       │   ┌──jmp    20                                                
 22.31 │16:│  mov    -0x8(%ebp),%eax                                  
  0.34 │   │  add    %eax,-0x4(%ebp)                                  
 26.32 │   │  addl   $0x1,-0x8(%ebp)                                  
  7.96 │20:└─→mov    -0x8(%ebp),%eax                                  
 42.54 │      cmp    0x8(%ebp),%eax                                    
       │    ↑ jl     16                                                
  0.24 │      mov    -0x4(%ebp),%eax                                  
  0.15 │      leave                                                       
  0.08 │      ret
```

我们上面贴出来的输出中只包含了wasteTime函数反汇编后采样在每条指令上的分布。我的小程序是C语言编写，通过编译后就变成了一段段的二进制指令，比如wasteTime函数就是由一堆指令所组成的。 perf annnoate报告中为了方便我们阅读这些指令，就对这些指令做了反汇编，编成了我们可以阅读的反汇编代码。

如果这是一条跳转指令（跳转指令改变了指令的顺序，高级语言的循环、判断语句都依赖跳转指令来完成），在跳转指令前，会显示↑↓上下箭头符号，指明指令跳转的方向，比如“jmp 20”指令就是向下跳的。如果把光标移到跳转指令“jmp 20”指令上面，perf界面上会和output中一样，通过箭头链接显示出“jmp 20”跳转目标：指令“mov -0x8\(%ebp\),%eax”。我们会注意到"jmp 20"中有个数字20，在它的跳转目标“mov”指令前也有一个20. 这个20是什么意思呢？20是个十六进制数，这是指令的偏移地址，表示这条指令在函数起始地址的0x20位置。“jmp 20”表示跳转到函数起始位置的+0x20位置。如果我们想看到这些指令的绝对地址，而不是相对地址，大家可以按下o来切换到绝对地址模式。

在wasteTime的Annotate的输出中，我们可以看到指令0x16-0x20附近是最热的代码。我们可以通过改变16-20行的汇编代码来提高性能，比如修改汇编或者使用不同的编译选项。但是，仅仅找到最消耗处理器资源指令行对于大部分应用开发者来说还不够，我们很难通过猜测这些指令和代码相关联。有没有办法让perf帮我们找到对应的源代码行呢呢？答案是可以的，我们需要执行第五步

#### **第五步：定位性能瓶颈的代码行**

我们在第四步时，没有办法通过perf annotate看到代码行的采样信息，是因为我们采样的程序没有调试信息。我们需要重新编译代码来给程序附上调试信息。

一般情况下，大部分编译器都只需要在编译时加上相应的选项。比如对于gcc编译器，我们只要在编译时加上“-g"选项，编译产生的模块就会包含调试信息。我们就可以在perf annotate子命令产生的报表中直接看到相关源代码了。

我们执行下面的命令来重新进行编译和采样，来给应用加上调试信息：

```text
# 通过-g选项重新编译sample.c, 输出的二进制程序为sample_with_source
gcc -g sample.c -o sample_with_source
# 通过perf record命令对sample_with_source程序采样
perf record -F 999 ./sample_with_source 2000
```

然后，我们通过单独执行命令perf annotate --stdio --symbol=wasteTime来打印wasteTime函数的采样信息。

参数--symbol=wasteTime指明了要分析对象是符号wasteTime。 这条命令等价于我们在上文第四步中通过perf界面执行“Annoate wasteTime”菜单。perf会把选中函数的代码行，每一行代码对应的指令，每条指令的采样数都打印出来，因为重新编译了的sample\_with\_source程序中包含了调试信息，我们就可以直接在输出中看到wasteTime函数的代码行以及这些代码行关联了哪些指令。

比如下面的wasteTime函数的代码行信息输出：wasteTime中的for循环语句是整个函数中采样最多的代码行。它编译后产生的8048440到804844a的这几条指令花费了绝大部分的处理时间。 这行代码就是性能的瓶颈。

```text
perf annotate --stdio --symbol=wastTime
         :      int wasteTime(int factor)
         :      {
         :          int i = 0;
         :          int testResult;
         ：          # 循环是性能瓶颈  
         :          for (i = 0; i < factor; i++)
   26.72 :        8048440:       addl   $0x1,-0x8(%ebp)
    7.69 :        8048444:       mov    -0x8(%ebp),%eax
   43.81 :        8048447:       cmp    0x8(%ebp),%eax
    0.00 :        804844a:       jl     804843a <wasteTime+0x16>
         :          {
         :              testResult += i;
         :              testResult = testResult * 1;
         :          }
         :          return testResult;
    0.24 :        804844c:       mov    -0x4(%ebp),%eax
         :      }
    0.09 :        804844f:       leave
    0.04 :        8048450:       ret
```

### 总结

今天我们学习了如何使用Perf去抓住代码中的性能瓶颈。首先，我们通过perf stats命令对程序的性能特点有了基本的了解，确认了程序存在处理器利用率高的问题；然后，我们通过perf record命令对程序进行了采样；最后，我们使用perf report命令分析采样，找到了代码中的性能热点。

我建议你可以在课后找台Linux安装好perf，按照我们今天所做的步骤，亲自动手使用perf做一做性能分析。欢迎和我讨论你在性能分析过程中所遇到的问题。

现在，我们知道perf是Linux平台上的做性能分析的利器，可以在很短时间内帮我们找到程序中的处理器开销最大的代码。它相比其他工具，安装起来更简单，操作也很方便，如果你能熟练操作git，一定也能熟练操作它。掌握好这个工具，你就可以不断压榨你的程序的性能, 写出更流畅，更省电的应用。我自己非常喜欢这样的感觉，我曾经把一个跑20分钟的数据分析应用通过剖析分析不断调优到5分钟跑完，哈，就像买了辆QQ，你把它魔改成了起步三秒的超跑。 【TODO：现在，我们知道perf是Linux平台上一个轻量的，分析性能问题的利器，它相比其他工具……如果你平时……，那么掌握好工具是……】

我这里也有一个小问题想留给你：在这节课中，我使用perf record采样，perf使用的默认计数器是“cpu-clock”, 那么在你的环境中，默认计数器会是什么呢？为什么是这个计数器呢？

