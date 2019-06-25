#include "stdafx.h"
#include "video_decoder.h"
#include <iostream>




void video_decoder::init(int width, int height, std::string video_file_path) 
{
	m_width = width; 
	m_height = height;
	m_video_file_path = video_file_path;

	
	
	if (ret = avformat_open_input(&pFormatCtx, m_video_file_path.c_str(), NULL, NULL) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "cannot open input file\n");
		return;
	}
	if (ret = avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "cannot get stream info\n");
		return;
	}
	// get video stream index
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			VideoStreamIndex = i;
			break;
		}
	}

	if (VideoStreamIndex < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "No video stream\n");
		return;
	}

	// dump video stream info
	av_dump_format(pFormatCtx, VideoStreamIndex, m_video_file_path.c_str(), false);

	

	// find decoding codec
	pCodec = avcodec_find_decoder(pFormatCtx->streams[VideoStreamIndex]->codecpar->codec_id);
	if (pCodec == NULL)
	{
		av_log(NULL, AV_LOG_ERROR, "No decoder found\n");
		return;
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	// retrieve codec params from format context
	if (ret = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[VideoStreamIndex]->codecpar) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot get codec parameters\n");
		return;
	}

	// try to open codec
	if (ret = avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
		return;
	}

	printf("\nDecoding codec is : %s\n", pCodec->name);

	/*pCodecParserCtx = av_parser_init(codec_id);
	if (!pCodecParserCtx) {
		printf("Could not allocate video parser context\n");
		return;
	}*/

	//init packet
	packet = av_packet_alloc();
	av_init_packet(packet);
	if (!packet)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot init packet\n");
		return;
	}

	pFrame = av_frame_alloc();
	if (!pFrame)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot init frame\n");
		return;
	}
}
void video_decoder::start() 
{
	while (true)
	{
		// read an encoded packet from file
		if (ret = av_read_frame(pFormatCtx, packet) < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "cannot read frame");
			break;
		}

		// if packet data is video data then send it to decoder
		if (packet->stream_index == VideoStreamIndex)
		{
			decode(pCodecCtx, pFrame, packet, NULL);
		}
		//flush decoder
		decode(pCodecCtx, pFrame, NULL, fout);
		// release packet buffers to be allocated again
		av_packet_unref(packet);
	}
	//flush decoder
	decode(pCodecCtx, pFrame, NULL, fout);

	fp_in.close();
	fp_out.close();

#if USE_SWSCALE
	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrameYUV);
#endif

	//av_parser_close(pCodecParserCtx);

	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
}



void video_decoder::decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt, FILE *f)
{
	int ret;
	
	//send packet to decoder
	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
		fprintf(stderr, "Error sending a packet for decoding\n");
		av_log(NULL, AV_LOG_ERROR, "Cannot init frame\n");
		//exit(1);
	}
	while (ret >= 0) {
		// receive frame from decoder
		// we may receive multiple frames or we may consume all data from decoder, then return to main loop
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			// something wrong, quit program
			fprintf(stderr, "Error during decoding\n");
			exit(1);
		}
		printf("saving frame %3d\n", dec_ctx->frame_number);
		fflush(stdout);


		// display frame on sdl window
		//displayFrame(frame, dec_ctx);

		// send frame info to writing function
		//pgm_save(frame->data[0], frame->linesize[0], frame->width, frame->height, f);
	}
}

void video_decoder::pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, FILE *f)
{
	// write header
	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
	// loop until all rows are written to file
	for (int i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);
}