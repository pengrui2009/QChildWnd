#include "h264swdecoder.h"

#include <QTime>
#include <QMutexLocker>
#include <QDebug>

H264SWDecoder::H264SWDecoder(QObject *parent)
    : H264Decoder(parent)
{
    m_is_run_ = false;
    m_is_first_i_frame_ = true;
}

H264SWDecoder::~H264SWDecoder()
{    
    flush();
    close();
    m_is_run_ = false;
}

bool H264SWDecoder::init(AVCodecParameters* codecParameters)
{    
    m_codec_ptr_ = (AVCodec*)avcodec_find_decoder(AVCodecID::AV_CODEC_ID_H264);
    if (!m_codec_ptr_)
    {
        return false;
    }

    m_avcodec_ctx_ptr_ = avcodec_alloc_context3(m_codec_ptr_);
    if (!m_avcodec_ctx_ptr_)
    {
        return false;
    }

    m_parser_ctx_ptr_ = av_parser_init(AVCodecID::AV_CODEC_ID_H264);
    if (!m_parser_ctx_ptr_)
    {
        return false;
    }

    m_is_run_ = true;
//    std::thread t(std::bind(&H264SWDecoder::handleFrameData, this));
//    t.detach();

    return true;
}

void H264SWDecoder::handleFrameData()
{
    while(m_is_run_)
    {
        QByteArray frame_data;
        if (frame_queue_.WaitDequeue(frame_data, 100))
        {
            int ret = 0;
            quint64 timestamp = 0;
            const int nBufferSize = frame_data.size() + FF_BUG_NO_PADDING;
            QByteArray buffer;
            buffer.fill(0, nBufferSize);
            memcpy(buffer.data(), frame_data.constData(), frame_data.size());

            timestamp = (static_cast<quint64>(frame_data[ret-8]) << 56) \
                      + (static_cast<quint64>(frame_data[ret-7]) << 48) \
                      + (static_cast<quint64>(frame_data[ret-6]) << 40) \
                      + (static_cast<quint64>(frame_data[ret-5]) << 32) \
                      + (static_cast<quint64>(frame_data[ret-4]) << 24) \
                      + (static_cast<quint64>(frame_data[ret-3]) << 16) \
                      + (static_cast<quint64>(frame_data[ret-2]) <<  8) \
                      + (static_cast<quint64>(frame_data[ret-1]) <<  0);
            ret = av_parser_parse2(
                        m_parser_ctx_ptr_,
                        m_avcodec_ctx_ptr_,
                        &m_av_packet_ptr_->data,
                        &m_av_packet_ptr_->size,
                        (const uint8_t *)buffer.constData(),
                        frame_data.size(),
                        AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                fprintf(stderr, "Error while parsing\n");
                return;
            }

            if (0 == frame_data.size())
            {
                continue;
            }

            switch (m_parser_ctx_ptr_->pict_type)
            {
                case AV_PICTURE_TYPE_I:
                    break;
                case AV_PICTURE_TYPE_P:
                    break;
                case AV_PICTURE_TYPE_B:
                    break;
                default:
                    break;
            }

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
}

bool H264SWDecoder::open()
{
    if (avcodec_open2(m_avcodec_ctx_ptr_, m_codec_ptr_, nullptr) < 0)
    {
        return false;
    }

    return true;
}

void H264SWDecoder::close()
{
    m_avcodec_ctx_ptr_ = nullptr;
}

void H264SWDecoder::flush()
{
    if (m_avcodec_ctx_ptr_) {
//        QScopedPointer<AVPacket, AVPacketDeleter> packet(av_packet_alloc());
//        decode(m_av_packet_ptr_.data());
    }
}

/**
* 检测是否是关键帧
 * @param data_ptr 编码后的h264视频数据
 * @param data_len 编码后的h264视频数据长度
 */
bool H264SWDecoder::isKeyFrame(const uint8_t *data_ptr, int data_len)
{
    if (data_len < 5)
    {
        return false;
    }
    //00 00 00 01
    if ((data_ptr[0] == 0)&&(data_ptr[1] == 0)&&(data_ptr[2] == 0)&&(data_ptr[3] == 1))
    {
        int nalType = data_ptr[4] & 0x1f;
        if (nalType == 0x07 || nalType == 0x05 || nalType == 0x08)
        {
            return true;
        }
    }
    //00 00 01
    if ((data_ptr[0] == 0)&&(data_ptr[1] == 0)&&(data_ptr[2] == 1))
    {
        int nalType = data_ptr[3] & 0x1f;
        if (nalType == 0x07 || nalType == 0x05 || nalType == 0x08)
        {
            return true;
        }
    }
    return false;
}

void H264SWDecoder::slot_input_frame_data(const uint8_t *data_ptr, int data_len)
{
#if 0
    QByteArray data_buffer;

    data_buffer.fill(0, data_len);
    memcpy(data_buffer.data(), data_ptr, data_len);
    frame_queue_.Enqueue(data_buffer);
#endif
    int ret = 0;

    if (m_is_first_i_frame_ == true)
    {
        if (!isKeyFrame(data_ptr, data_len))
        {
            return;
        }
        m_is_first_i_frame_ = false;
    }

    while (data_len > 0)
    {
        quint64 timestamp = 0;
        ret = av_parser_parse2(m_parser_ctx_ptr_, m_avcodec_ctx_ptr_, &m_av_packet_ptr_->data, &m_av_packet_ptr_->size,
                               data_ptr, data_len, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            fprintf(stderr, "Error while parsing\n");
            return;
        }

        timestamp = (static_cast<quint64>(data_ptr[ret-8]) << 56) \
                  + (static_cast<quint64>(data_ptr[ret-7]) << 48) \
                  + (static_cast<quint64>(data_ptr[ret-6]) << 40) \
                  + (static_cast<quint64>(data_ptr[ret-5]) << 32) \
                  + (static_cast<quint64>(data_ptr[ret-4]) << 24) \
                  + (static_cast<quint64>(data_ptr[ret-3]) << 16) \
                  + (static_cast<quint64>(data_ptr[ret-2]) <<  8) \
                  + (static_cast<quint64>(data_ptr[ret-1]) <<  0);

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

int H264SWDecoder::decode(quint64 timestamp, AVPacket *packet)
{
    int ret = 0;
//    QMutexLocker _(&m_mutex_);
    QScopedPointer<AVFrame, AVFrameDeleter> decoded_frame;

    if (packet->size == 0)
    {
        return -1;
    }
    decoded_frame.reset(av_frame_alloc());
    ret = avcodec_send_packet(m_avcodec_ctx_ptr_, m_av_packet_ptr_.data());
    if (ret < 0)
    {
        qWarning() << "Error during decoding avcodec_send_packet ret:" << ret;
        return ret;
    }

    ret = avcodec_receive_frame(m_avcodec_ctx_ptr_, decoded_frame.data());
    if (ret < 0)
    {
        qWarning() << "Error during decoding avcodec_receive_frame ret:" << ret;
        return -1;
    }

    if (!convert_AVFrame_to_RGB32(decoded_frame.data()))
    {
        qWarning() << "Error while decoding, convert_AVFrame_to_RGB32!";
        return -1;
    }

    QImage image(m_avframe_picture_ptr_->data[0], m_video_width_, m_video_height_, QImage::Format_ARGB32);
    emit decodedImageData(timestamp, image);

    return 0;
}

