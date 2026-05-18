# IGH安装教程
## STEP1 安装依赖工具
使用以下命令：
```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install git autoconf libtool pkg-config make build-essential net-tools
```
## STEP2 安装IGH
使用以下命令：
```
git clone https://gitlab.com/etherlab.org/ethercat.git
cd ethercat
git checkout stable-1.5
./bootstrap
./configure --prefix=/usr/local/etherlab --disable-8139too --disable-eoe --enable-generic
make all modules
sudo make modules_install install
sudo depmod
```
## STEP3 配置系统
使用以下命令：
```
sudo ln -s /usr/local/etherlab/bin/ethercat /usr/bin/
sudo ln -s /usr/local/etherlab/etc/init.d/ethercat /etc/init.d/ethercat
sudo mkdir -p /etc/sysconfig
sudo cp /usr/local/etherlab/etc/sysconfig/ethercat /etc/sysconfig/ethercat
```
## STEP4 网络配置
首先创建udev规则：  
`echo 'KERNEL=="EtherCAT[0-9]*", MODE="0666"' | sudo tee /etc/udev/rules.d/99-EtherCAT.rules`  
接着编辑EtherCAT适配文件：  
`sudo gedit /etc/sysconfig/ethercat`  
最后修改MAC地址和驱动程序：  
`MASTER0_DEVICE="xx:xx:xx:xx:xx:xx" ` 这里替换成你自己的MAC地址  
`DEVICE_MODULES="generic"`
