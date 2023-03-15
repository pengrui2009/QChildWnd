/**
 * @file h264swdecoder.h
 * @author your name (you@domain.com)
 * @brief ffmpeg H264 software decoder
 * @version 0.1
 * @date 2022-08-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef H264SWDECODER_H
#define H264SWDECODER_H

#include "h264decoder.h"
#include "thread_safe_queue.h"

#include <QObject>
#include <QByteArray>
#include <QMutex>
#include <QByteArray>
#include <QThread>
#include <QImage>
#include <QTimer>

extern "C"{
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

class H264SWDecoder : public H264Decoder
{
    Q_OBJECT
public:
    explicit H264SWDecoder(QObject *parent = nullptr);
    ~H264SWDecoder() override;

    bool init(AVCodecParameters* codecParameters = nullptr) override;
    bool open() override;
    void close() override;
    void flush() override;

    int decode(quint64 timestamp, AVPacket *packet) override;
private:
    QMutex m_mutex_;
    bool m_is_run_;
    bool m_is_first_i_frame_;

    ThreadSafeQueue<QByteArray> frame_queue_;

    bool isKeyFrame(const uint8_t *data_ptr, int data_len);
    void handleFrameData();
signals:
//    void imageData(QImage image);

public slots:
    void slot_input_frame_data(const uint8_t *data_ptr, int data_len) override;
};

#endif // H264DSWECODER_H
