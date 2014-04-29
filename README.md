RTServer
========

@name	Real Time Server
@author	bishenghua
@email	net.bsh@gmail.com


一、项目划分：

	1）主服务RTServer：
	编译命令：make clean && make （加入了Makefile文件，编译轻松多了，类似nginx哦）
	启动命令：objs/bin/RTServer
		RTServer成功运行...

	2）用户管理RTSAdmin：
	编译命令：make clean && make
	启动命令：objs/bin/RTSAdmin 
		请输入代表数字：
		0.建表
		1.添加
		2.查看全部
		3.根据id查看
		4.根据name查看
		5.删除
		choose[0 - 5]:

	3）简单交互客户端：
	编译命令：make clean && make
	启动命令：objs/bin/RTClient 127.0.0.1 5566
		{"action":"login","name":"test1","password":"123456","id":1}
		{"action":"message","token":"abcdefg","toid":2,"id":1,"content":"test2你好啊"}
		{"action":"register","name":"test2","password":"123456"}
		{"action":"logout","name":"test1","password":"123456","id":1}

	4）压力测试工具：
	编译命令：make clean && make
	启动命令：objs/bin/RTTest 127.0.0.1 5566 10
		4-接收数据:{"code":"0000","message":"登录成功","token":"92e7953a6e9a4a3c805bfde21258ca50","id":2}
		7-接收数据:{"code":"1003","message":"该用户已经在其他地方成功登录"}
		8-接收数据:{"code":"1003","message":"该用户已经在其他地方成功登录"}
		9-接收数据:{"code":"0000","message":"登录成功","token":"85dca709e71ec64d8b7ce8e99fade68e","id":4}
		10-接收数据:{"code":"1003","message":"该用户已经在其他地方成功登录"}
		5-接收数据:{"code":"0000","message":"登录成功","token":"8b7fe19484b6200eb604abab6afa9e2d","id":3}
		3-接收数据:{"code":"0000","message":"登录成功","token":"e0dd4aa2334825be2c0c04b197d366cb","id":1}
		6-接收数据:{"code":"1003","message":"该用户已经在其他地方成功登录"}
		11-接收数据:{"code":"1003","message":"该用户已经在其他地方成功登录"}
		12-接收数据:{"code":"1003","message":"该用户已经在其他地方成功登录"}



二、接下来要做：

	1）android客户端
	2）IOS客户端
	3）优化服务端(已经借助libevent，用bufferevent实现了epoll方式，并且是多线程的，但还有一些bug，慢慢修复)



三、GIT相关操作

cmd:
mkdir RTServer
cd RTServer
git init
git config --local user.email net.bsh@gmail.com
git config --local user.name bishenghua
git config --local core.editor vim
git config --local remote.origin.url git@github.com:RTServer/RTServer.git
git pull -f origin

添加文件:
git add test.txt
git commit -m 'test' #提交到本地master库中
git push -u origin master:master #将本地的master库提交到远程服务器的master库(同 svn ci -m 'commit')

注：
1.生成密钥：
$ssh-keygen -t rsa -C net.bsh@gmail.com
连续敲三次回车
2.$cat ~/.ssh/id_rsa.pub
将里面的内容添加到服务器上：https://github.com/settings/ssh

