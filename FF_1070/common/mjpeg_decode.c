#include "video_decode.h"
#include "user_common.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "video_input.h"
#include "user_common.h"
/***** 解码后数据输出回调函数 *****/
static void (*jpg_decode_frame_read_callback)(struct ak_vdec_frame *) = NULL;

/*****  解码句柄 *****/
static int jpg_decode_handle_id = -1;

/***** 解码使能 *****/
static bool jpg_decode_enable = false;

static int jpg_decode_frame_count = 0;

static pthread_mutex_t jpg_decode_mutex;
static pthread_mutex_t jpg_decode_io_mutex;
static bool jpg_decode_end_stream_sent = false;
/***
** 日期: 2022-05-10 11:23
** 作者: leo.liu
** 函数作用：打开解码器
** 返回参数说明：
***/
static int jpg_decode_device_open(void)
{
	struct ak_vdec_param param;
	int handle_id = -1;
	memset(&param, 0, sizeof(struct ak_vdec_param));
	param.vdec_type = MJPEG_ENC_TYPE;
	param.output_type = AK_YUV420SP;
	param.sc_width = SUB_VIDEO_PIXEL_WIDTH;
	param.sc_height = SUB_VIDEO_PIXEL_HIGHT;
	param.stream_buf_size = 512 * 1024;
	param.frame_buf_num = 3;
	ak_vdec_open(&param, &handle_id);
	if (handle_id != -1)
	{
		ak_vdec_clear_buff(handle_id);
		printf("jpeg decode open success ! \n");
	}
	else
	{
		printf("jpeg decode open failed ! \n");
	}
	return handle_id;
}

/***
** 日期: 2022-05-10 11:23
** 作者: leo.liu
** 函数作用：关闭解码器
** 返回参数说明：
***/
static void jpg_decode_device_close(int handle_id)
{
	ak_vdec_close(handle_id);
	printf("jpeg decode close success ! \n");
}

static void jpg_decode_handle_close_commit(int handle_id)
{
	pthread_mutex_lock(&jpg_decode_mutex);
	if (jpg_decode_handle_id == handle_id)
	{
		jpg_decode_handle_id = -1;
		jpg_decode_frame_count = 0;
		jpg_decode_frame_read_callback = NULL;
		jpg_decode_end_stream_sent = false;
	}
	pthread_mutex_unlock(&jpg_decode_mutex);
}

/***
** 日期: 2022-05-10 10:59
** 作者: leo.liu
** 函数作用：接收数据队列函数
** 返回参数说明：
***/
static void *jpg_decode_task(void *arg)
{
	struct ak_vdec_frame frame = {0};
	int status = 0;
	printf("***** jpg stream task create sccess ! *****\n");
	while (1)
	{
		int handle_id = -1;
		int frame_count = 0;
		bool enable = false;
		bool end_stream_sent = false;
		bool need_open = false;
		decode_finish_callback read_callback = NULL;

		pthread_mutex_lock(&jpg_decode_mutex);
		need_open = ((jpg_decode_enable == true) && (jpg_decode_handle_id == -1));
		handle_id = jpg_decode_handle_id;
		frame_count = jpg_decode_frame_count;
		enable = jpg_decode_enable;
		end_stream_sent = jpg_decode_end_stream_sent;
		pthread_mutex_unlock(&jpg_decode_mutex);

		if (need_open == true)
		{
			int new_handle_id = -1;

			pthread_mutex_lock(&jpg_decode_io_mutex);
			pthread_mutex_lock(&jpg_decode_mutex);
			need_open = ((jpg_decode_enable == true) && (jpg_decode_handle_id == -1));
			pthread_mutex_unlock(&jpg_decode_mutex);
			if (need_open == true)
			{
				new_handle_id = jpg_decode_device_open();
			}

			pthread_mutex_lock(&jpg_decode_mutex);
			if ((need_open == true) && (new_handle_id != -1) &&
				(jpg_decode_enable == true) && (jpg_decode_handle_id == -1))
			{
				jpg_decode_handle_id = new_handle_id;
				jpg_decode_frame_count = 0;
				jpg_decode_end_stream_sent = false;
				new_handle_id = -1;
			}
			pthread_mutex_unlock(&jpg_decode_mutex);

			if (new_handle_id != -1)
			{
				jpg_decode_device_close(new_handle_id);
			}
			pthread_mutex_unlock(&jpg_decode_io_mutex);
		}
		else if (handle_id != -1)
		{
			if ((enable == false) && (end_stream_sent == false))
			{
				bool send_end_stream = false;

				pthread_mutex_lock(&jpg_decode_io_mutex);
				pthread_mutex_lock(&jpg_decode_mutex);
				if ((jpg_decode_handle_id == handle_id) &&
					(jpg_decode_enable == false) &&
					(jpg_decode_end_stream_sent == false))
				{
					jpg_decode_end_stream_sent = true;
					send_end_stream = true;
				}
				pthread_mutex_unlock(&jpg_decode_mutex);
				if (send_end_stream == true)
				{
					ak_vdec_end_stream(handle_id);
				}
				pthread_mutex_unlock(&jpg_decode_io_mutex);
			}

			if (frame_count > 0)
			{
				int ret = -1;

				pthread_mutex_lock(&jpg_decode_io_mutex);
				pthread_mutex_lock(&jpg_decode_mutex);
				if ((jpg_decode_handle_id == handle_id) && (jpg_decode_frame_count > 0))
				{
					enable = jpg_decode_enable;
					read_callback = jpg_decode_frame_read_callback;
				}
				else
				{
					handle_id = -1;
				}
				pthread_mutex_unlock(&jpg_decode_mutex);

				memset(&frame, 0, sizeof(struct ak_vdec_frame));
				if (handle_id != -1)
				{
					ret = ak_vdec_get_frame(handle_id, &frame);
				}
				if (ret == 0)
				{
					if ((enable == true) && (read_callback != NULL))
					{
						read_callback(&frame);
					}
					ak_vdec_release_frame(handle_id, &frame);

					pthread_mutex_lock(&jpg_decode_mutex);
					if ((jpg_decode_handle_id == handle_id) && (jpg_decode_frame_count > 0))
					{
						jpg_decode_frame_count--;
					}
					pthread_mutex_unlock(&jpg_decode_mutex);
				}
				pthread_mutex_unlock(&jpg_decode_io_mutex);
			}

			pthread_mutex_lock(&jpg_decode_mutex);
			enable = jpg_decode_enable;
			end_stream_sent = jpg_decode_end_stream_sent;
			handle_id = jpg_decode_handle_id;
			pthread_mutex_unlock(&jpg_decode_mutex);
			if ((handle_id != -1) && (enable == false) && (end_stream_sent == true))
			{
				bool close_handle = false;

				pthread_mutex_lock(&jpg_decode_io_mutex);
				pthread_mutex_lock(&jpg_decode_mutex);
				if ((jpg_decode_handle_id == handle_id) && (jpg_decode_enable == false))
				{
					close_handle = true;
				}
				pthread_mutex_unlock(&jpg_decode_mutex);
				if (close_handle == true)
				{
					status = 0;
					ak_vdec_get_decode_finish(handle_id, &status);
					if (status)
					{
						jpg_decode_device_close(handle_id);
						jpg_decode_handle_close_commit(handle_id);
					}
				}
				pthread_mutex_unlock(&jpg_decode_io_mutex);
			}
		}
		usleep(1 * 1000);
	}
	return NULL;
}

