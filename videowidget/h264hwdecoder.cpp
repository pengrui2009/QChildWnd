#include "h264hwdecoder.h"

#include <QTime>
#include <QDebug>

AVPixelFormat H264HWDecoder::m_hw_pix_fmt_ = AV_PIX_FMT_NONE;

H264HWDecoder::H264HWDecoder(QObject * parent)
    : H264Decoder(parent),
      m_type_(AV_HWDEVICE_TYPE_NONE),
      avbuffer_ptr_(nullptr)
{
    avdevice_register_all();

    m_device_name_ = "cuda";
    m_hw_pix_fmt_ = AV_PIX_FMT_CUDA;
}

H264HWDecoder::~H264HWDecoder()
{
    flush();
    close();
}

void H264HWDecoder::slot_input_frame_data(const uint8_t *data_ptr, int data_len)
{
    int ret = 0;
    quint64 timestamp = 0;

    if (data_len < 4)
    {
        return;
    }

    timestamp = (static_cast<quint64>(data_ptr[data_len-8]) << 56) \
              + (static_cast<quint64>(data_ptr[data_len-7]) << 48) \
              + (static_cast<quint64>(data_ptr[data_len-6]) << 40) \
              + (static_cast<quint64>(data_ptr[data_len-5]) << 32) \
              + (static_cast<quint64>(data_ptr[data_len-4]) << 24) \
              + (static_cast<quint64>(data_ptr[data_len-3]) << 16) \
              + (static_cast<quint64>(data_ptr[data_len-2]) <<  8) \
              + (static_cast<quint64>(data_ptr[data_len-1]) <<  0);
    data_len -=8;
    while (data_len > 0)
    {

        ret = av_parser_parse2(m_parser_ctx_ptr_, m_avcodec_ctx_ptr_, &m_av_packet_ptr_->data, &m_av_packet_ptr_->size,
                               data_ptr, data_len, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            fprintf(stderr, "Error while parsing\n");
            return;
        }

        data_ptr += ret;
        data_len -= ret;

        if (m_av_packet_ptr_->size)
        {

            ret = decode(timestamp, m_av_packet_ptr_.data());
            if (ret <0)
            {
                return;
            }
        }
    }

}

