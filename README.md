# got-call

only for x86_64 linux

* DSOの他の関数を呼び出す例
  * GOTを利用して、ある関数から既知のオフセットに存在する関数を呼び出す
  * このときの注意として、GOTを利用する際に、その関数が初回呼び出しを終えて、GOTにDSOの正しいアドレスが指定されていることが前提となる
  * また、基準とする関数が`LD_PRELOAD`で挿入されたDSOにリンクされた場合は既知のオフセット情報が利用できず、正常に機能しない

## how to build
```
g++ -std=c++11 main.cpp -ldl -g3 -o got-call
```

```
$ objdump -d got-call | grep -A 3 "^[0-9a-e]* <getpid@plt>"
0000000000400ae0 <getpid@plt>:
  400ae0:       ff 25 5a 15 20 00       jmpq   *0x20155a(%rip)        # 602040 <_GLOBAL_OFFSET_TABLE_+0x40>
  400ae6:       68 05 00 00 00          pushq  $0x5
  400aeb:       e9 90 ff ff ff          jmpq   400a80 <_init+0x28>
```

```
$ readelf -S got-call | grep -A 1 .got.plt
  [25] .got.plt          PROGBITS         0000000000602000  00002000
       0000000000000098  0000000000000008  WA       0     0     8
```

## how to run
```
$ ./got-call | column -s : -t
&getppid(dlsym)                                      0x7f93d6c69230
&getpid(dlsym)                                       0x7f93d6c691f0
&getppid(dlsym) - &getpid(dlsym)                     0x40
getpid@plt address                                   0x400ae0
getpid@plt asm                                       0x5680020155a25ff
offset(got - plt)                                    0x20155a
.got.plt(getpid) adderss                             0x602040
pre  call got asm(next instruction address of plt)   0x400ae6
post call got asm(getpid(dlsym))                     0x7f93d6c691f0
&getppid - &getpid                                   0x40
pid                                                  11073
ppid(found from got)                                 31215
ppid(expected)                                       31215
```

