#include "stdafx.h"
#include "video_decoder.h"
#include <iostream>
#include <stdio.h>



void video_decoder::init(int width, int height, std::string video_file_path) {
	m_width = width;
	m_height = height;
	m_video_file_path = video_file_path;

	packet = av_packet_alloc();

	if (ret = avformat_open_input(&pFormatCtx, m_video_file_path.c_str(), NULL, NULL) < 0) {
		av_log(NULL, AV_LOG_ERROR, "cannot open input file\n");
		return;
	}
	if (ret = avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		av_log(NULL, AV_LOG_ERROR, "cannot get stream info\n");
		return;
	}
	// get video stream index
	for (int i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			VideoStreamIndex = i;
			break;
		}
	}

	if (VideoStreamIndex < 0) {
		av_log(NULL, AV_LOG_ERROR, "No video stream\n");
		return;
	}

	// dump video stream info
	av_dump_format(pFormatCtx, VideoStreamIndex, m_video_file_path.c_str(), false);



	// find decoding codec
	pCodec = avcodec_find_decoder(pFormatCtx->streams[VideoStreamIndex]->codecpar->codec_id);
	if (pCodec == NULL) {
		av_log(NULL, AV_LOG_ERROR, "No decoder found\n");
		return;
	}

	pCodecParserCtx = av_parser_init(pCodec->id);
	if (!pCodecParserCtx) {
		fprintf(stderr, "parser not found\n");
		return;
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		fprintf(stderr, "Could not allocate video codec context\n");
		return;
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		return;
	}

	fin = fopen(video_file_path.c_str(), "rb");
	if (!fin) {
		fprintf(stderr, "Could not open %s\n", video_file_path);
		exit(1);
	}
	pFrame = av_frame_alloc();
	if (!pFrame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	
	memset(in_buffer + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
	
}
void video_decoder::start() {
	size_t data_size;
	while (!feof(fin)) {
		data_size = fread(in_buffer, 1, INBUF_SIZE, fin);
		if (!data_size)
			break;

		cur_ptr = in_buffer;
		while (data_size > 0) {
			ret = av_parser_parse2(pCodecParserCtx, pCodecCtx, &packet->data, &packet->size, cur_ptr, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
			if (ret < 0) {
				fprintf(stderr, "Error while parsing\n");
				exit(1);

			}
			cur_ptr += ret;
			data_size -= ret;

			if (packet->size)
				decode(pCodecCtx, pFrame, packet, "D:\\22222.raw");
		}
	}

	decode(pCodecCtx, pFrame, NULL, "D:\\22222.raw");

	fclose(fin);

	av_parser_close(pCodecParserCtx);
	avcodec_free_context(&pCodecCtx);
	av_frame_free(&pFrame);
	av_packet_free(&packet);
}


void video_decoder::decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,
	const char *filename) {
	char buf[1024];
	int ret;

	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
		fprintf(stderr, "Error sending a packet for decoding\n");
		exit(1);

	}

	while (ret >= 0) {
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during decoding\n");
			exit(1);

		}

		printf("saving frame %3d\n", dec_ctx->frame_number);
		fflush(stdout);

		_snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);

		unsigned char* rgb_image = new unsigned char[frame->width * frame->height * 3]; //width and height of the image to be converted
		AVFrame *dstFrame = av_frame_alloc();
		dstFrame->format = AV_PIX_FMT_RGB24;
		dstFrame->width = frame->width;
		dstFrame->height = frame->height;
		convertToRGB(frame->width, frame->height, frame, dstFrame);
		bitmap_save(dstFrame->data[0], frame->width * frame->height * 3, (char *)filename);
		
	}
}

void video_decoder::pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
	char *filename) {
	FILE *f;
	int i;

	f = fopen(filename, "w");
	//fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
	for (i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);
	fclose(f);
}
void video_decoder::bitmap_save(unsigned char *buf, int size, char *filename)
{
	FILE *f;
	f = fopen(filename, "wb");
	fwrite(buf , 1, size, f);
	fclose(f);
}

void video_decoder::getDate(unsigned char* data)
{

}

void video_decoder::convertToRGB(int width, int height, AVFrame* yuyv_image, AVFrame *dstFrame)
{

	struct SwsContext *swsContext = sws_getContext(yuyv_image->width, yuyv_image->height, AV_PIX_FMT_YUV420P,
		dstFrame->width, dstFrame->height,
		(enum AVPixelFormat) dstFrame->format,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);

	int dstBufferSize = av_image_get_buffer_size((enum AVPixelFormat) dstFrame->format, dstFrame->width,
		dstFrame->height, 1);
	uint8_t *dstBuffer = (uint8_t *)av_malloc(
		sizeof(uint8_t) * dstBufferSize);

	av_image_fill_arrays(dstFrame->data, dstFrame->linesize, dstBuffer,
		(enum AVPixelFormat) dstFrame->format, dstFrame->width,
		dstFrame->height,
		1);

	sws_scale(swsContext, (const uint8_t *const *)yuyv_image->data,
		yuyv_image->linesize, 0, yuyv_image->height,
		dstFrame->data, dstFrame->linesize);



	//int y;
	//int cr;
	//int cb;
	//double r;
	//double g;
	//double b;
	//int intR;
	//int intG;
	//int intB;
	//long loopsCount = width * height * 3;
	//for (int i = 0, j = 0; i < loopsCount; i += 3, j += 1) {
	//	//first pixel
	//	y = yuyv_image->data[0][j];
	//	int index = j / 2;
	//	cb = yuyv_image->data[1][index];
	//	cr = yuyv_image->data[2][index];
	//	r = y + (1.4065 * (cr - 128));
	//	g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
	//	b = y + (1.7790 * (cb - 128));
	//	//This prevents colour distortions in your rgb image
	//	if (r < 0) r = 0;
	//	else if (r > 255) r = 255;
	//	if (g < 0) g = 0;
	//	else if (g > 255) g = 255;
	//	if (b < 0) b = 0;
	//	else if (b > 255) b = 255;
	//	
	//	rgb_image[i] = (unsigned char)r;
	//	rgb_image[i + 1] = (unsigned char)g;
	//	rgb_image[i + 2] = (unsigned char)b;
	//}
}