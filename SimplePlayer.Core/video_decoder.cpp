#include "stdafx.h"
#include "video_decoder.h"
#include <iostream>




void video_decoder::init(int width, int height, std::string video_file_path) 
{
	m_width = width; 
	m_height = height;
	m_video_file_path = video_file_path;

	
//#if TEST_HEVC
//	enum AVCodecID codec_id = AV_CODEC_ID_HEVC;
//	char filepath_in[] = "bigbuckbunny_480x272.hevc";
//#elif TEST_H264
//	AVCodecID codec_id = AV_CODEC_ID_H265;
//	char filepath_in[] = "bigbuckbunny_480x272.h264";
//#else
//	AVCodecID codec_id = AV_CODEC_ID_MPEG2VIDEO;
//	char filepath_in[] = "bigbuckbunny_480x272.m2v";
//#endif
//
//	char filepath_out[] = "bigbuckbunny_480x272.yuv";
//	int first_time = 1;
//
//#if USE_SWSCALE
//	struct SwsContext *img_convert_ctx;
//	AVFrame	*pFrameYUV;
//	uint8_t *out_buffer;
//
//#endif
	
	//avcodec_register_all();
	//av_register_all();

	
	
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

	pCodecCtx = avcodec_alloc_context3(NULL);
	// retrieve codec params from format context
	if (ret = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[VideoStreamIndex]->codecpar) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot get codec parameters\n");
		return;
	}

	// find decoding codec
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		av_log(NULL, AV_LOG_ERROR, "No decoder found\n");
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
	
	//Input File
	fp_in.open(m_video_file_path.c_str(), std::ios::in | std::ios::binary);
	if (!fp_in) {
		printf("Could not open input stream\n");
		return;
	}

	////Output File
	//fp_out.open(filepath_out, std::ios::out | std::ios::binary);
	//if (!fp_out) {
	//	printf("Could not open output YUV file\n");
	//	return;
	//}


	

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
		decode(pCodecCtx, pFrame, NULL, NULL);
		// release packet buffers to be allocated again
		av_packet_unref(packet);
	}
	
//	while (true)
//	{
//		fp_in.read(in_buffer, INBUF_SIZE);
//		cur_size = fp_in.gcount();
//		if (cur_size == 0)
//			break;
//		cur_ptr = in_buffer;
//
//		while (cur_size > 0)
//		{
//			int len = av_parser_parse2(
//				pCodecParserCtx, pCodecCtx,
//				&packet.data, &packet.size,
//				(uint8_t *)cur_ptr, cur_size,
//				AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
//
//			cur_ptr += len;
//			cur_size -= len;
//
//			if (packet.size == 0)
//				continue;
//
//			//Some Info from AVCodecParserContext
//			printf("[Packet]Size:%6d\t", packet.size);
//			switch (pCodecParserCtx->pict_type) {
//			case AV_PICTURE_TYPE_I: printf("Type:I\t"); break;
//			case AV_PICTURE_TYPE_P: printf("Type:P\t"); break;
//			case AV_PICTURE_TYPE_B: printf("Type:B\t"); break;
//			default: printf("Type:Other\t"); break;
//			}
//			printf("Number:%4d\n", pCodecParserCtx->output_picture_number);
//
//			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
//			if (ret < 0) {
//				printf("Decode Error.\n");
//				return;
//			}
//
//			if (got_picture) {
//#if USE_SWSCALE
//				if (first_time) {
//					printf("\nCodec Full Name:%s\n", pCodecCtx->codec->long_name);
//					printf("width:%d\nheight:%d\n\n", pCodecCtx->width, pCodecCtx->height);
//					//SwsContext
//					img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
//						pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
//
//					pFrameYUV = av_frame_alloc();
//					out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
//					avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
//
//					y_size = pCodecCtx->width*pCodecCtx->height;
//
//					first_time = 0;
//				}
//				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
//					pFrameYUV->data, pFrameYUV->linesize);
//
//				fwrite(pFrameYUV->data[0], 1, y_size, fp_out);     //Y 
//				fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_out);   //U
//				fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_out);   //V
//#else
//				int i = 0;
//				const char* tempptr = (const char *)pFrame->data[0];
//				for (i = 0; i < pFrame->height; i++) {      
//					fp_out.write(tempptr, pFrame->width);  //Y
//					tempptr += pFrame->linesize[0];
//				}
//				tempptr = (const char *)pFrame->data[1];
//				for (i = 0; i < pFrame->height / 2; i++) {   
//					fp_out.write(tempptr, pFrame->width / 2);  //U
//					tempptr += pFrame->linesize[1];
//				}
//				tempptr = (const char *)pFrame->data[2];
//				for (i = 0; i < pFrame->height / 2; i++) {   
//					fp_out.write(tempptr, pFrame->width / 2);  //V
//					tempptr += pFrame->linesize[2];
//				}
//#endif
//
//				printf("Succeed to decode 1 frame!\n");
//			}
//		}
//	}
//
//	//Flush Decoder
//	packet.data = NULL;
//	packet.size = 0;
//	while (true)
//	{
//		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
//		if (ret < 0) {
//			printf("Decode Error.\n");
//			return
//				;
//		}
//		if (!got_picture)
//			break;
//		if (got_picture) {
//
//#if USE_SWSCALE
//			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
//				pFrameYUV->data, pFrameYUV->linesize);
//
//			fwrite(pFrameYUV->data[0], 1, y_size, fp_out);     //Y
//			fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_out);   //U
//			fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_out);   //V
//#else
//			int i = 0;
//			unsigned char* tempptr = NULL;
//			tempptr = pFrame->data[0];
//			for (i = 0; i < pFrame->height; i++) {    
//				fp_out.write((const char *)tempptr, pFrame->width); //Y 
//				tempptr += pFrame->linesize[0];
//			}
//			tempptr = pFrame->data[1];
//			for (i = 0; i < pFrame->height / 2; i++) {  
//				fp_out.write((const char *)tempptr, pFrame->width / 2); //U
//				tempptr += pFrame->linesize[1];
//			}
//			tempptr = pFrame->data[2];
//			for (i = 0; i < pFrame->height / 2; i++) {
//				fp_out.write((const char *)tempptr, pFrame->width / 2); //V
//				tempptr += pFrame->linesize[2];
//			}
//#endif
//			printf("Flush Decoder: Succeed to decode 1 frame!\n");
//		}
//	}

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
		exit(1);
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