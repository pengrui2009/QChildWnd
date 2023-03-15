#ifndef H264DECODERFACTORY_H
#define H264DECODERFACTORY_H

#include <QObject>
#include "h264swdecoder.h"
#include "h264hwdecoder.h"



class H264DecoderFactory {
public:

    static H264Decoder *createH264Decoder(QObject * parent = nullptr, H264CODEREN type=H264_SOFTWARE_DECODER);
};

#endif /* H264DECODERFACTORY_H */
