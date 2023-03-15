#ifndef H264DECODER_H
#define H264DECODER_H

#include <QObject>
#include <QImage>

extern "C" {
//    #include "libavcodec/packet.h"
//    #include "libavcodec/avcodec.h"
//    #include <libavformat/avformat.h>
//    #include <libswscale/swscale.h>
//    #include <libavutil/imgutils.h>
//    #include <libavutil/pixdesc.h>
//    #include <libavutil/hwcontext.h>
//    #include <libavutil/opt.h>
//    #include <libavutil/avassert.h>
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavdevice/avdevice.h"
    #include "libavutil/frame.h"
    #include "libswscale/swscale.h"
    #include "libavutil/opt.h"
    #include "libavutil/error.h"
    #include "libavutil/parseutils.h"
    #include "libavutil/samplefmt.h"
    #include "libavutil/fifo.h"
    #include "libavutil/intreadwrite.h"
    #include "libavutil/dict.h"
    #include "libavutil/mathematics.h"
    #include "libavutil/pixdesc.h"
    #include "libavutil/avstring.h"
    #include "libavutil/imgutils.h"
    #include "libswresample/swresample.h"
    #include "libavfilter/avfilter.h"
    // #include "libavfilter/avfiltergraph.h"
    #include "libavfilter/buffersink.h"
    #include "libavfilter/buffersrc.h"
}

enum H264CODEREN {
    H264_HARDWARE_DECODER = 0,
    H264_SOFTWARE_DECODER = 1
};

struct AVFrameDeleter
{
    static inline void cleanup(void *pointer) {
        if (pointer)
            av_frame_free((AVFrame**)&pointer);
    }
};

struct AVPacketDeleter
{
    static inline void cleanup(void *pointer) {
        if (pointer)
            av_packet_unref((AVPacket*)pointer);
    }
};

struct AVFormatContextDeleter
{
    static inline void cleanup(void *pointer) {
        if (pointer)
            avformat_close_input((AVFormatContext**)&pointer);
    }
};

class H264Decoder : public QObject
{
    Q_OBJECT
public:
    explicit H264Decoder(QObject *parent = nullptr);
    ~H264Decoder();

    virtual bool init(AVCodecParameters* codecParameters = nullptr) = 0;
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual void flush() = 0;
    virtual int decode(quint64 timestamp, AVPacket *packet) = 0;

    const char* get_pix_fmt_name(AVPixelFormat pixformat);
signals:
    void decodedImageData(quint64 timestamp, QImage frame);
public slots:
    virtual void slot_input_frame_data(const uint8_t *data_ptr, int data_len) = 0;
protected:
    bool m_is_first_;

    bool convert_AVFrame_to_RGB32(AVFrame *sw_frame_ptr);
//    int initHWContext(const enum AVHWDeviceType m_type);
//    void sendFrame(VideoFrame * frame);

//    virtual VideoFrame* createHWVideoFrame(const AVFrame * frame) = 0;
//    VideoFrame* createSWVideoFrame(const AVFrame *frame);

    const AVCodec *m_codec_ptr_;
    AVCodecContext *m_avcodec_ctx_ptr_;
    AVCodecParserContext *m_parser_ctx_ptr_;
    SwsContext *m_sws_context_ptr_;

    QScopedPointer<AVPacket, AVPacketDeleter> m_av_packet_ptr_;
    QScopedPointer<AVFrame, AVFrameDeleter> m_avframe_decoder_ptr_;
    QScopedPointer<AVFrame, AVFrameDeleter> m_avframe_picture_ptr_;
//    AVHWDeviceType m_type;
//    AVBufferRef *m_hwDeviceCtx;

//    AVCodec *m_decoder;
    uint8_t *m_picbuffer_ptr_;
    int m_video_width_;
    int m_video_height_;
private:

};


#endif // H264DECODER_H
