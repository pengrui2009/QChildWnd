#include "h264decoder.h"

#include <QDebug>

H264Decoder::H264Decoder(QObject *parent) : QObject(parent)
  , m_is_first_(true)
  , m_codec_ptr_(nullptr)
  , m_avcodec_ctx_ptr_(nullptr)
  , m_parser_ctx_ptr_(nullptr)
  , m_sws_context_ptr_(nullptr)
  , m_av_packet_ptr_(nullptr)
  , m_avframe_decoder_ptr_(nullptr)
  , m_avframe_picture_ptr_(nullptr)
  , m_picbuffer_ptr_(nullptr)
{
    m_av_packet_ptr_.reset(av_packet_alloc());
    av_init_packet(m_av_packet_ptr_.data());

    m_avframe_decoder_ptr_.reset(av_frame_alloc());
    m_avframe_picture_ptr_.reset(av_frame_alloc());

}

H264Decoder::~H264Decoder()
{
    av_free(m_picbuffer_ptr_);
//    av_packet_free(&m_av_packet_ptr_);
//    av_frame_free(&m_avframe_decoder_ptr_);
//    av_frame_free(&m_avframe_picture_ptr_);
    avcodec_free_context(&m_avcodec_ctx_ptr_);
}

const char* get_pix_fmt_name(AVPixelFormat pixformat)
{
    return av_get_pix_fmt_name(pixformat);
}

bool H264Decoder::convert_AVFrame_to_RGB32(AVFrame *sw_frame_ptr)
{
    if (m_is_first_)
    {
        unsigned int bytes = 0;
        AVPixelFormat srcFmt = AV_PIX_FMT_NV12;
        AVPixelFormat dstFmt = AV_PIX_FMT_RGB32;

        m_video_width_ = m_avcodec_ctx_ptr_->width;
        m_video_height_ = m_avcodec_ctx_ptr_->height;

        srcFmt = static_cast<AVPixelFormat>(sw_frame_ptr->format);

        bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, m_video_width_, m_video_height_, 1);
        m_picbuffer_ptr_ = reinterpret_cast<uint8_t *>(av_malloc(bytes * sizeof(uint8_t)));

        av_image_fill_arrays(m_avframe_picture_ptr_->data, m_avframe_picture_ptr_->linesize,
                             m_picbuffer_ptr_, dstFmt, m_video_width_, m_video_height_, 1);

        m_sws_context_ptr_ = sws_getContext(m_video_width_, m_video_height_, srcFmt, m_video_width_,
                                            m_video_height_, dstFmt, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

        m_is_first_ = false;
    }

    sws_scale(m_sws_context_ptr_, sw_frame_ptr->data, sw_frame_ptr->linesize, 0, m_video_height_, m_avframe_picture_ptr_->data, m_avframe_picture_ptr_->linesize);

    return true;
}
