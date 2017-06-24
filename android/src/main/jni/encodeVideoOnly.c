/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <android/log.h>
#include "encodeVideoOnly.h"

typedef struct FilteringContext {
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
} FilteringContext;

typedef struct StreamContext {
    AVCodecContext *dec_ctx;
    AVCodecContext *enc_ctx;
} StreamContext;

typedef struct ConversionData {
    AVFormatContext *ifmt_ctx;
    AVFormatContext *ofmt_ctx;
    int video_idx;
    int64_t nb_frames;
    int64_t nb_frames_written;
    FilteringContext *filter_ctx;
    StreamContext *stream_ctx;
};

static int find_video_stream_idx(AVFormatContext *fmt_ctx) {
    unsigned int i;
    for (i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *stream = fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            return i;
        }
    }
    return -1;
}

static int open_input_file(struct ConversionData *cData, const char *filename) {
    int ret;
    cData->video_idx = -1;
    cData->ifmt_ctx = NULL;

    if ((ret = avformat_open_input(&cData->ifmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(cData->ifmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    cData->stream_ctx = (StreamContext *) av_mallocz_array(1, sizeof (*cData->stream_ctx)); /*ifmt_ctx->nb_streams*/
    if (!cData->stream_ctx)
        return AVERROR(ENOMEM);

    if ((cData->video_idx = find_video_stream_idx(cData->ifmt_ctx)) < 0)
        return -1;

    AVStream *stream = cData->ifmt_ctx->streams[cData->video_idx];
    cData->nb_frames = stream->nb_frames;
    AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
    AVCodecContext *codec_ctx;
    if (!dec) {
        av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for video stream\n");
        return AVERROR_DECODER_NOT_FOUND;
    }
    codec_ctx = avcodec_alloc_context3(dec);
    if (!codec_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for video stream\n");
        return AVERROR(ENOMEM);
    }
    ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
                "for video stream\n");
        return ret;
    }
    codec_ctx->framerate = av_guess_frame_rate(cData->ifmt_ctx, stream, NULL);
    /* Open decoder */
    ret = avcodec_open2(codec_ctx, dec, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream\n");
        return ret;
    }
    cData->stream_ctx[0].dec_ctx = codec_ctx;

    av_dump_format(cData->ifmt_ctx, 0, filename, 0);
    return 0;
}

static int open_output_file(struct ConversionData *cData, const char *filename, const char *out_codec_str) {
    AVStream *out_stream;
    AVCodecContext *dec_ctx, *enc_ctx;
    AVCodec *encoder;
    int ret;

    cData->ofmt_ctx = NULL;
    avformat_alloc_output_context2(&cData->ofmt_ctx, NULL, NULL, filename);
    if (!cData->ofmt_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
        return AVERROR_UNKNOWN;
    }

    out_stream = avformat_new_stream(cData->ofmt_ctx, NULL);
    if (!out_stream) {
        av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
        return AVERROR_UNKNOWN;
    }
    dec_ctx = cData->stream_ctx[0].dec_ctx;

    encoder = avcodec_find_encoder_by_name(out_codec_str);
    if (!encoder) {
        av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
        return AVERROR_INVALIDDATA;
    }
    enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
        av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
        return AVERROR(ENOMEM);
    }

    /* In this example, we transcode to same properties (picture size,
     * sample rate etc.). These properties can be changed for output
     * streams easily using filters */
    enc_ctx->height = dec_ctx->height;
    enc_ctx->width = dec_ctx->width;
    enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
    /* take first format from list of supported formats */
    if (encoder->pix_fmts)
        enc_ctx->pix_fmt = encoder->pix_fmts[0];
    else
        enc_ctx->pix_fmt = dec_ctx->pix_fmt;
    /* video time_base can be set to whatever is handy and supported by encoder */
    enc_ctx->time_base = av_inv_q(dec_ctx->framerate);

    /* Third parameter can be used to pass settings to encoder */
    ret = avcodec_open2(enc_ctx, encoder, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream\n");
        return ret;
    }
    ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream\n");
        return ret;
    }
    if (cData->ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    out_stream->time_base = enc_ctx->time_base;
    cData->stream_ctx[0].enc_ctx = enc_ctx;

    av_dump_format(cData->ofmt_ctx, 0, filename, 1);

    if (!(cData->ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&cData->ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
            return ret;
        }
    }
    av_log(NULL, AV_LOG_DEBUG, "Num of streams in output file %u\n", cData->ofmt_ctx->nb_streams);
    /* init muxer, write output file header */
    ret = avformat_write_header(cData->ofmt_ctx, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
        return ret;
    }

    return 0;
}

static int init_filter(FilteringContext* fctx, AVCodecContext *dec_ctx,
        AVCodecContext *enc_ctx, const char *filter_spec) {
    char args[512];
    int ret = 0;
    AVFilter *buffersrc = NULL;
    AVFilter *buffersink = NULL;
    AVFilterContext *buffersrc_ctx = NULL;
    AVFilterContext *buffersink_ctx = NULL;
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVFilterGraph *filter_graph = avfilter_graph_alloc();

    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    buffersrc = avfilter_get_by_name("buffer");
    buffersink = avfilter_get_by_name("buffersink");
    if (!buffersrc || !buffersink) {
        av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
             dec_ctx->time_base.num, dec_ctx->time_base.den,
             dec_ctx->sample_aspect_ratio.num,
             dec_ctx->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

    ret = av_opt_set_bin(buffersink_ctx, "pix_fmts",
                         (uint8_t *) &enc_ctx->pix_fmt, sizeof(enc_ctx->pix_fmt),
                         AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /* Endpoints for the filter graph. */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if (!outputs->name || !inputs->name) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_spec,
                                     &inputs, &outputs, NULL)) < 0) {
        __android_log_print(ANDROID_LOG_INFO, "react-native-ffmpeg",
                            "avfilter_graph_parse2 failed");
        goto end;
    }

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0) {
        __android_log_print(ANDROID_LOG_INFO, "react-native-ffmpeg", "avfilter_graph_config failed");
        goto end;
    }

    __android_log_print(ANDROID_LOG_INFO, "react-native-ffmpeg", "Number of filters setup are %u", filter_graph->nb_filters);

    /* Fill FilteringContext */
    fctx->buffersrc_ctx = buffersrc_ctx;
    fctx->buffersink_ctx = buffersink_ctx;
    fctx->filter_graph = filter_graph;

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}

