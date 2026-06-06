//
// Created by michael on 24-6-17.
//

#include "raw_uart.h"


#include <sys/select.h>     //select函数
#include <sys/types.h>      //定义数据类型，如 ssiz e_t off_t 等

#include <stdio.h>          //标准输入输出,如printf、scanf 以及文件操作
#include <unistd.h>         //定义 read write close lseek 等Unix标准函数
#include <fcntl.h>          //文件控制定义
#include <termios.h>        //终端I/O
#include <string.h>         //字符串操作
#include <time.h>           //时间
#include <assert.h>         //断言

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
int raw_uart_open(const char *node_name, int flag, int block) {

  assert((block == 0) || (block == FNDELAY));
  if (node_name == NULL) {
    return -1;
  }
  int fd = open(node_name, flag);
  if (fd < 0) {
    perror(node_name);
    return -1;
  }
  // 设置串口阻塞， 0：阻塞， FNDELAY：非阻塞
  if (fcntl(fd, F_SETFL, block) < 0) {
    printf("fcntl failed!\n");
  }
  // 判断是否是控制台
  if (isatty(fd) == 0) {
    printf("standard input is not a terminal device\n");
    close(fd);
    return -1;
  } else {
    printf("is a tty success!\n");
  }
  printf("fd-open=%d\n", fd);
  return fd;
}

/**
 * 设置 RAW uart 参数
 * @param fd 串口设备文件描述
 * @param speed 波特率
 * @param n_bits 数据位数
 * @param n_parity 校验位
 * @param n_stop 停止位
 * @return -1：设置失败 0：设置成功
 */
int set_uart_options(int fd, int speed, int n_bits, int n_parity, int n_stop) {

  struct termios new_io, old_io;
  // 保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
  if (tcgetattr(fd, &old_io) != 0) {
    perror("SetupSerial 1");
    return -1;
  }

  bzero(&new_io, sizeof(new_io));   //新termios参数清零
  new_io.c_cflag |= CLOCAL | CREAD;      //CLOCAL--忽略 modem 控制线,本地连线, 不具数据机控制功能, CREAD--使能接收标志
  // 设置数据位数
  new_io.c_cflag &= ~CSIZE;              //清数据位标志
  switch (n_bits) {
    case 7:new_io.c_cflag |= CS7;
      break;
    case 8:new_io.c_cflag |= CS8;
      break;
    default:fprintf(stderr, "Unsupported data size\n");
      return -1;
  }
  // 设置校验位
  switch (n_parity) {
    case 'o':
    case 'O':                     //奇校验
      new_io.c_cflag |= PARENB;
      new_io.c_cflag |= PARODD;
      new_io.c_iflag |= (INPCK | ISTRIP);
      break;
    case 'e':
    case 'E':                     //偶校验
      new_io.c_iflag |= (INPCK | ISTRIP);
      new_io.c_cflag |= PARENB;
      new_io.c_cflag &= ~PARODD;
      break;
    case 'n':
    case 'N':                    //无校验
      new_io.c_cflag &= ~PARENB;
      break;
    default:fprintf(stderr, "Unsupported parity\n");
      return -1;
  }
  // 设置停止位
  switch (n_stop) {
    case 1:new_io.c_cflag &= ~CSTOPB;
      break;
    case 2:new_io.c_cflag |= CSTOPB;
      break;
    default:fprintf(stderr, "Unsupported stop bits\n");
      return -1;
  }
  // 设置波特率 2400/4800/9600/19200/38400/57600/115200/230400
  switch (speed) {
    case 2400:cfsetispeed(&new_io, B2400);
      cfsetospeed(&new_io, B2400);
      break;
    case 4800:cfsetispeed(&new_io, B4800);
      cfsetospeed(&new_io, B4800);
      break;
    case 9600:cfsetispeed(&new_io, B9600);
      cfsetospeed(&new_io, B9600);
      break;
    case 19200:cfsetispeed(&new_io, B19200);
      cfsetospeed(&new_io, B19200);
      break;
    case 38400:cfsetispeed(&new_io, B38400);
      cfsetospeed(&new_io, B38400);
      break;
    case 57600:cfsetispeed(&new_io, B57600);
      cfsetospeed(&new_io, B57600);
      break;
    case 115200:cfsetispeed(&new_io, B115200);
      cfsetospeed(&new_io, B115200);
      break;
    case 230400:cfsetispeed(&new_io, B230400);
      cfsetospeed(&new_io, B230400);
      break;
    default:printf("\tSorry, Unsupported baud rate, set default 9600!\n\n");
      cfsetispeed(&new_io, B9600);
      cfsetospeed(&new_io, B9600);
      break;
  }
  // 设置read读取最小字节数和超时时间
  new_io.c_cc[VTIME] = 1;         // 读取一个字符等待1*(1/10)s
  new_io.c_cc[VMIN] = 1;          // 读取字符的最少个数为1

  tcflush(fd, TCIFLUSH);         //清空缓冲区X
  //激活新设置
  if (tcsetattr(fd, TCSANOW, &new_io) != 0) {
    perror("SetupSerial TCSANOW");
    return -1;
  }
  printf("Serial set done!\n");
  tcflush(fd, TCIOFLUSH);    //清掉串口缓存
  return 0;
}

