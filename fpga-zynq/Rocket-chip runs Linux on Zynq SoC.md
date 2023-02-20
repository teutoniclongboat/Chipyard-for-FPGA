# Rocket-chip runs Linux on Zynq SoC

This tutorial shows how to boot Embedded Linux (Buildroot) on programmable logic (PL) RISC-V with ARMv7(PS) as the media, and It have quick instruction, modified you Rocket-core or build everything.
![Overview](https://i.imgur.com/AsqELUc.png)
Reference: https://github.com/ucb-bar/fpga-zynq

## I. Quick instruction
These solution would use a prebuilt image for zedboard and learn how to run binaries on RISC-V Rocket core, and you only need compatiable board - neither Vivado nor RISC-V toolchain are necessary.

First, enter the working section and clone the repository from github.
```
git clone https://github.com/ucb-bar/fpga-zynq.git
```
In this case, use `zedboard` to demostrated this solution, make the image package first, it will clone the submodule repository from github automatically.

```
cd  fpga-zynq/zedboard
make fetch-images
```

Next, the buildroot has a default hashing password, but it unrecongizable. You need to set the password yourself, install some requirement tool first.
```
sudo apt-get install mount u-boot-tools screen
```
Make a ramdisk folder to edited the buildroot's filesystem, and mount to selected folder ( we choose `fpga-images-zedboard/`).
```
make ramdisk-open
sudo mount -t ext2 ramdisk/home/root/buildroot.rootfs.ext2 fpga-images-zedboard
```
Now, you can edit file inside the buildroot, change directory to `fpga-images-zedboard/etc` and edit the file named `shadow`.
```
cd fpga-images-zedboard/etc
sudo nano shadow
```
In `etc/shadow` has recorded the Linux user's password and related information, but hashing password is not recongizable, change `user:password `to `root:root`, and be awared of the colon (`:`) could not be missing.

see reference: [Linux 的`etc/shadow`檔案結構：儲存真實密碼的地方。](https://blog.gtwang.org/linux/linux-etc-shadow-file-format/)
```
# file: etc/shadow
root:$1$2CMKSo57$U.FHm8zmyfTa8X/LTHx8o0::::::: # changed
daemon:*:10933:0:99999:7:::
bin:*:10933:0:99999:7:::
sys:*:10933:0:99999:7:::
sync:*:10933:0:99999:7:::
mail:*:10933:0:99999:7:::
www-data:*:10933:0:99999:7:::
operator:*:10933:0:99999:7:::
nobody:*:10933:0:99999:7:::
```
After chage and save, you can unmount this space and update your change to the `buildroot.rootfs.ext2`, and load into boot SD card. Plugin SD card to your host, and make sure where is it located.
```
sudo umount fpga-images-zedboard/
make ramdisk-close
sudo rm -rf ramdisk/
make load-sd SD=/media/<your_SD_card_location>
```
Eject SD card and insert it to `zedboard`, in this case we use the serial USB (We found zedboard device at `/dev/ttyACM0`) to connecting the board, and you could get into the environmnent of Petalinux of ARM.
```
screen /dev/ttyACM0 115200,cs8,-parenb,-cstopb
```
After get into the Petalinux, the password should be `root:root` as the default, you can test the Front-on-Server(`fesvr-zynq`) with `hello` example.
```
./fesvr-zynq pk hello
```
Terminal shows result.
```
root@zynq:~#./fesvr-zynq pk hello
Hello, world! 
```
Check Linux information in Petalinux `root@zynq:~#`
```
uname -a
## Linux zynq 3.15.0-xilinx-06044-g6fd59fe #231 SMP PREEMPTY <date> 2014 armv7l GNU/Linux ##
```
Bootloading buildroot into RISC-V Rocket-core.
```
./fesvr-zynq +blkdev=buildroot.rootfs.ext2 bbl
```
Login buildroot with `root:root`, while login successful, `#` will show on your terminal, and check Linux information.
```
uname -a
## Linux buildroot 4.15.0-rc6-01961-gc48c452-dirty #1 SMP <date> 2018 riscv64 GNU/Linux ##
```
The result in first`uname -a` shows Zynq Linux information would be `armv7l GUN/Linux`, and command of `uname -a`in RISC-V shows `riscv64 GUN/Linux`.

After linux boots you'll be presented with a busybox prompt from riscv-linux running on rocket!

## II. Push your Rocket-chip modifications to the FPGA
**(Researching...)**

In these solution, Vivado and JVM with **correct version** (super important !!) are required.

For JVM version, it had better to develop with JDK v8.0, sbt v0.13.12.
```
sudo apt-get install openjdk-8-jre sbt=0.13.12
```
Switching the java version to desired, as following example, type `2` to change the java version.
```
sudo update-alternatives --config java

#####
 There are 2 choices for the alternative java (providing /usr/bin/java).

 Selection    Path                                            Priority   Status

 * 0            /usr/lib/jvm/java-11-openjdk-amd64/bin/java       1111      auto mode
   1            /usr/lib/jvm/java-11-openjdk-amd64/jre/bin/java   1111      manual mode
   2            /usr/lib/jvm/java-8-openjdk-amd64/bin/java        1081      manual mode
######
```