static int init_filters(struct ConversionData *cData, const char *filter_spec) {
    int ret;
    cData->filter_ctx = av_malloc_array(1, sizeof (*cData->filter_ctx));
    if (!cData->filter_ctx)
        return AVERROR(ENOMEM);

    cData->filter_ctx[0].buffersrc_ctx = NULL;
    cData->filter_ctx[0].buffersink_ctx = NULL;
    cData->filter_ctx[0].filter_graph = NULL;
    AVFilter *f = avfilter_get_by_name("paletteuse");
    if (f) {
        __android_log_print(ANDROID_LOG_INFO, "react-native-ffmpeg",  "Filter paletteuse found");
    } else {
        __android_log_print(ANDROID_LOG_INFO, "react-native-ffmpeg",  "Filter paletteuse NOT found");
    }
    //filter_spec = "split[a][b],[a]palettegen=stats_mode=2[a],[b][a]paletteuse=new=1";

    ret = init_filter(&cData->filter_ctx[0], cData->stream_ctx[0].dec_ctx,
                      cData->stream_ctx[0].enc_ctx, filter_spec);
    return ret;
}

static int encode_write_frame(struct ConversionData *cData, AVFrame *filt_frame, int *got_frame) {
    int ret;
    int got_frame_local;
    AVPacket enc_pkt;
    int (*enc_func)(AVCodecContext *, AVPacket *, const AVFrame *, int *);
    enc_func = avcodec_encode_video2;

    if (!got_frame)
        got_frame = &got_frame_local;

    /* encode filtered frame */
    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    av_init_packet(&enc_pkt);
    ret = enc_func(cData->stream_ctx[0].enc_ctx, &enc_pkt,
            filt_frame, got_frame);
    av_frame_free(&filt_frame);
    if (ret < 0)
        return ret;
    if (!(*got_frame))
        return 0;

    /* prepare packet for muxing */
    enc_pkt.stream_index = 0;
    av_packet_rescale_ts(&enc_pkt,
                         cData->stream_ctx[0].enc_ctx->time_base,
                         cData->ofmt_ctx->streams[0]->time_base);

    /* mux encoded frame */
    ret = av_interleaved_write_frame(cData->ofmt_ctx, &enc_pkt);
    return ret;
}

