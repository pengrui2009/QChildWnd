#include "h264decoder.h"
#include "h264fileprocessor.h"

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

#include <QThread>
#include <QDebug>
#include <QDateTime>

H264FileProcessor::H264FileProcessor(QObject *parent) : QObject(parent)
{
    m_file_ptr_ = nullptr;
    m_run_flag_ = true;
}

H264FileProcessor::~H264FileProcessor()
{
    m_run_flag_ = false;

    if (m_file_ptr_)
    {
        m_file_ptr_->close();

        delete  m_file_ptr_;
    }
}

bool H264FileProcessor::init()
{
    m_file_ptr_ = new QFile();

    return true;
}

bool H264FileProcessor::open(const QString &filename)
{
    bool ret = false;
    m_file_ptr_->setFileName(filename);

    ret = m_file_ptr_->open(QFile::ReadOnly);
    if (!ret)
    {
        return false;
    }

    return true;
}

void H264FileProcessor::processFile()
{
    bool is_idr_frame = true;
    bool flag = true;

    QByteArray data_buffer;
    QByteArray read_data;
    QByteArray sync_bytes1;
    QByteArray sync_bytes2;
    sync_bytes1[0] = 0x00;
    sync_bytes1[1] = 0x00;
    sync_bytes1[2] = 0x00;
    sync_bytes1[3] = 0x01;
    sync_bytes1[4] = 0x61;

    sync_bytes2[0] = 0x00;
    sync_bytes2[1] = 0x00;
    sync_bytes2[2] = 0x00;
    sync_bytes2[3] = 0x01;
    sync_bytes2[4] = 0x67;

    while (m_run_flag_)
    {
        read_data= m_file_ptr_->read(4096);
        if (read_data.isEmpty())
        {
            m_run_flag_ = false;
            break;
        }
        data_buffer += read_data;
        if (is_idr_frame)
        {

            int pos = data_buffer.indexOf(sync_bytes1);
            if (-1 != pos)
            {
                QByteArray data_frame = data_buffer.mid(0, pos);
                QDateTime datetimestamp = QDateTime::currentDateTime();
                qint64 timestamp = datetimestamp.currentMSecsSinceEpoch();

                data_buffer.remove(0, pos);

                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0xFF00000000000000) >> 56));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x00FF000000000000) >> 48));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x0000FF0000000000) >> 40));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x000000FF00000000) >> 32));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000FF000000) >> 24));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x0000000000FF0000) >> 16));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x000000000000FF00) >> 8));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000000000FF) >> 0));
//                data_frame[pos-8] = static_cast<char>((static_cast<quint64>(timestamp) & 0xFF00000000000000) >> 56);
//                data_frame[pos-7] = static_cast<char>((static_cast<quint64>(timestamp) & 0x00FF000000000000) >> 48);
//                data_frame[pos-6] = static_cast<char>((static_cast<quint64>(timestamp) & 0x0000FF0000000000) >> 40);
//                data_frame[pos-5] = static_cast<char>((static_cast<quint64>(timestamp) & 0x000000FF00000000) >> 32);
//                data_frame[pos-4] = static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000FF000000) >> 24);
//                data_frame[pos-3] = static_cast<char>((static_cast<quint64>(timestamp) & 0x0000000000FF0000) >> 16);
//                data_frame[pos-2] = static_cast<char>((static_cast<quint64>(timestamp) & 0x000000000000FF00) >> 8);
//                data_frame[pos-1] = static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000000000FF) >> 0);
                // emit signal
                emit sendFrameData(reinterpret_cast<uint8_t *>(data_frame.data()), data_frame.size());
                is_idr_frame = false;
                QThread::msleep(100);
            }
        } else {
            int pos = -1;
            pos = data_buffer.indexOf(sync_bytes1, 4);
            if (-1 != pos)
            {
                QByteArray data_frame = data_buffer.mid(0, pos);
                QDateTime datetimestamp = QDateTime::currentDateTime();
                qint64 timestamp = datetimestamp.currentMSecsSinceEpoch();

                data_buffer.remove(0, pos);

                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0xFF00000000000000) >> 56));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x00FF000000000000) >> 48));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x0000FF0000000000) >> 40));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x000000FF00000000) >> 32));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000FF000000) >> 24));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x0000000000FF0000) >> 16));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x000000000000FF00) >> 8));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000000000FF) >> 0));