/***
** 日期: 2022-05-10 10:55
** 作者: leo.liu
** 函数作用：jpg解码器初始化
** 返回参数说明：
***/
bool jpg_decode_init(void)
{
	static bool inited = false;
	if (inited == true)
	{
		printf("jpg decode It's already initialized \n");
		return false;
	}
	inited = true;
	pthread_t pthread;
	pthread_mutex_init(&jpg_decode_mutex, NULL);
	pthread_mutex_init(&jpg_decode_io_mutex, NULL);
	pthread_create(&pthread, user_pthread_atter_get(), jpg_decode_task, NULL);
	return true;
}

/***
** 日期: 2022-05-10 12:02
** 作者: leo.liu
** 函数作用：打开jpg解码器
** 返回参数说明：
***/
bool jpg_decode_open(void (*read_frame)(struct ak_vdec_frame *frame))
{
	pthread_mutex_lock(&jpg_decode_mutex);
	if (jpg_decode_enable == true)
	{
		pthread_mutex_unlock(&jpg_decode_mutex);
		return false;
	}
	if (jpg_decode_handle_id != -1)
	{
		printf("jpeg decoder is still closing, handle=%d\n", jpg_decode_handle_id);
		pthread_mutex_unlock(&jpg_decode_mutex);
		return false;
	}
	jpg_decode_frame_read_callback = read_frame;
	jpg_decode_end_stream_sent = false;
	jpg_decode_enable = true;
	pthread_mutex_unlock(&jpg_decode_mutex);
	return true;
}

/***
** 日期: 2022-05-10 13:33
** 作者: leo.liu
** 函数作用：
** 返回参数说明：
***/
bool jpg_decode_close(void)
{
	bool closed;

	pthread_mutex_lock(&jpg_decode_mutex);
	if (jpg_decode_enable == true)
	{
		jpg_decode_enable = false;
	}
	pthread_mutex_unlock(&jpg_decode_mutex);

	for (int count = 0; count < 100; count++)
	{
		pthread_mutex_lock(&jpg_decode_mutex);
		closed = (jpg_decode_handle_id == -1);
		pthread_mutex_unlock(&jpg_decode_mutex);
		if (closed == true)
		{
			return true;
		}
		usleep(10 * 1000);
	}

	pthread_mutex_lock(&jpg_decode_mutex);
	closed = (jpg_decode_handle_id == -1);
	if (closed == false)
	{
		printf("jpg decode close timeout, handle=%d\n", jpg_decode_handle_id);
	}
	pthread_mutex_unlock(&jpg_decode_mutex);
	return false;
}

