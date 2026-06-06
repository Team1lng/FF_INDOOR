CROSS_COMPILE="/home/leo/workspace/FF_Indoor/AK37E_SDK_V1.05/tools/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-"
mkdir ../ubd
make O=../ubd/ anycloud_ak37e_config CROSS_COMPILE=$CROSS_COMPILE
make  O=../ubd/ all -j8 CROSS_COMPILE=$CROSS_COMPILE
