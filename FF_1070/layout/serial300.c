/**
 *  @file   serial300.c
 *  @brief  serial driver to t300 SoC
 *  $Id: serial300.c,v 1.4 2014/02/27 07:55:07 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.4 $
 *
 *  Copyright (c) 2006 Terawins Inc. All rights reserved.
 *
 *  @date   2007/01/04  New file.
 *
 */

#include <stdio.h>
#include <sys.h>
#include <io.h>
#include <serial300.h>
#include <common/irq.h>
#include <common/interrupt.h>

#define AFC_UART_BASE       0xb2400000
#define UART_BASE	        0xb2000000
#define UART_RBR(x)         ((x) + 0x00)
#define UART_THR(x)         ((x) + 0x00)
#define UART_DLL(x)	        ((x) + 0x00)
#define UART_DLH(x)         ((x) + 0x04)
#define UART_IER(x)		    ((x) + 0x04)
#define UART_IIR(x)         ((x) + 0x08)
#define UART_FCR(x)         ((x) + 0x08)
#define UART_LCR(x)			((x) + 0x0C)
#define UART_MCR(x)			((x) + 0x10)
#define UART_LSR(x)         ((x) + 0x14)
#define UART_MSR(x)         ((x) + 0x18)
#define UART_USR(x)			((x) + 0x7C)

#define MAX_FIFO    32
static unsigned char fifo[MAX_FIFO];
static unsigned char qf = 0, qt = 0;

unsigned char Enter_one_power = 0;
/******************************************************************************
*
* serial_init - initialize a channel
*
* This routine initializes the number of data bits, parity
* and set the selected baud rate. Interrupts are disabled.
* Set the modem control signals if the option is selected.
*
* RETURNS: N/A
*/
int serial_init(unsigned long uart_baud)
{
    unsigned clkdiv;

    writeb(0x83, UART_LCR(UART_BASE));  /* Divisor Latch */

    clkdiv = (sys_apb_clk + (8 * uart_baud)) / (16 * uart_baud);

    writeb(clkdiv >> 8, UART_DLH(UART_BASE));
    writeb(clkdiv & 0xff, UART_DLL(UART_BASE));

    writeb(0x03, UART_LCR(UART_BASE));

    // FIFO mode enable
    writeb(0x81, UART_FCR(UART_BASE));

    return 0;
}

static void fill_fifo(void)
{
    while (readb(UART_LSR(UART_BASE)) & 0x01)
    {
        fifo[qf++] = readb(UART_RBR(UART_BASE));
        if (qf >= MAX_FIFO)
        {
            qf = 0;
        }
    }
}


static void uart_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
    fill_fifo();
}

int uart_irq_open(enum RCV_THRD thrd)
{
    writeb(0x03 | ((thrd << 6) & 0xC0), UART_FCR(UART_BASE));
    writeb(0x00, UART_IER(UART_BASE));   // 禁止硬件中断
    return 0;
}

int uart_irq_close(void)
{
    writeb(0, UART_IER(UART_BASE));
    return 0;
}

void UartSend(const char c)
{
    int i;

    if (!Enter_one_power)
    {
        return ;
    }
    
    for (i = 0;  i < 100000; i++)
    {
        if (readb(UART_LSR(UART_BASE)) & 0x40) /* until Trasnsmitter empty */
        {
            break;
        }
    }
    writeb(c, UART_THR(UART_BASE));
}
/* non-block getb, if no character ready then return -1 */
int UartReceive(void)
{
    int data;
	
    if (!Enter_one_power)
    {
        return 0;
    }
    
    if (qt == qf)
    {
        return -1;
    }
    else
    {
        data = (int)(fifo[qt++]);
        if (qt >= MAX_FIFO)
        {
            qt = 0;
        }
    }
    return data;
}

void putb2(const char c)
{
    //UartSend(c);
    //afc_putb2(c);///<
}

void putb(const char c)
{
    if (!config_console_enable)
    {
        return;
    }

    if (c == '\n')
    {
        putb2('\r');
    }

    putb2(c);
}

