/**

 */
#include <sys/wait.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <thread>
#include "liteav_trtc_cloud.h"
#include "trtc_send_video.h"

namespace liteav {
namespace trtc {

#define INFO(fmt, ...) \
        printf("INFO: %s(): (line:%d) " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define WARN(fmt, ...) \
        printf("WARN: %s(): (line:%d) " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define ERROR(fmt, ...) \
        printf("ERROR: %s(): (line:%d) " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

const char* StateToString(ConnectionState state) 
{
    switch (state) {
    case CONNECTION_STATE_INIT:
        return "INIT";
    case CONNECTION_STATE_DISCONNECTED:
        return "DISCONNECTED";
    case CONNECTION_STATE_CONNECTING:
        return "CONNECTING";
    case CONNECTION_STATE_CONNECTED:
        return "CONNECTED";
    default:
        return "UNKNOWN";
    }
}    

TctcSendVideo::TctcSendVideo(QObject *parent, int loop_count) :
    QObject(parent),
    cloud_(TRTCCloud::Create(this)),
    loop_count_(loop_count),
    is_running_(false),
    ready_send_audio_(false),
    ready_send_video_(false) 
{

}

TctcSendVideo::~TctcSendVideo()
{ 
    TRTCCloud::Destroy(cloud_); 
}

void TctcSendVideo::EnterRoom(const EnterRoomParams& params)
{
    is_running_ = true;

    ready_send_video_ = true;
#if 0
    liteav::trtc::EnterRoomParams params;
    params.room.user_id = user.c_str();
    params.room.sdk_app_id = sdk_app_id;
    params.room.user_sig = sig;
    params.room.str_room_id = "888666";//room_id;
    // params.scene must be liteav::trtc::TRTC_SCENE_VIDEO_CALL
    params.scene = liteav::trtc::TRTC_SCENE_VIDEO_CALL;
    // params.role must be liteav::trtc::TRTC_ROLE_ANCHOR
    params.role = liteav::trtc::TRTC_ROLE_ANCHOR;

    // 发送 yuv 数据 params.use_pixel_frame_input must be true
    params.use_pixel_frame_input = true;
#endif
    // Step1. 进入 TRTC 房间。
    cloud_->EnterRoom(params);    
}

bool TctcSendVideo::IsRunning() 
{ 
    return is_running_; 
}

void TctcSendVideo::ExitRoom() 
{
    is_running_ = false;

    StopPush();
    // Step 4. 退出房间
    cloud_->ExitRoom();
}

void TctcSendVideo::StartPush() 
{
    if (!sender_thread_) 
    {
        //sender_thread_.reset(new std::thread(SendThreadMain, this));
    }
}

void TctcSendVideo::StopPush() 
{
    if (sender_thread_) 
    {
        sender_thread_->join();
        sender_thread_.reset();
    }
}

/**
* 检测是否是关键帧
 * @param data_ptr 编码后的h264视频数据
 * @param data_len 编码后的h264视频数据长度
 */
bool TctcSendVideo::isKeyFrame(uint8_t *data_ptr, uint32_t data_len) 
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

void TctcSendVideo::SendH264Video(uint8_t *data_ptr, uint32_t data_len, int fps)
{
    int result = 0;
    bool is_key_frame = isKeyFrame(data_ptr, data_len);
    std::unique_ptr<trtc::VideoFrame> frame_ptr(new trtc::VideoFrame());
    // frame_ptr->width = width;
    // frame_ptr->height = height;
    frame_ptr->codec = VIDEO_CODEC_TYPE_H264;
    frame_ptr->SetData(data_ptr, data_len);
    frame_ptr->pts = timestamp_;
    frame_ptr->dts = timestamp_;
    frame_ptr->is_key_frame =  is_key_frame;
    timestamp_ = 0;//+= 1000 / fps;
    
    int count = loop_count_;
    int frame_interval = 1000 / fps;
    uint32_t next_pts = 30;
    // auto start = std::chrono::steady_clock::now();
    // int len = (data_len > 32) ? 32 : data_len;
    // printf("SendH264Video len:%d data:", data_len);
    // for (int i=0; i<len; i++)
    //     printf("%02X ", data_ptr[i]);
    // printf("\n");
    // while (is_running_ && count > 0)
    // {
        // if (frame_ptr) 
        //{
            //frame_ptr->pts = next_pts;
    result = cloud_->SendVideoFrame(STREAM_TYPE_VIDEO_HIGH, *frame_ptr);
    if (result != 0)
    {
        ERROR("SendVideoFrame failed, result:%d", result);
    }
        //}

        // auto now = std::chrono::steady_clock::now();
        next_pts += frame_interval;

        // auto wait = frame_interval - (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() %frame_interval);
        // std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    // }

    //return frame;
}

#if 0
static void TctcSendVideo::SendThreadMain(SendCloud* sender) 
{ 
    sender->SendLoop(); 
}

// 音视频发送线程。
void TctcSendVideo::SendLoop() 
{
    is_running_ = true;

    if (ready_send_audio_ && !ready_send_video_) 
    {
        SendAudioOnly();
    } else if (ready_send_audio_ && ready_send_video_) {
        SendAudioAndVideo();
    } else if (!ready_send_audio_ && ready_send_video_) {
        SendVideoOnly();
    }

    is_running_ = false;
}

std::unique_ptr<AudioFrame> TctcSendVideo::GetAudioFrame() 
{
    auto data = pcm_reader_->Read();
    if (!data) 
    {
        return nullptr;
    }

    std::unique_ptr<AudioFrame> frame(new AudioFrame());
    frame->bits_per_sample = 16;
    frame->codec = AUDIO_CODEC_TYPE_PCM;
    frame->sample_rate = pcm_reader_->sample_rate();
    frame->SetData(data.get(), pcm_reader_->frame_length_types());
    frame->pts = pcm_reader_->timestamp();
    frame->channels = pcm_reader_->channels();
    return frame;
}

void TctcSendVideo::SendAudioOnly() 
{
    int count = loop_count_;
    // 音频发送接口要求 帧长为 20ms
    int frame_interval = 20;
    uint32_t next_pts = 20;
    auto start = std::chrono::steady_clock::now();
    while (is_running_ && count > 0) {
        auto frame = GetAudioFrame();
        if (frame) {
            frame->pts = next_pts;
            cloud_->SendAudioFrame(*frame.get());
        } else {
            pcm_reader_->Open(pcm_reader_->path(), pcm_reader_->sample_rate(),
                            pcm_reader_->channels());
            count--;
        }

        auto now = std::chrono::steady_clock::now();
        next_pts += frame_interval;

        auto wait =
            frame_interval -
            (std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
                .count() %
            frame_interval);
        std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    }
}

void TctcSendVideo::SendVideoOnly() 
{
    int count = loop_count_;
    int frame_interval = 1000 / yuv_reader_->frame_rate();
    uint32_t next_pts = 20;
    auto start = std::chrono::steady_clock::now();
    while (is_running_ && count > 0)
    {
        auto frame = yuv_reader_->Read();
        if (frame) {
        frame->pts = next_pts;
        cloud_->SendVideoFrame(STREAM_TYPE_VIDEO_HIGH, *frame.get());
        } else {
        yuv_reader_->Open(yuv_reader_->path(), yuv_reader_->width(),
                            yuv_reader_->height(), yuv_reader_->frame_rate());
        count--;
        }

        auto now = std::chrono::steady_clock::now();
        next_pts += frame_interval;

        auto wait =
            frame_interval -
            (std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
                .count() %
            frame_interval);
        std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    }
}

// 发送音视频需要考虑音画同步。
// 音视频交错发送。
void TctcSendVideo::SendAudioAndVideo() 
{
    int count = loop_count_;
    // 音频发送接口要求 帧长为 20ms
    int audio_interval = 20;
    int video_interval = 1000 / yuv_reader_->frame_rate();
    uint32_t time_gone = 0;
    uint32_t audio_next_pts = 10;
    uint32_t video_next_pts = 10;
    bool audio_eof = false;
    bool video_eof = false;
    auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    while (is_running_ && count > 0) {
        if (audio_next_pts <= video_next_pts) {
        auto frame = GetAudioFrame();

        if (frame) {
            frame->pts = audio_next_pts;
            cloud_->SendAudioFrame(*frame.get());
        } else {
            audio_eof = true;
        }
        now = std::chrono::steady_clock::now();
        audio_next_pts += audio_interval;
        } else {
        auto frame = yuv_reader_->Read();

        if (frame) {
            frame->pts = video_next_pts;
            cloud_->SendVideoFrame(STREAM_TYPE_VIDEO_HIGH, *frame.get());
        } else {
            video_eof = true;
        }
        now = std::chrono::steady_clock::now();
        video_next_pts += video_interval;
        }

        time_gone =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
                .count();

        // 模拟真实的采集过程，按实际帧的间隔发送。
        int wait = std::min(audio_next_pts, video_next_pts) - time_gone;
        std::this_thread::sleep_for(std::chrono::milliseconds(wait));

        // 循环处理。
        if (audio_eof && video_eof) {
            pcm_reader_->Open(pcm_reader_->path(), pcm_reader_->sample_rate(),
                                pcm_reader_->channels());
            yuv_reader_->Open(yuv_reader_->path(), yuv_reader_->width(),
                                yuv_reader_->height(), yuv_reader_->frame_rate());
            count--;
            audio_eof = false;
            video_eof = false;
        }
    }
}
#endif
// 以下是回调事件处理，
void TctcSendVideo::OnError(Error error) 
{
    ERROR("TRTCCloud Error occured!!!");
}

void TctcSendVideo::OnEnterRoom() 
{
    INFO("OnEnterRoom.")
    // Step 2. 进房成功后，按需创建音视频发送通道。
    if (ready_send_audio_) 
    {
        liteav::trtc::AudioEncodeParams params;
        params.channels = 2;
        cloud_->CreateLocalAudioChannel(params);
    }

    if (ready_send_video_) 
    {
        cloud_->CreateLocalVideoChannel(liteav::trtc::STREAM_TYPE_VIDEO_HIGH);
    }
}

void TctcSendVideo::OnLocalAudioChannelCreated() 
{
    // Step 3. 音频通道创建成功后，开始推流
    // StartPush();
}

void TctcSendVideo::OnLocalVideoChannelCreated(StreamType type) 
{
    // Step 3. 视频通道创建成功后，开始推流
    // StartPush();
}

void TctcSendVideo::OnExitRoom()
{ 
    INFO("OnExitRoom!"); 
}

// Note:
// 默认自动订阅远端用户的音视频数据
// 这里可以主动取消 音频/视频 的订阅
void TctcSendVideo::OnRemoteUserEnterRoom(const UserInfo& info) 
{
    INFO("OnRemoteUserEnterRoom Unsubscribe User:%s", info.user_id);
//    cloud_->Unsubscribe(info.user_id, STREAM_TYPE_VIDEO_HIGH);
//    cloud_->Unsubscribe(info.user_id, STREAM_TYPE_VIDEO_LOW);
//    cloud_->Unsubscribe(info.user_id, STREAM_TYPE_VIDEO_AUX);
//    cloud_->Unsubscribe(info.user_id, STREAM_TYPE_AUDIO);
}

void TctcSendVideo::OnRemoteUserExitRoom(const UserInfo& info) 
{
    INFO("OnRemoteUserExitRoom User:%s", info.user_id);
}

void TctcSendVideo::OnRemoteAudioAvailable(const char* user_id, bool available) 
{
    INFO("OnRemoteAudioAvailable User:%s available:%d", user_id, available);
}

void TctcSendVideo::OnRemoteVideoAvailable(const char* user_id,
                            bool available,
                            StreamType type) 
{
    INFO("OnRemoteVideoAvailable User:%s available:%d", user_id, available);
}

void TctcSendVideo::OnRemoteVideoReceived(const char* user_id,
                            StreamType type,
                            const VideoFrame& frame) 
{
//    INFO("OnRemoteVideoReceived User:%s type:%d", user_id, type);
    emit sendFrameData(frame.data(), static_cast<int>(frame.size()));
}

void TctcSendVideo::OnRemoteVideoReceived(const char* user_id,
                            StreamType type,
                            const PixelFrame& frame) 
{
    INFO("OnRemoteVideoReceived User:%s type:%d", user_id, type);
}

void TctcSendVideo::OnRemoteAudioReceived(const char* user_id,
                            const AudioFrame& frame) 
{
    INFO("OnRemoteAudioReceived User:%s", user_id);
}

void TctcSendVideo::OnRemoteMixedAudioReceived(const AudioFrame& frame) 
{
    INFO("OnRemoteVideoReceived");
}

void TctcSendVideo::OnConnectionStateChanged(ConnectionState old_state,
                            ConnectionState new_state) 
{
    INFO("OnConnectionStateChanged from %s to %s", StateToString(old_state),StateToString(new_state));
}

void TctcSendVideo::OnLocalAudioChannelDestroyed() 
{

}

void TctcSendVideo::OnLocalVideoChannelDestroyed(StreamType type) 
{

}

void TctcSendVideo::OnRequestChangeVideoEncodeBitrate(StreamType type,
                                        int bitrate_bps) 
{

}

}  // namespace trtc
}  // namespace liteav
