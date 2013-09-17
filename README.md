RTServer
========

Real Time Server

bishenghua zhuna zhaiyanbin yuanjing

cmd:
mkdir RTServer
cd RTServer
git init
git config --global user.email net.bsh@gmail.com
git config --global user.name bishenghua
git config --global core.editor vim
git config --global remote.origin.url git@github.com:RTServer/RTServer.git
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