//                data_frame[pos-8] = static_cast<char>((static_cast<quint64>(timestamp) & 0xFF00000000000000) >> 56);
//                data_frame[pos-7] = static_cast<char>((static_cast<quint64>(timestamp) & 0x00FF000000000000) >> 48);
//                data_frame[pos-6] = static_cast<char>((static_cast<quint64>(timestamp) & 0x0000FF0000000000) >> 40);
//                data_frame[pos-5] = static_cast<char>((static_cast<quint64>(timestamp) & 0x000000FF00000000) >> 32);
//                data_frame[pos-4] = static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000FF000000) >> 24);
//                data_frame[pos-3] = static_cast<char>((static_cast<quint64>(timestamp) & 0x0000000000FF0000) >> 16);
//                data_frame[pos-2] = static_cast<char>((static_cast<quint64>(timestamp) & 0x000000000000FF00) >> 8);
//                data_frame[pos-1] = static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000000000FF) >> 0);
                // emit signal
                emit sendFrameData(reinterpret_cast<uint8_t *>(data_frame.data()), data_frame.size());

                QThread::msleep(100);
            }

            pos = data_buffer.indexOf(sync_bytes2);
            if (-1 != pos)
            {
                QByteArray data_frame = data_buffer.mid(0, pos);
                QDateTime datetimestamp = QDateTime::currentDateTime();
                qint64 timestamp = datetimestamp.currentMSecsSinceEpoch();

                data_buffer.remove(0, pos);

                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0xFF00000000000000) >> 56));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x00FF000000000000) >> 48));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x0000FF0000000000) >> 40));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x000000FF00000000) >> 32));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000FF000000) >> 24));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x0000000000FF0000) >> 16));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x000000000000FF00) >> 8));
                data_frame.append(static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000000000FF) >> 0));
//                data_frame[pos-8] = static_cast<char>((static_cast<quint64>(timestamp) & 0xFF00000000000000) >> 56);
//                data_frame[pos-7] = static_cast<char>((static_cast<quint64>(timestamp) & 0x00FF000000000000) >> 48);
//                data_frame[pos-6] = static_cast<char>((static_cast<quint64>(timestamp) & 0x0000FF0000000000) >> 40);
//                data_frame[pos-5] = static_cast<char>((static_cast<quint64>(timestamp) & 0x000000FF00000000) >> 32);
//                data_frame[pos-4] = static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000FF000000) >> 24);
//                data_frame[pos-3] = static_cast<char>((static_cast<quint64>(timestamp) & 0x0000000000FF0000) >> 16);
//                data_frame[pos-2] = static_cast<char>((static_cast<quint64>(timestamp) & 0x000000000000FF00) >> 8);
//                data_frame[pos-1] = static_cast<char>((static_cast<quint64>(timestamp) & 0x00000000000000FF) >> 0);
                // emit signal
                emit sendFrameData(reinterpret_cast<uint8_t *>(data_frame.data()), data_frame.size());
                is_idr_frame = true;
                QThread::msleep(100);
            }
        }
    }
}

void H264FileProcessor::processStream(const QString &input)
{
    int video_stream, ret;
    AVPacket packet;

    /* open the input file */
    AVFormatContext * formatCtx = nullptr;
    if (avformat_open_input(&formatCtx, input.toStdString().c_str(), NULL, NULL) != 0) {
        qWarning() << "Cannot open input file" << input;
        return;
    }

    QScopedPointer<AVFormatContext, AVFormatContextDeleter> inputCtx(formatCtx);

    if (avformat_find_stream_info(inputCtx.data(), NULL) < 0) {
        qWarning() << "Cannot find input stream information.";
        return;
    }

    /* find the video stream information */
    ret = av_find_best_stream(inputCtx.data(), AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        qWarning() << "Cannot find a video stream in the input file";
        return;
    }
    video_stream = ret;
    AVCodecParameters* codecParameters = inputCtx->streams[video_stream]->codecpar;

//    if (!m_decoder->init(codecParameters)) {
//        return;
//    }

//    if (!m_decoder->open()) {
//        return;
//    }

    //Decoding loop
    while (ret >= 0) {
        if ((ret = av_read_frame(inputCtx.data(), &packet)) < 0)
            break;

        if (video_stream == packet.stream_index)
//            ret = m_decoder->decode(&packet);

        av_packet_unref(&packet);
    }

}
