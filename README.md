# ZYB 翻译笔 U-Boot

本仓库是 ZYB 翻译笔的 U-Boot 2015.07 源码，基于 [sprd-linux/u-boot15](https://github.com/sprd-linux/u-boot15) 的 `2fe7dce` 版本修改。上游通用说明保留在 [README](README) 中。

## 功能

- 支持 AOSP fastboot `fetch` 读取分区。
- 快速连续重启保护：检测到重启循环时进入 fastboot。
- 支持 raw Android boot 镜像以及 DHTB 封装的 Linux 镜像和 dtbo。
- 双启动只使用音量加键：采样 EIC 170，5 次采样中至少 3 次为高电平时选择 `boot_2`（Android），否则选择 `boot`（Linux）。

**最新 volumeup 镜像尚未经过硬件验证。** 刷写或启动存在设备无法启动及数据丢失的风险；请先备份相关分区，并确认具备设备适用的恢复方式。

## 构建

需要安装 ARM 交叉编译工具链（命令前缀 `arm-linux-gnueabi-`）及常规 U-Boot 构建依赖。在仓库根目录执行：

```sh
make ARCH=arm O=out32u CROSS_COMPILE=arm-linux-gnueabi- sp9832e_1h10_32b_defconfig
make ARCH=arm O=out32u CROSS_COMPILE=arm-linux-gnueabi- KCFLAGS='-DZCFG_TSX_32KLESS -fno-pie -fgnu89-inline' -j"$(nproc)"
```

构建输出位于 `out32u/`。仓库内发布的可刷写镜像是经过额外打包的产物，不能假定普通构建输出可直接刷写。

## 发布镜像

- 文件：[releases/32bit/uboot-flashable-32b-dualboot-volup.img](releases/32bit/uboot-flashable-32b-dualboot-volup.img)
- MD5：`0c416b964d3d5b2cb17f207a6ad23865`
- 状态：最新音量加键版本尚未实机验证。

可使用 `md5sum releases/32bit/uboot-flashable-32b-dualboot-volup.img` 校验文件完整性。