/* non-block getb, if no character ready then return -1 */
int getb2(void)
{

}

/* block getb, if no character ready then return -1 */
int getb(void)
{
    int c;
    while (1)
    {
        if ((c = getb2()) >= 0)
        {
            break;
        }
    }
    return c;
}


/******************************************************************************
*
* serial_init - initialize a channel
*
* This routine initializes the number of data bits, parity
* and set the selected baud rate. Interrupts are disabled.
* Set the modem control signals if the option is selected.
*
* RETURNS: N/A
*/
int afc_serial_init(unsigned long uart_baud)
{
    unsigned clkdiv;

    writeb(0x83, UART_LCR(AFC_UART_BASE));  /* Divisor Latch */

    clkdiv = (sys_apb_clk + (8 * uart_baud)) / (16 * uart_baud);

    writeb(clkdiv >> 8, UART_DLH(AFC_UART_BASE));
    writeb(clkdiv & 0xff, UART_DLL(AFC_UART_BASE));

    writeb(0x03, UART_LCR(AFC_UART_BASE));

    // FIFO mode enable
    writeb(0x81, UART_FCR(AFC_UART_BASE));

    //Enable CTS RTS Function
    //afc_enable_CTSRTS(1);

    return 0;
}

static unsigned char fifo_w = 0, fifo_r = 0;
#define MAX_FIFO    128
static unsigned char afc_fifo[MAX_FIFO];

void afc_fill_fifo()
{
    while (readb(UART_LSR(AFC_UART_BASE)) & 0x01)
    {
        afc_fifo[fifo_w++] = readb(UART_RBR(AFC_UART_BASE));
        if (fifo_w >= MAX_FIFO)
        {
            fifo_w = 0;
        }
        printf("[AFC_ISR_RX] %02X w=%d r=%d\n",afc_fifo[fifo_w - 1], fifo_w, fifo_r);
    }
}

static void afc_uart_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
    afc_fill_fifo();
}

int afc_uart_irq_open(enum RCV_THRD thrd)
{
    writeb(0x03 | ((thrd << 6) & 0xC0), UART_FCR(AFC_UART_BASE));
    writeb(0x00, UART_IER(AFC_UART_BASE));   // 禁止硬件中断
    return 0;
}

int afc_uart_irq_close(void)
{
    writeb(0, UART_IER(AFC_UART_BASE));
    return 0;
}

/* non-block getb, if no character ready then return -1 */
int afc_getb2(void)
{
    int data;
    if (!Enter_one_power)
    {
        return -1;
    }
    if (fifo_r == fifo_w)
    {
        return -1;
    }
    else
    {
        data = (int)afc_fifo[fifo_r++];
        if (fifo_r >= MAX_FIFO)
        {
            fifo_r = 0;
        }
        printf("[AFC_RX] %02X\n", data & 0xFF);
        return data;
    }
}

/* block getb, if no character ready then return -1 */
int afc_getb(void)
{
    int c;
    while (1)
    {
        if ((c = afc_getb2()) >= 0)
            break;
    }
    return c;
}


#ifndef INTERCOM_UART_BYTE_LOG
#define INTERCOM_UART_BYTE_LOG 0
#endif

void afc_putb2(const char c)
{
#if INTERCOM_UART_BYTE_LOG
    printf("[UART_TX] 0x%02X (%d)\n", c, c);
#endif
    int i;
    if (!Enter_one_power)
    {
        return ;
    }

    for (i = 0;  i < 100000; i++)
    {
        if (readb(UART_LSR(AFC_UART_BASE)) & 0x40) /* until Trasnsmitter empty */
            break;
    }

    writeb(c, UART_THR(AFC_UART_BASE));
}

void afc_putb(const char c)
{
    if (c == '\n')
        afc_putb2('\r');

    afc_putb2(c);
}

void afc_enable_CTSRTS(int enable)
{
    if (enable)
        writeb(0x22, UART_MCR(AFC_UART_BASE));
    else
        writeb(0x00, UART_MCR(AFC_UART_BASE));
}


void CleanUart(void)
{
	fifo_w = 0;
	fifo_r = 0;
}
