# APP 版本日期重新编译修复

## 问题现象

打包 `app` 分区升级后，设置里的版本日期没有更新。

## 原因

当前版本日期由 `layout_logo.c` 中的 `__DATE__` 编译宏生成：

```c
char temp[] = __DATE__;
```

`__DATE__` 只会在源文件重新编译时更新。如果只重新打包 `app` 分区，或者 CMake 认为 `layout_logo.c` 没变化，就会复用旧的 `layout_logo.c.o`，导致新生成的 `FF.BIN` 仍然带旧日期。

原 `Makefile` 中用于删除旧对象文件的命令被注释，并且路径是旧工程路径 `/home/wxj/...`，在当前工程 `/home/leo/workspace/FF_Indoor/FF_1070` 下即使取消注释也删不到正确对象文件。

## 修改内容

修改 `FF_1070/Makefile`：

```make
rm -f build/CMakeFiles/FF.BIN.dir$(CURDIR)/layout/layout_logo.c.o
```

每次执行 `cd FF_1070 && make` 时，先删除当前工作区下的 `layout_logo.c.o`，强制重新编译 `layout_logo.c`，让 `__DATE__` 更新到本次编译日期。

## 影响范围

- 只影响 `FF_1070` 的 APP 构建流程。
- 不改变 UI 显示格式。
- 不改变升级包分区结构。
- 会让每次 `make` 都重新编译 `layout_logo.c` 并重新链接 `FF.BIN`。

## 验证方式

```sh
cd FF_1070
make
strings build/FF.BIN | grep 2026
```

也可以用月份字符串确认，例如：

```sh
strings build/FF.BIN | grep "Jun"
```

升级 `app` 分区后，设置关于界面的版本日期应显示本次编译日期。

## 建议提交信息

```text
fix: force rebuild logo version date object
```
