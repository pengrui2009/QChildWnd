#include "h264decoderfactory.h"

H264Decoder *H264DecoderFactory::createH264Decoder(QObject * parent, H264CODEREN type)
{
    switch(type)
    {
    case H264_HARDWARE_DECODER:
        return new H264HWDecoder(parent);
    case H264_SOFTWARE_DECODER:
        return new H264SWDecoder(parent);
    }
}