static int filter_encode_write_frame(struct ConversionData *cData, AVFrame *frame) {
    int ret;
    AVFrame *filt_frame;

    /* push the decoded frame into the filtergraph */
    ret = av_buffersrc_add_frame_flags(cData->filter_ctx[0].buffersrc_ctx,
            frame, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        return ret;
    }

    /* pull filtered frames from the filtergraph */
    while (1) {
        filt_frame = av_frame_alloc();
        if (!filt_frame) {
            ret = AVERROR(ENOMEM);
            break;
        }
        av_log(NULL, AV_LOG_DEBUG, "Pulling filtered frame from filters\n");
        ret = av_buffersink_get_frame(cData->filter_ctx[0].buffersink_ctx,
                filt_frame);
        if (ret < 0) {
            /* if no more frames for output - returns AVERROR(EAGAIN)
             * if flushed and no more frames for output - returns AVERROR_EOF
             * rewrite retcode to 0 to show it as normal procedure completion
             */
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                ret = 0;
            av_frame_free(&filt_frame);
            break;
        }

        filt_frame->pict_type = AV_PICTURE_TYPE_NONE;
        ret = encode_write_frame(cData, filt_frame, NULL);
        if (ret < 0)
            break;
    }

    return ret;
}

static int flush_encoder(struct ConversionData *cData, unsigned int stream_index) {
    int ret;
    int got_frame;

    if (!(cData->stream_ctx[0].enc_ctx->codec->capabilities &
            AV_CODEC_CAP_DELAY))
        return 0;

    while (1) {
        av_log(NULL, AV_LOG_DEBUG, "Flushing stream #%u encoder\n", stream_index);
        ret = encode_write_frame(cData, NULL, &got_frame);
        if (ret < 0)
            break;
        if (!got_frame)
            return 0;
    }
    return ret;
}

void android_log(void *ptr, int level, const char* fmt, va_list vl) {
    int and_level = ANDROID_LOG_DEBUG;
    switch (level) {
        case AV_LOG_INFO:
            and_level = ANDROID_LOG_INFO;
            break;
        case AV_LOG_DEBUG:
            and_level = ANDROID_LOG_DEBUG;
            break;
        case AV_LOG_ERROR:
            and_level = ANDROID_LOG_ERROR;
            break;
        case AV_LOG_FATAL:
            and_level = ANDROID_LOG_FATAL;
            break;
        case AV_LOG_TRACE:
            and_level = ANDROID_LOG_VERBOSE;
            break;
        default:
            and_level = ANDROID_LOG_DEBUG;
            break;
    }
    __android_log_vprint(and_level, "react-native-ffmpeg", fmt, vl);
}

