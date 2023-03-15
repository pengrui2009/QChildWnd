/**
 * @file h264hwdecoder.cpp
 * @author your name (you@domain.com)
 * @brief H264 Hardware decoder with ffmpeg
 * @version 0.1
 * @date 2022-08-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef H264HWDECODER_H
#define H264HWDECODER_H

#include "h264decoder.h"

extern "C" {
    #include "libavcodec/packet.h"
    #include "libavcodec/avcodec.h"
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/pixdesc.h>
    #include <libavutil/hwcontext.h>
    #include <libavutil/opt.h>
    #include <libavutil/avassert.h>
}

class H264HWDecoder: public H264Decoder
{
    Q_OBJECT
public:
    static enum AVPixelFormat getFormat(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);

    H264HWDecoder(QObject * parent = nullptr);
    ~H264HWDecoder();

    virtual bool init(AVCodecParameters* codecParameters = nullptr);
    virtual bool open();
    virtual void close();
    virtual void flush();
    virtual int decode(quint64 timestamp, AVPacket *packet);
Q_SIGNALS:
//    void frameDecoded(VideoFramePtr frame);
signals:
//    void decodedImageData(QImage image);

public slots:
    void slot_input_frame_data(const uint8_t *data_ptr, int data_len);
protected:

private:
//    virtual VideoFrame* createHWVideoFrame(const AVFrame * frame) = 0;
//    VideoFrame* createSWVideoFrame(const AVFrame *frame);
    QString m_device_name_;
    static AVPixelFormat m_hw_pix_fmt_;

    AVHWDeviceType m_type_;
    AVBufferRef *avbuffer_ptr_;
    // H264 parser

    void get_sw_support_pix_fmt();
    int sendFrame(quint64 timestamp, AVFrame * frame);
    int initHWContext(const enum AVHWDeviceType m_type);
    int memory_Copy_from_Cuda(AVFrame *hw_frame_ptr, AVFrame *sw_frame_ptr);
};

#endif /* H264HWDECODER_H */
