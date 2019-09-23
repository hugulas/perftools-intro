# 从随手可得的top开始

你好，我是陈沁悦。

你可能也会遇到这样的场景：自己写了个程序，编译好后，在Linux上跑一会儿，整个机器就卡住了，没有任何响应，怎么回事？

这个时候，作为程序员，你的第一反应是什么？

* 在Windows上，我会打开任务管理器看看什么地方出问题了；
* 在Linux或者AIX上，我会打开top或者topas看看问题出在哪里；
* 而在Mac上，图形界面爱好者会打开Activity Monitor，而命令行的发烧友会继续使用top。

其实你可能没有意识到，当打开这些工具的时候，你已经开始在做性能分析的基础工作了。

像任务管理器、top、topas、Activity Monitor这些程序，它们被统称为即时性能分析工具，这种工具共同的特点是：“即时”，它们是十分简单、极为常见的工具，就像一把小刀一样，随身携带，使用方便。

不管什么平台，你都可以找到这样的即时性能分析工具，它们界面和使用方式也差不多。你如果精通了其中一个，到其他平台上稍微看看操作说明，上手其他的也不难。

通过这些即时性能监控工具，你可以看一看问题大概出现在哪个方面，这样“概要”的查看和确定，也就是最简单的概要性能分析方式。通过概要性能分析，我们可以确定，问题是处理器问题呢，还是内存问题，从而对症下药，找出解决方案。

下面我们就来一起以top为例，来实践一下：如何给你的应用做一个概要性能分析，我们来看个例子。

## 如何运用你的即时分析工具？