void encodeVideoOnly(const char *infile,
                     const char *outfile,
                     const char *out_codec_str,
                     const char *filter_spec,
                     void *vp,
                     progress prgs,
                     done dn,
                     error er) {
    int ret;
    AVPacket packet = {.data = NULL, .size = 0};
    AVFrame *frame = NULL;
    enum AVMediaType type;
    int got_frame;
    int (*dec_func)(AVCodecContext *, AVFrame *, int *, const AVPacket *);
    struct ConversionData cData = { 0 };

    av_register_all();
    avfilter_register_all();
    av_log_set_callback(android_log);

    if ((ret = open_input_file(&cData, infile)) < 0) {
        av_log(NULL, AV_LOG_ERROR, ">>>>>>>>> Error in opening input file\n");
        goto end;
    }
    if ((ret = open_output_file(&cData, outfile, out_codec_str)) < 0) {
        av_log(NULL, AV_LOG_ERROR, ">>>>>>>>> Error in opening output file\n");
        goto end;
    }
    if ((ret = init_filters(&cData, filter_spec)) < 0)
        goto end;

    /* read all packets */
    while (1) {
        if ((ret = av_read_frame(cData.ifmt_ctx, &packet)) < 0)
            break;
        if (packet.stream_index != cData.video_idx)
            continue;
        type = cData.ifmt_ctx->streams[cData.video_idx]->codecpar->codec_type;
        av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n",
               cData.video_idx);
        if (type != AVMEDIA_TYPE_VIDEO)
            continue;
        if (cData.filter_ctx[0].filter_graph) {
            av_log(NULL, AV_LOG_DEBUG, "Going to re-encode & filter the frame\n");
            frame = av_frame_alloc();
            if (!frame) {
                ret = AVERROR(ENOMEM);
                break;
            }
            av_packet_rescale_ts(&packet,
                                 cData.ifmt_ctx->streams[cData.video_idx]->time_base,
                                 cData.stream_ctx[0].dec_ctx->time_base);
            dec_func = avcodec_decode_video2;
            ret = dec_func(cData.stream_ctx[0].dec_ctx, frame,
                    &got_frame, &packet);
            if (ret < 0) {
                av_frame_free(&frame);
                av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
                break;
            }

            if (got_frame) {
                cData.nb_frames_written++;
                frame->pts = frame->best_effort_timestamp;
                ret = filter_encode_write_frame(&cData, frame);
                av_frame_free(&frame);
                if (ret < 0)
                    goto end;
                int p = ((double_t)cData.nb_frames_written / cData.nb_frames) * 100;
                av_log(NULL, AV_LOG_INFO, "nb_frames: %lld, nb_frames_written: %lld, percentage: %d\n", cData.nb_frames, cData.nb_frames_written, p);
                prgs(vp, p);
            } else {
                av_frame_free(&frame);
            }
        } else {
            /* remux this frame without reencoding */
            av_packet_rescale_ts(&packet,
                                 cData.ifmt_ctx->streams[cData.video_idx]->time_base,
                                 cData.ofmt_ctx->streams[0]->time_base);

            ret = av_interleaved_write_frame(cData.ofmt_ctx, &packet);
            if (ret < 0)
                goto end;
        }
        av_packet_unref(&packet);
    }

    /* flush filters and encoders */
    if (cData.filter_ctx[0].filter_graph) {
        ret = filter_encode_write_frame(&cData, NULL);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Flushing filter failed\n");
            goto end;
        }

        /* flush encoder */
        ret = flush_encoder(&cData, 0);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Flushing encoder failed\n");
            goto end;
        }
    }

    av_write_trailer(cData.ofmt_ctx);
end:
    av_packet_unref(&packet);
    av_frame_free(&frame);

    avcodec_free_context(&cData.stream_ctx[0].dec_ctx);
    avcodec_free_context(&cData.stream_ctx[0].enc_ctx);

    if (cData.filter_ctx && cData.filter_ctx[0 ].filter_graph)
        avfilter_graph_free(&cData.filter_ctx[0].filter_graph);

    av_free(cData.filter_ctx);
    av_free(cData.stream_ctx);
    avformat_close_input(&cData.ifmt_ctx);
    if (cData.ofmt_ctx && !(cData.ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&cData.ofmt_ctx->pb);
    avformat_free_context(cData.ofmt_ctx);

    if (ret < 0)
        __android_log_print(ANDROID_LOG_INFO, "react-native-ffmpeg",  "Error occurred: %s\n", av_err2str(ret));

    ret ? er(vp, av_err2str(ret)) : dn(vp);

}