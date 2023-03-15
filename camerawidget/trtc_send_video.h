#ifndef TRTC_SEND_VIDEO_H_
#define TRTC_SEND_VIDEO_H_


#include <sys/wait.h>

#include <QObject>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <thread>
//#include "liteav_trtc_cloud.h"

// #include "cxxopts.h"
// #include "file.h"
// #include "liteav_trtc_cloud.h"
// #include "log.h"
// #include "pcm_reader.h"
// #include "user_sig_generator.h"
// #include "yuv_reader.h"

namespace liteav {
namespace trtc {


class TctcSendVideo : public QObject, public TRTCCloudDelegate
{
    Q_OBJECT
public:
    explicit TctcSendVideo(QObject *parent  = nullptr, int loop_count = 3);
    ~TctcSendVideo();
    // |pcm_file_path| = '' 表示不推音频
    // |yuv_file_path| = '' 表示不推视频
    // |width| yuv 数据的宽，单位像素
    // |height| yuv 数据的高，单位像素
    // |fps| 推送帧率
    void EnterRoom(const EnterRoomParams& params);
    bool IsRunning();
    void ExitRoom();

    void SendH264Video(uint8_t *data_ptr, uint32_t data_len, int fps);
signals:
    void sendFrameData(const uint8_t *data_ptr, int data_len);
protected:
    void StartPush() ;
    void StopPush();
    // static void SendThreadMain(SendCloud* sender);
    // 音视频发送线程。
    // void SendLoop();
    // std::unique_ptr<AudioFrame> GetAudioFrame();
    // void SendAudioOnly();
    // void SendVideoOnly();
    // 发送音视频需要考虑音画同步。
    // 音视频交错发送。
    // void SendAudioAndVideo();
    bool isKeyFrame(uint8_t *data_ptr, uint32_t data_len);
    

    // 以下是回调事件处理，
    void OnError(Error error) override;
    void OnEnterRoom() override;
    void OnLocalAudioChannelCreated() override;
    void OnLocalVideoChannelCreated(StreamType type) override;
    void OnExitRoom() override;
    // Note:
    // 默认自动订阅远端用户的音视频数据
    // 这里可以主动取消 音频/视频 的订阅
    void OnRemoteUserEnterRoom(const UserInfo& info) override;
    void OnRemoteUserExitRoom(const UserInfo& info) override;
    void OnRemoteAudioAvailable(const char* user_id, bool available) override;
    void OnRemoteVideoAvailable(const char* user_id,
                                bool available,
                                StreamType type) override;
    void OnRemoteVideoReceived(const char* user_id,
                                StreamType type,
                                const VideoFrame& frame) override;
    void OnRemoteVideoReceived(const char* user_id,
                                StreamType type,
                                const PixelFrame& frame) override;
    void OnRemoteAudioReceived(const char* user_id, const AudioFrame& frame) override;
    void OnRemoteMixedAudioReceived(const AudioFrame& frame) override;
    void OnConnectionStateChanged(ConnectionState old_state, ConnectionState new_state) override ;
    void OnLocalAudioChannelDestroyed() override;
    void OnLocalVideoChannelDestroyed(StreamType type) override;
    void OnRequestChangeVideoEncodeBitrate(StreamType type, int bitrate_bps) override;
private:
    //   std::unique_ptr<PcmReader> pcm_reader_;
    //   std::unique_ptr<YuvReader> yuv_reader_;
    TRTCCloud* cloud_;
    std::unique_ptr<std::thread> sender_thread_;
    int loop_count_;
    bool is_running_;
    bool ready_send_audio_;
    bool ready_send_video_;
    int timestamp_;
};

}  // namespace trtc
}  // namespace liteav


#endif /* TRTC_SEND_VIDEO_H_ */
