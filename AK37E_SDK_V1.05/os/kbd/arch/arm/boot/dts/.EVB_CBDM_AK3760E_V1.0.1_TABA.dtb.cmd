cmd_arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb := mkdir -p arch/arm/boot/dts/ ; /opt/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.d.pre.tmp -nostdinc -I/home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts -I/home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/include -I/home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.dts.tmp /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb -b 0 -i /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/  -d arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.d.dtc.tmp arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.dts.tmp ; cat arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.d.pre.tmp arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.d.dtc.tmp > arch/arm/boot/dts/.EVB_CBDM_AK3760E_V1.0.1_TABA.dtb.d

source_arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb := /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dts

deps_arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb := \
  /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_ak37e.dtsi \
  /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_ak37e_common.dtsi \
  /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \
  /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/include/dt-bindings/clock/ak37e-clock.h \
  /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_ak37e_pinctrl.dtsi \
  /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/include/dt-bindings/pinctrl/ak_37e_pinctrl.h \
  /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_norflash.dtsi \
  /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_nandflash.dtsi \
  /home/zio/Share/FF_Indoor/AK37E_SDK_V1.05/os/kernel/arch/arm/boot/dts/anycloud_lcd.dtsi \

arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb: $(deps_arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb)

$(deps_arch/arm/boot/dts/EVB_CBDM_AK3760E_V1.0.1_TABA.dtb):