/***
**   日期:2022-05-23 16:19:24
**   作者: leo.liu
**   函数作用：等待解码器开启
**   参数说明:
***/
static bool jpg_decode_enable_wait(void)
{
	int count = 100;
	while (1)
	{
		bool enable;
		bool ready;

		pthread_mutex_lock(&jpg_decode_mutex);
		enable = jpg_decode_enable;
		ready = (jpg_decode_handle_id != -1);
		pthread_mutex_unlock(&jpg_decode_mutex);
		if (ready == true)
		{
			return true;
		}
		if (enable == false)
		{
			return false;
		}
		usleep(10 * 1000);
		count--;
		if (count == 0)
		{
			return false;
		}
	}
	return true;
}

/***
** 日期: 2022-05-10 13:37
** 作者: leo.liu
** 函数作用：写入解码流
** 返回参数说明：
***/
bool jpg_decode_stream_write(const unsigned char *data, int size)
{
	int handle_id;

	pthread_mutex_lock(&jpg_decode_mutex);
	if (jpg_decode_enable == false)
	{
		pthread_mutex_unlock(&jpg_decode_mutex);
		return false;
	}
	else if (jpg_decode_handle_id == -1)
	{
		pthread_mutex_unlock(&jpg_decode_mutex);
		if (jpg_decode_enable_wait() == false)
		{
			return false;
		}
		pthread_mutex_lock(&jpg_decode_mutex);
	}
	handle_id = jpg_decode_handle_id;
	if (handle_id == -1)
	{
		pthread_mutex_unlock(&jpg_decode_mutex);
		return false;
	}
	pthread_mutex_unlock(&jpg_decode_mutex);

	pthread_mutex_lock(&jpg_decode_io_mutex);
	pthread_mutex_lock(&jpg_decode_mutex);
	if ((jpg_decode_enable == false) || (jpg_decode_handle_id != handle_id))
	{
		pthread_mutex_unlock(&jpg_decode_mutex);
		pthread_mutex_unlock(&jpg_decode_io_mutex);
		return false;
	}
	pthread_mutex_unlock(&jpg_decode_mutex);

	int dec_len = 0;
	int read_len = size;
	int send_len = 0;
	while (read_len > 0)
	{
		int ret;

		dec_len = 0;
		ret = ak_vdec_send_stream(handle_id, &data[send_len], read_len, NONBLOCK, &dec_len);
		if ((ret != 0) || (dec_len <= 0))
		{
			printf("jpeg decode send stream failed, ret=%d dec_len=%d read_len=%d\n",
				   ret, dec_len, read_len);
			pthread_mutex_unlock(&jpg_decode_io_mutex);
			return false;
		}
		read_len -= dec_len;
		send_len += dec_len;
	}
	pthread_mutex_lock(&jpg_decode_mutex);
	if ((jpg_decode_enable == true) && (jpg_decode_handle_id == handle_id))
	{
		jpg_decode_frame_count++;
	}
	pthread_mutex_unlock(&jpg_decode_mutex);
	pthread_mutex_unlock(&jpg_decode_io_mutex);
	return true;
}

/***
** 日期: 2022-05-10 13:41
** 作者: leo.liu
** 函数作用：判断解码线程是否彻底关闭
** 返回参数说明：
***/
bool jpg_decode_device_state(void)
{
	bool opened;

	pthread_mutex_lock(&jpg_decode_mutex);
	opened = (jpg_decode_handle_id != -1);
	pthread_mutex_unlock(&jpg_decode_mutex);
	return opened;
}
/***
**   日期:2022-05-24 16:54:22
**   作者: leo.liu
**   函数作用：修改视频输出的解码函数
**   参数说明:
***/
decode_finish_callback jpg_decode_read_frame_func_register(decode_finish_callback read_frame)
{
	pthread_mutex_lock(&jpg_decode_mutex);
	if (jpg_decode_enable == false)
	{
		pthread_mutex_unlock(&jpg_decode_mutex);
		return NULL;
	}
	decode_finish_callback old_callback = jpg_decode_frame_read_callback;
	jpg_decode_frame_read_callback = read_frame;
	pthread_mutex_unlock(&jpg_decode_mutex);
	return old_callback;
}
/***
**   日期:2022-05-25 18:21:21
**   作者: leo.liu
**   函数作用：获取带解码帧
**   参数说明:
***/
void jpg_decode_buffer_clear(void)
{
	int handle_id;

	pthread_mutex_lock(&jpg_decode_io_mutex);
	pthread_mutex_lock(&jpg_decode_mutex);
	handle_id = jpg_decode_handle_id;
	pthread_mutex_unlock(&jpg_decode_mutex);
	if (handle_id != -1)
	{
		ak_vdec_clear_buff(handle_id);
	}
	pthread_mutex_unlock(&jpg_decode_io_mutex);
}