我有一个例子程序，放在GitHub上。（[https://github.com/hugulas/perftools-intro/blob/master/linux-perf/sample1/sample.c](https://github.com/hugulas/perftools-intro/blob/master/linux-perf/sample1/sample.c)）。 你可以下载下来，然后用gcc命令编译，再执行这个程序。

```text
# 编译程序
gcc sample.c -o sample
# 执行程序 参数 20000
./sample 20000
```

我们可以在程序执行起来后，在另外一个终端输入“top”命令，就可以看到top的输出了。

top的输出反应了当时的系统性能状态。输出分成了上下两部分，上面是系统的性能统计信息，下面是进程的性能信息列表，中间有一个空行隔着的。

```text
####################   系统的性能统计信息  ##############################
top - 03:42:22 up 293 days,  2:06,  2 users,  load average: 0.28, 0.07, 0.02
Tasks: 117 total,   2 running, 115 sleeping,   0 stopped,   0 zombie
Cpu(s): 99.3%us,  0.7%sy,  0.0%ni,  0.0%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
Mem:    520132k total,   485160k used,    34972k free,    79540k buffers
Swap:   135164k total,     1172k used,   133992k free,   313232k cached

###################  进程的性能信息  ################################ 
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND
11649 root      20   0  2064  328  284 R 99.2  0.1   0:19.24 sample
   20 root      20   0     0    0    0 S  0.3  0.0 231:45.18 kworker/0:1
 2677 nobody    20   0 15228 9388 4188 S  0.3  1.8   2:44.69 python
11613 root      20   0 11868 6184 5408 S  0.3  1.2   0:00.04 sshd
11650 root      20   0  2768 1912 1656 R  0.3  0.4   0:00.11 top
    1 root      20   0  2964 2036 1936 S  0.0  0.4   0:01.78 init
```

系统的性能统计信息分成五行，我们来逐行解释下。

1. 第一行是系统信息： 系统信息包括了当前的系统时间3点42分22秒（03:42:22），“up 293 days，2:06” 当前的系统已经运行了293天2小时6分， 2 users表示当前有2个登录的用户，“load avergage:"后面紧跟着的是1分钟、5分钟、15分钟平均负载，平均负载分别是0.28，0.07，0.02。
2. 第二行是任务信息。“117 total，2 running, 115 sleeping”表示一共117个任务，2个正在运行，115个睡眠的，“0 stopped, 0 zombie”表示没有停止的进程和僵尸进程。
3. 第三行是处理器信息。“99.3%us表示用户态进程占用了99.3%的CPU时间，0.7%sy”表示内核占用了0.7%的处理器时间。看到这一行，你应该就看出点问题来了，系统的处理器时间被用满了，难怪我这台只有单核的虚拟机卡得不行。
4. 第四行是内存信息。“520132k total，485160k used，34972k free”表示有520132k物理内存，已经用了485160k。换算下来我的这台虚拟机一共507M内存，已经用了473M，还剩下34M。79540k buffers表示内核缓存的物理内存量是77M。
5. 第五行是交换空间的信息。“135164k total,  1172k used,  133992k free”表示交换空间一共135164k，已经用了1172k，还剩下133992k。

从系统性能统计信息中，我们已经可以注意到，处理器占用率出现了异常。我们再来通过进程信息列表看每个进程的信息。

进程信息列表是一个表格，在我们的输出里，一共有12列：

* PID（进程号）；
* USER（进程拥有者的用户名）；
* PR（进程的优先级，数字越小，优先级越高，优先被执行）；
* NI（进程优先级的NICE值）；
* VIRT（进程占用的虚拟内存值）； 
* RES（进程占用的物理内存）；
* SHR（进程使用的共享内存）；
* S（进程的状态，值是R表示running正在运行，值是S表示sleeping休眠中）；
* %CPU（进程占有处理器时间百分比）；
* %MEM（进程使用的物理内存占总内存的百分比）； 
* TIME+表示进程启动后使用的总处理器时间；
* COMMAND就是启动进程的命令。

默认情况下，这个列表是按照处理器占用率降序排列的。我们可以看到排在第一个的 进程，就是我们刚刚运行的那个sample程序，它占了99.2%的处理器时间，它的处理器占用率太高了，这个应用有处理器利用率方面的问题。

如果你想要具体找到哪行代码出了问题，我会在后面的“perf小试牛刀”一篇中，教给你一个最高效的办法。

通过这个实例，我们了解了top命令的输出，并找到了应用性能问题所属的领域，你是不是对使用这个工具已经有点把握了呢。那我们不妨再来试试，用top再来找找下一个应用是出了什么问题吧。

## 如何找到应用的问题出在哪里？

我们可以从GitHub上下载第二个应用的源代码（[https://github.com/hugulas/perftools-intro/blob/master/linux-perf/sample2/mem.c](https://github.com/hugulas/perftools-intro/blob/master/linux-perf/sample2/mem.c)）。打开第二个应用让它跑一会后，整个系统都不好了，完全不响应了。我不信这个邪，打开了top，然后另外找了一个终端执行程序。

```text
# 编译
gcc mem.c -o mem
# 执行，每次调用间隔500000微秒
./mem 500000
```

通过上一部分的介绍，我们知道如何查看top的输出。我们来检查一下，有哪些异常的数据。

~~物理内存占用涨到了359556k~~

```text
#################### 系统的性能统计信息 ##############################
top - 11:31:19 up 293 days,  9:55,  2 users,  load average: 0.00, 0.00, 0.00
Tasks: 119 total,   1 running, 118 sleeping,   0 stopped,   0 zombie
Cpu(s):  0.0%us,  0.0%sy,  0.0%ni,100.0%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
Mem:    520132k total,   359556k used,   160576k free,     5076k buffers
Swap:   135164k total,    51256k used,    83908k free,    28732k cached

################### 进程的性能信息 ################################ 
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND
13208 root      20   0  2768 1932 1680 R  0.3  0.4   0:00.97 top          1 root      20   0  2964    8    4 S  0.0  0.0   0:01.82 init
    2 root      20   0     0    0    0 S  0.0  0.0   0:00.00 kthreadd   
    4 root       0 -20     0    0    0 S  0.0  0.0   0:00.00 kworker/0:0H
```

你会发现，在系统性能状态的第四行mem那一行，used那一栏显示的物理内存占用在蹭蹭地往上涨，一眨眼的功夫，内存占用就从跑之前的82704k上涨到了下面输出中的359556k。这是怎么回事呢？

我们不妨在top界面按下大写O，来改变进程列表的排序方式。这时，top界面会让我们选择根据哪一列来排序，从下面的列表可以看出，内存占用“%mem”对应的是字母n，我们按下“n”，然后按下回车。

屏幕上显示是这样的：

```text
Current Sort Field:  K  for window 1:Def
Select sort field via field letter, type any other key to return


  a: PID        = Process Id                                          
  b: PPID       = Parent Process Pid            
  c: RUSER      = Real user name                
  d: UID        = User Id      
  e: USER       = User Name
  f: GROUP      = Group Name
  g: TTY        = Controlling Tty
  h: PR         = Priority
  i: NI         = Nice value
  j: P          = Last used cpu (SMP)
* K: %CPU       = CPU usage
  l: TIME       = CPU Time
  m: TIME+      = CPU Time, hundredths
  n: %MEM       = Memory usage (RES)
  o: VIRT       = Virtual Image (kb)
  p: SWAP       = Swapped size (kb)
  q: RES        = Resident size (kb)
  r: CODE       = Code size (kb)
  s: DATA       = Data+Stack size (kb)
  t: SHR        = Shared Mem size (kb)
  u: nFLT       = Page Fault count
  v: nDRT       = Dirty Pages count
  w: S          = Process Status
  x: COMMAND    = Command name/line
  y: WCHAN      = Sleeping in Function
  z: Flags      = Task Flags <sched.h>
```

然后，我们会发现top界面中，进程列表的第一个就是我们的刚刚运行的mem小程序，它花的2内存也在短短的几十秒内爬到了271M，大大超出了我对一个不到100行的小程序的预期。

这个程序有内存问题，需要进一步的调优。关于如何做内存的分析，我会在后面章节的内存部分介绍。

```text
top - 11:34:03 up 293 days,  9:57,  2 users,  load average: 0.04, 0.02, 0.00
Tasks: 119 total,   1 running, 118 sleeping,   0 stopped,   0 zombie
Cpu(s):  0.0%us,  0.3%sy,  0.0%ni, 99.7%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
Mem:    520132k total,   359524k used,   160608k free,     5100k buffers
Swap:   135164k total,    51192k used,    83972k free,    28728k cached


  PID USER      PR  VIRT  NI  RES  SHR S %CPU %MEM    TIME+  COMMAND
13212 root      20  271m   0 271m  952 S  0.0 53.4   0:00.87 mem
13193 root      20 12068   0 6216 5308 S  0.0  1.2   0:00.36 sshd
13130 root      20 11756   0 6016 5244 S  0.0  1.2   0:00.12 sshd
13132 root      20  5296   0 2984 2712 S  0.0  0.6   0:00.08 bash
```

通过这两个例子，我们学会了使用top，了解了top界面都有哪些信息，怎么解读这些信息，怎么将进程列表按照某一种信息排序找出问题最突出的进程。

## 总结

你可以在课后把程序下载下来，自己动手跑一跑，用top分析一下。我想，你很快就能学会top的使用。如果你能熟练地掌握这个工具，遇到性能问题就能很快判断出这大概是哪一种资源用得太多了。

这里给你留个思考题，如果用户和我说，你们的应用每天凌晨三点会瘫痪，你怎么办？你是等到凌晨三点一直盯着top吗，还是有更好的办法呢？

