# Compile information 
```
cc -std=c99 -Wall parsing.c lib/mpc-master/mpc.c -ledit -lm -o parsing
```
* We list all file (__parsing.c__ and __lib/mpc-master/mpc.c__)
* We add dependance :
  * __ledit__ for readline on linux
  * __lm__ for math (include un mpc) 

# CURRENT
[here](https://buildyourownlisp.com/chapter8_error_handling)