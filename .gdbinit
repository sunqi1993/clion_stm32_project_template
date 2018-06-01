#这个文件放在 ~/ 路径下(Linux)  或者 $HOME/路径下 (Win下 需要添加环境变量HOME 数值自己选择合理的路径)
define target hookpost-remote
monitor reset
monitor halt
load
break main
end