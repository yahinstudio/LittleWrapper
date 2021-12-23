## LittleWrapper

LittleWrapper是一个适用于Windows平台的可执行文件打包程序，用于打包一个文件夹到一个文件里，在运行时，释放到临时文件夹然后执行。

### 功能

+ 支持释放缓存，二次启动会比首次启动要更快
+ 支持单实例传递，多次运行路径相同的文件时，不会反复解压，而是重复启动被临时释放的入口程序
+ 支持抑制stdout的输出，避免作为子进程运行时，stdout的输出超过父进程buffer大小导致子进程卡死
+ 支持快速启动，二次启动可以选择跳过已有内容校验，进一步提升启动速度

### CLI

`[]`为可选参数，`<>`为必选参数

```bash
# 显示帮助信息
--help

# 打包文件夹到一个文件里
# source_dir：要被打包的文件夹路径
# exec：启动命令行（通常是启动文件夹里主程序）
# output_file：打包后的输出文件名
# no-hashing：以禁用二次启动时的文件校验过程，来换取启动速度（首次启动不受影响）

# exec里支持3个路径变量，（3个变量末尾均没有路径分隔符）
# $_lw_tempdir：解压时的临时文件夹
# $_lw_exedir：打包后EXE文件所在路径
# $_lw_exefile：打包后EXE文件路径
--pack <source_dir> --exec <command> [--output <output_file>] [--no-hashing]

# 解压已经打包好的文件（只是解压出来，不会执行）
# output_dir：解压后存放的位置，默认为文件旁边
# 功能一样，但有3种不同的写法
--extract [--output <output_dir>]
--extract=[output_dir] # 不同的写法
-e[output_dir] # 不同的写法

# 以json格式列出当前都打包了哪些文件和内容（结果会输出到stdout里）
--detail

# 以显示console窗口的方式运行已经打包好的exe文件
# 打包时使用这个参数会为后续的运行设定一个默认的执行方式：显示窗口的方式运行
# 运行时使用这个参数会覆盖打包时设置的默认参数
# 当--show-console或者--hide-console都没有被指定时，--show-console会被设置成默认值
--show-console

# 以隐藏console窗口的方式运行已经打包好的exe文件
# 打包时使用这个参数会为后续的运行设定一个默认的执行方式：不显示窗口的方式运行
# 运行时使用这个参数会覆盖打包时设置的默认参数
--hide-console
-x # 不同的写法

# 屏蔽释放时的日志输出，避免当本程序作为子进程运行时，stdout的输出超过父进程buffer大小导致子进程卡死
--suppress-output
-u # 不同的写法

# 启动参数传递，参数会附加到--pack的exec参数末尾传递给真正的主程序
# 此参数仅在启动程序时有效，打包时使用无效
--paramater-pass "<paramater-string>"
-a # 不同的写法
```

### 运行时变量

出了父进程的环境变量以外，LittleWrapper还会给子进程添加额外的环境变量：

| 环境变量          | 描述                            | 示例                 |
| ----------------- | ------------------------------- | -------------------- |
| \_LW_EXEFILE      | LittleWrapper的路径             | c:\\some_dir\\lw.exe |
| \_LW_EXEDIR       | LittleWrapper所在目录的路径     | c:\\some_dir         |
| \_LW_TEMPDIR      | LittleWrapper临时释放文件的路径 | c:\\temp_dir\\LW-xxx |
| \_LW_EXEFILE\_    | 正斜线版本的\_LW_EXEFILE        | c:/some_dir/lw.exe   |
| \_LW_EXEDIR\_     | 正斜线版本的\_LW_EXEDIR         | c:/some_dir          |
| \_LW_TEMPDIR\_    | 正斜线版本的\_LW_TEMPDIR        | c:/temp_dir/LW-xxx   |
| \_LW_VERSION      | LittleWrapper版本号             | v1.0.0               |
| \_LW_COMPILE_TIME | LittleWrapper的编译时间(utc +0) | Dec 23 2021 18:26:53 |

### 开源项目

感谢以下开源项目的作者！

+ [zlib](http://www.zlib.net)：压缩算法支持库
+ [wingetopt](https://github.com/alex85k/wingetopt)：CLI参数解析库
+ [cjson](https://sourceforge.net/projects/cjson)：JSON解析和生成库
+ [md5](https://blog.csdn.net/wudishine/article/details/42466831)：MD5散列算法支持
+ [StackWalker](https://github.com/JochenKalmbach/StackWalker)：CPP的调用堆栈支持库

