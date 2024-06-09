
请求
method
url
version

header_key
header_value

body

响应
version
code
description

header_key
header_value

body

method_POST
method_GET

状态机模式的优点
- 将


需求
- 系统初始化各种回调，一次匹配过程中输入str和该str从长度，同时返回是状态
- 一个总的解析器类负责总体任务
- 解析器不存储消息内容，消息内容在外部连续保存

- 一个字符串和该字符串大小，按字节不停匹配。如果不匹配成功，返回相应错误码，如果匹配成功，在匹配成功的不同阶段，调用相应阶段的回调
- 

