#pragma once

#include <string>
#include <fstream>

extern "C" {
	#include "libswscale\swscale.h"
	#include "libavcodec\avcodec.h"
	#include "libswresample\swresample.h"
	#include "libavutil\mathematics.h"
	#include "libavformat\avformat.h"
	#include "libavutil\opt.h"
	#include <libavutil\channel_layout.h>
	#include <libavutil\frame.h>
	#include <libavutil\time.h>
	#include <libavutil\mathematics.h>
	#include <libavutil\imgutils.h>
}

#define INBUF_SIZE					 8192
#define USE_SWSCALE					 0

//test different codec
#define TEST_H264					 1
#define TEST_HEVC					 0

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#endif


//#ifdef	VIDEOCAPTURE_EXPORTS  
#define VIDEODECODER_API __declspec(dllexport)   
/*#else  
#define VIDEOCAPTURE_API __declspec(dllimport)   
#endif*/ 

class video_decoder
{
private:
	int						m_width;
	int						m_height;
	std::string				m_video_file_path;
	AVCodec*				pCodec;
	AVCodecContext* 		pCodecCtx = NULL;
	AVCodecParserContext*	pCodecParserCtx = NULL;
	AVFormatContext*		pFormatCtx = NULL;

	std::ifstream			fp_in;
	std::ofstream			fp_out;
	AVFrame	*				pFrame;
	AVFrame	*				pRGBFrame;

	uint8_t					in_buffer[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	uint8_t *				cur_ptr;
	long long				cur_size;
	AVPacket*				packet;
	int						ret, got_picture;
	int						y_size;
	int						VideoStreamIndex = -1;
	FILE *fin = NULL; 
	FILE *fout = NULL;
	void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,
		const char *filename);
	void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
		char *filename);

	void convertToRGB(int width, int height, AVFrame* yuyv_image, AVFrame *dstFrame);
	void bitmap_save(unsigned char *buf, int size, char *filename);
public:

	VIDEODECODER_API void init(int width, int height, std::string video_file_path);
	VIDEODECODER_API void start();
	VIDEODECODER_API void getDate(unsigned char* data);
};