bool H264HWDecoder::init(AVCodecParameters* codecParameters)
{
#if 0
    m_codec_ptr_ = avcodec_find_decoder_by_name("h264_cuvid");//avcodec_find_decoder(codecParameters->codec_id);
    m_codec_ptr_ = avcodec_find_decoder(AV_CODEC_ID_H264/*codecpar->codec_id*/);
    if (!m_codec_ptr_)
    {
        return false;
    }

    m_parser_ctx_ptr_ = av_parser_init(m_codec_ptr_->id);
    if (!m_parser_ctx_ptr_) {
        fprintf(stderr, "parser not found\n");
        return false;
    }

//    m_type_ = av_hwdevice_find_type_by_name(m_device_name_.toStdString().c_str());
//    if (m_type_ == AV_HWDEVICE_TYPE_NONE) {
//        qWarning() << "Device type" << m_device_name_ << "is not supported.";
//        qWarning() << "Available device types:";
//        while((m_type_ = av_hwdevice_iterate_types(m_type_)) != AV_HWDEVICE_TYPE_NONE)
//            qWarning() << QString::fromStdString(av_hwdevice_get_type_name(m_type_));
//        return false;
//    }
//    avcodec_find_decoder_by_name("h264_cuvid");

    if (!(m_avcodec_ctx_ptr_ = avcodec_alloc_context3(m_codec_ptr_)))
    {
        return false;
    }


    m_avcodec_ctx_ptr_->get_format  = getFormat;
//    m_avcodec_ctx_ptr_->refcounted_frames = 1;

    if (initHWContext(m_type_) < 0)
        return false;
#endif
    int result = 0;

//     codec_avframe_ptr_ = av_frame_alloc();
//     if (codec_avframe_ptr_ == nullptr)
//     {
//         LOG_ERROR("decode avframe av_frame_alloc failed!");
//         return -1;
//     }

//     bgr_frame_ptr_ = av_frame_alloc();
//     if (bgr_frame_ptr_ == nullptr)
//     {
//         LOG_ERROR("bgr avframe av_frame_alloc failed!");
//         return -1;
//     }

//     avpacket_ptr_ = av_packet_alloc();
//     if (avpacket_ptr_ == nullptr)
//     {
//         LOG_ERROR("avpacket av_packet_alloc failed!");
//         return -1;
//     }

     av_init_packet(m_av_packet_ptr_.get());

//     format_ctx_ptr_ = avformat_alloc_context();
//     if (format_ctx_ptr_ == nullptr)
//     {
//         qDebug() << "avformat_alloc_context failed!";
//         return -1;
//     }

     if (1) {
         // Nvidia hardware codec, AV_CODEC_ID_H264 cuvid.c
         m_codec_ptr_ = avcodec_find_decoder_by_name("h264_cuvid");
//         m_codec_ptr_ = avcodec_find_decoder(AV_CODEC_ID_H264);
         qDebug() << "H264 Decoder Method:Nvidia";
     } else {
         // software codec
         m_codec_ptr_ = avcodec_find_decoder(AV_CODEC_ID_H264/*codecpar->codec_id*/);
         qDebug() << "H264 Decoder Method:Software";
     }
     if (m_codec_ptr_ == nullptr)
     {
         qDebug() << "avcodec_find_decoder_by_name h264_cuvid failed.";
         return false;
     }

     m_parser_ctx_ptr_ = av_parser_init(m_codec_ptr_->id);
     if (!m_parser_ctx_ptr_)
     {
         qDebug() << "parser not found.";
         return false;
     }

     // enum AVHWDeviceType device_type = av_hwdevice_find_type_by_name("cuda");
     // if (device_type == AV_HWDEVICE_TYPE_NONE)
     // {
     //     // qWarning() << "Device type" << m_device_name_ << "is not supported.";
     //     // qWarning() << "Available device types:";
     //     while((device_type = av_hwdevice_iterate_types(device_type)) != AV_HWDEVICE_TYPE_NONE)
     //     {
     //         // qWarning() << QString::fromStdString(av_hwdevice_get_type_name(m_type_));
     //     }

     //     return false;
     // }

     m_avcodec_ctx_ptr_ = avcodec_alloc_context3(m_codec_ptr_);
     if (m_avcodec_ctx_ptr_ == nullptr)
     {
         av_log(NULL, AV_LOG_FATAL, "avcodec_alloc_context3 failed\n");
         return false;
     }

     // avcodec_ctx_ptr_->get_format  = getFormat;
     // avcodec_ctx_ptr_->refcounted_frames = 1;

     enum AVHWDeviceType type = AV_HWDEVICE_TYPE_CUDA;
     result= av_hwdevice_ctx_create(&avbuffer_ptr_, type, nullptr, nullptr, 0);
     if (result < 0)
     {
         qDebug() << "Failed to create specified HW device.";
         return false;
     }
     m_avcodec_ctx_ptr_->hw_device_ctx = av_buffer_ref(avbuffer_ptr_);

     result = avcodec_open2(m_avcodec_ctx_ptr_, m_codec_ptr_, NULL);
     if (result < 0)
     {
         qDebug() << "avcodec_open2 failed!";
         return false;
     }

    return true;
}

int H264HWDecoder::initHWContext(const enum AVHWDeviceType type)
{
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&avbuffer_ptr_, AV_HWDEVICE_TYPE_CUDA,
                                      nullptr, nullptr, 0)) < 0) {
        qWarning() << "Failed to create specified HW device.";
        return err;
    }
    m_avcodec_ctx_ptr_->hw_device_ctx = av_buffer_ref(avbuffer_ptr_);

    return err;
}

void H264HWDecoder::get_sw_support_pix_fmt()
{
    struct AVBufferRef *hwDeviceCtx = nullptr;
    enum AVHWDeviceType type = AV_HWDEVICE_TYPE_CUDA;

    int err = av_hwdevice_ctx_create(&hwDeviceCtx, type, nullptr, nullptr, 0);
    if (err < 0) {
        // Err
    }
    // fprintf(stdout, "av_hwdevice_ctx_create\n");

    AVHWFramesConstraints* hw_frames_const = av_hwdevice_get_hwframe_constraints(hwDeviceCtx, nullptr);
    if (hw_frames_const == nullptr)
    {
        // Err
    }

    // Check if we can convert the pixel format to a readable format.
    enum AVPixelFormat found = AV_PIX_FMT_NONE;
    for (enum AVPixelFormat* p = hw_frames_const->valid_sw_formats;
        *p != AV_PIX_FMT_NONE; p++)
    {
        // Check if we can convert to the desired format.
        if (sws_isSupportedInput(*p))
        {
            // Ok! This format can be used with swscale!
            found = *p;
            const char* found_format = av_get_pix_fmt_name((enum AVPixelFormat)found);
            fprintf(stdout, "support foramt:%s\n", found_format);
            //break;
        }
    }

    // Don't forget to free the constraint object.
    av_hwframe_constraints_free(&hw_frames_const);

    // Attach your hw device to your codec context if you want to use hw decoding.
    // Check AVCodecContext.hw_device_ctx!
}

