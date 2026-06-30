# 升级包版本字段拼写修复说明

## 针对问题

升级打包脚本在生成 env 临时配置时，写入的升级包版本字段为：

```text
uprade_image_version
```

该字段少了一个 `g`。如果后续 APP、脚本或其他工具按正确字段名 `upgrade_image_version` 读取升级包版本，会读取不到该值。

## 修改内容

修改文件：

- `AK37E_SDK_V1.05/upgrade/make_image.sh`

具体修改：

- 将写入 env 临时配置的字段名从 `uprade_image_version` 修改为 `upgrade_image_version`。

修改后脚本会生成：

```text
upgrade_image_version <YYYYMMDDHHMMSS>
```

## 注意事项

本次只修改打包脚本，不修改 APP 设置页或 Logo 页的软件版本显示逻辑。

已有的 `AK37E_SDK_V1.05/tools/envtool/env_ak3760e_nor_tmp.cfg` 是之前打包生成的临时文件，里面仍可能保留旧字段。后续重新执行打包脚本后，会按新字段重新生成。

## 验证结果

已执行文本检查：

```sh
rg -n "uprade_image_version|upgrade_image_version" AK37E_SDK_V1.05/upgrade/make_image.sh AK37E_SDK_V1.05/tools/envtool/env_ak3760e_nor_tmp.cfg
```

结果确认：

- `AK37E_SDK_V1.05/upgrade/make_image.sh` 已改为 `upgrade_image_version`。
- 旧的临时生成文件仍保留旧字段，需重新打包后更新。

## 建议提交信息

```text
fix: correct upgrade image version env key
```
