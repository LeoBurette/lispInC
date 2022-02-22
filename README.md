# Compile information 
```
cc -std=c99 -Wall src/parsing.c lib/mpc-master/mpc.c src/eval.c src/lval.c src/util.c  -ledit -lm -o parsing
```
* We list all file (__parsing.c__ and __lib/mpc-master/mpc.c__)
* We add dependance :
  * __ledit__ for readline on linux
  * __lm__ for math (include un mpc) 

# CURRENT
[here](https://buildyourownlisp.com/chapter10_q_expressions#collapseOne)

BONUS MARK !