bool H264HWDecoder::open()
{
//    if (m_codec_ptr_ && m_avcodec_ctx_ptr_
//            && avcodec_open2(m_avcodec_ctx_ptr_, m_codec_ptr_, nullptr) == 0) {
//        return true;
//    }

//    qWarning() << "Failed to open codec";
//    return false;
    return true;
}


void H264HWDecoder::close()
{
//    avcodec_free_context(&m_avcodec_ctx_ptr_);
    av_buffer_unref(&avbuffer_ptr_);

    m_avcodec_ctx_ptr_ = nullptr;
    avbuffer_ptr_ = nullptr;
}

void H264HWDecoder::flush()
{
    if (m_avcodec_ctx_ptr_) {
//        QScopedPointer<AVPacket, AVPacketDeleter> packet(av_packet_alloc());
//        decode(m_av_packet_ptr_.data());
    }
}

enum AVPixelFormat H264HWDecoder::getFormat(AVCodecContext *ctx,
                                        const enum AVPixelFormat *pix_fmts)
{
    Q_UNUSED(ctx)
    const enum AVPixelFormat *p;

    //nadaless: Try to decode baseline profiles with HW (Android WebRTC Streams)
    if (ctx->profile == FF_PROFILE_H264_BASELINE)
        ctx->hwaccel_flags |= AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == m_hw_pix_fmt_)
            return *p;
    }

    qWarning() << "Failed to get HW surface format.";
    return AV_PIX_FMT_NONE;
}

int H264HWDecoder::decode(quint64 timestamp, AVPacket *packet)
{
    int ret = 0;
    QScopedPointer<AVFrame, AVFrameDeleter> decoded_frame;

    ret = avcodec_send_packet(m_avcodec_ctx_ptr_, packet);
    if (ret < 0)
    {
        qWarning() << "Error during decoding avcodec_send_packet ret:" << ret;
        return ret;
    }

    while (ret >= 0)
    {
        decoded_frame.reset(av_frame_alloc());
        if (!decoded_frame.data()) {
            qWarning() << "Can not alloc frame to get decoded data!";
            return AVERROR(ENOMEM);
        }

        ret = avcodec_receive_frame(m_avcodec_ctx_ptr_, decoded_frame.data());
        if (ret == AVERROR(EAGAIN)) {
            return 0;
        } else if (ret < 0) {
            switch(ret) {
                case AVERROR_EOF:                    
                    sendFrame(timestamp, decoded_frame.data());
                    break;
                default:
                    qWarning() << "Error while decoding, code:" << ret;
                    break;
            }
            return ret;
        }

        sendFrame(timestamp, decoded_frame.data());
    }
    return 0;
}

int H264HWDecoder::memory_Copy_from_Cuda(AVFrame *hw_frame_ptr, AVFrame *sw_frame_ptr)
{
    int ret = 0;

    if (hw_frame_ptr->format == m_hw_pix_fmt_) {
        // dest sw frame format
        sw_frame_ptr->format = AV_PIX_FMT_NV12;

        /* retrieve data from GPU to CPU */
        if ((ret = av_hwframe_transfer_data(sw_frame_ptr, hw_frame_ptr, 0)) < 0) {
            fprintf(stderr, "Error transferring the data to system memory\n");
            // goto fail;
            return ret;
        }

    } else {
        sw_frame_ptr = hw_frame_ptr;
    }

    return ret;
}

//VideoFrame* H264HWDecoder::createSWVideoFrame(const AVFrame *frame)
//{
//    Q_UNUSED(frame)
//    Q_UNIMPLEMENTED();
//    return new VideoFrame();
//}

int H264HWDecoder::sendFrame(quint64 timestamp, AVFrame *decoded_frame)
{
    int ret = 0;
//    QScopedPointer<AVFrame, AVFrameDeleter> software_frame;
    // VideoFramePtr sharedFrame(frame);
    // Q_EMIT frameDecoded(sharedFrame);
//    software_frame.reset(av_frame_alloc());

    ret = memory_Copy_from_Cuda(decoded_frame, m_avframe_decoder_ptr_.data());
    if (ret < 0)
    {
        qWarning() << "Error while decoding, Memory_Copy_from_Cuda failed ret:" << ret;
        return -1;
    }

    if (!convert_AVFrame_to_RGB32(m_avframe_decoder_ptr_.data()))
    {
        qWarning() << "Error while decoding, Convert_NV12_to_RGB32!";
        return -1;
    }

    QImage image(m_avframe_picture_ptr_->data[0], m_video_width_, m_video_height_, QImage::Format_ARGB32);
    emit decodedImageData(timestamp, image);

    return ret;
}
