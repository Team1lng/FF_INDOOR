//
// Created by michael on 24-6-17.
//

#ifndef AK_ITC_UART_H
#define AK_ITC_UART_H
#include <stdint.h>
/**
 * 打开串口设备,使用 close 函数关闭已打开设备
 * @param node_name 设备节点
 * @param flag  O_RDWR ： 可读可写
                O_NOCTTY ：该参数不会使打开的文件成为该进程的控制终端。如果没有指定这个标志，那么任何一个 输入都将会影响用户的进程。
                O_NDELAY ：这个程序不关心DCD信号线所处的状态,端口的另一端是否激活或者停止。
                           如果用户不指定了这个标志，则进程将会一直处在睡眠状态，直到DCD信号线被激活。

 * @param block  设置串口阻塞， 0：阻塞， FNDELAY：非阻塞
 * @return -1： 打开失败 其他：fd (file descriptor)
 */
int raw_uart_open(const char *node_name, int flag, int block);

/**
 * 设置 RAW uart 参数
 * @param fd 串口设备文件描述
 * @param speed 波特率
 * @param n_bits 数据位数
 * @param n_parity 校验位
 * @param n_stop 停止位
 * @return -1：设置失败 0：设置成功
 */
int set_uart_options(int fd, int speed, int n_bits, int n_parity, int n_stop);

/**
 * 串口数据读取
 * @param fd 串口设备文件描述
 * @param data 读取数据缓存
 * @param size  读取数据长度
 * @param timeout 读取超时时间
 * @return -1： 读取失败 其他：读取数据长度
 */
int raw_uart_read(int fd, unsigned char  *data, int size, int timeout);

/**
 * 串口发送数据
 * @param fd 串口设备文件描述
 * @param data 数据缓存
 * @param size 数据长度
 * @return -1：发送数据失败 其他：已经发送数据长度
 */
int raw_uart_write(int fd, unsigned char  *data, int size);

#ifdef RAW_UART_TEST
#define DEV_TEST_NAME    "/dev/ttySAK3"    ///< 串口设备
/**
 * 串口设备测试函数,将收到的数据转发出去，默认波特率 115200 8位 无校验 1位停止位
 * @param name 串口设备名称
 * @return -1：失败
 */
int raw_uart_test(const  char *name);
#endif //RAW_UART_TEST

#endif //AK_ITC_UART_H