/**
 * 串口数据读取
 * @param fd 串口设备文件描述
 * @param data 读取数据缓存
 * @param size  读取数据长度
 * @param timeout 读取超时时间
 * @return -1： 读取失败 其他：读取数据长度
 */
int raw_uart_read(int fd, unsigned char *data, int size, int timeout) {
  int read_size, fs_sel;
  fd_set fs_read;
  struct timeval time;

  time.tv_sec = timeout / 1000;            //set the rcv wait time
  time.tv_usec = timeout % 1000 * 1000;    //100000us = 0.1s

  FD_ZERO(&fs_read);        //每次循环都要清空集合，否则不能检测描述符变化
  FD_SET(fd, &fs_read);     //添加描述符

  // 超时等待读变化，>0：就绪描述字的正数目， -1：出错， 0 ：超时
  fs_sel = select(fd + 1, &fs_read, NULL, NULL, &time);
  //    printf("fs_sel = %d\n", fs_sel);
  if (fs_sel) {
    read_size = read(fd, data, size);
    return read_size;
  } else {
//    printf("read timeout 10s\n");
    return -1;
  }
}

/**
 * 串口发送数据
 * @param fd 串口设备文件描述
 * @param data 数据缓存
 * @param size 数据长度
 * @return -1：发送数据失败 其他：已经发送数据长度
 */
int raw_uart_write(int fd, unsigned char *data, int size) {
  ssize_t ret = write(fd, data, size);
  if (ret == size) {
#ifdef ITC_UART
    char str[1024] = {0};
    for (int i = 0; i < size; ++i) {
      sprintf(&str[i * 3], " %2.2x", data[i]);
    }
    printf("UART SEND START >> %s \n", str);
#endif
    return ret;
  } else {
    perror("write device error\n");
    tcflush(fd, TCOFLUSH);
    return -1;
  }
}

#ifdef RAW_UART_TEST
/**
 * 串口设备测试函数,将收到的数据转发出去，默认波特率 115200 8位 无校验 1位停止位
 * @param name 串口设备名称
 * @return -1：失败
 */
int raw_uart_test(const char *name) {
  // 打开串口设备
  //O_RDWR ： 可读可写
  //O_NOCTTY ：该参数不会使打开的文件成为该进程的控制终端。如果没有指定这个标志，那么任何一个 输入都将会影响用户的进程。
  //O_NDELAY ：这个程序不关心DCD信号线所处的状态,端口的另一端是否激活或者停止。如果用户不指定了这个标志，则进程将会一直处在睡眠状态，直到DCD信号线被激活。
  int fd = raw_uart_open(name, O_RDWR | O_NOCTTY | O_NDELAY, 0);
  if (fd < 0) {
    perror(name);
    return -1;
  }
  // 设置串口参数: 8位数据位、1位停止位、无校验
  if (set_uart_options(fd, 115200, 8, 'N', 1) == -1) {
    fprintf(stderr, "Set opt Error\n");
    close(fd);
    exit(1);
  }

  uint8_t receive_data[100];
  //循环读取数据
  while (1) {
    int size = raw_uart_read(fd, receive_data, 99, 10000);
    if (size > 0) {
      receive_data[size] = '\0';
      printf("receive data[%d] is %s\n", size, receive_data);
      raw_uart_write(fd, receive_data, size);
    }
    usleep(10000);    //10ms
  }
}
#endif //RAW_UART_TEST
