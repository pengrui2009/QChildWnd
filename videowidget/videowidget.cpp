//#pragma execution_character_set("utf-8")

#include "videowidget.h"
#include "qfontdatabase.h"
#include "qpushbutton.h"
#include "qtreewidget.h"
#include "qlayout.h"
#include "qtimer.h"
#include "qdir.h"
#include "qpainter.h"
#include "qevent.h"
#include "qmimedata.h"
#include "qurl.h"
#include "qdebug.h"

#include <iostream>
#include <string>


//#include <QtConcurrent>


VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    //设置强焦点
//    setFocusPolicy(Qt::StrongFocus);
    setFocusPolicy(Qt::NoFocus);

    // setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
    //设置支持拖放
    setAcceptDrops(true);

    checkLive = true;
    timerCheck = new QTimer(this);
    timerCheck->setInterval(1 * 1000);
    connect(timerCheck, SIGNAL(timeout()), this, SLOT(checkVideo()));

    image = QImage();

    //顶部工具栏,默认隐藏,鼠标移入显示移除隐藏
//    flowPanel8 = new SignalRssi(this);
//    flowPanel8->setObjectName("flowPanel8");
//    flowPanel8->setVisible(true);

    left_camera_Pannel = new CameraWidget(this);
    left_camera_Pannel->setObjectName("left_camera_Pannel");
    left_camera_Pannel->setTitle("左后视角");
    left_camera_Pannel->setPannelPosition(SignalRssi::POSITION_LEFT_TOP);
//    left_camera_Pannel->setVisible(true);
    left_camera_Pannel->setVisible(true);

    right_camera_Pannel = new CameraWidget(this);
    right_camera_Pannel->setObjectName("right_camera_Pannel");
    right_camera_Pannel->setTitle("右后视角");
    right_camera_Pannel->setPannelPosition(SignalRssi::POSITION_RIGHT_TOP);
//    right_camera_Pannel->setVisible(true);
    right_camera_Pannel->setVisible(true);

    //用布局顶住,左侧弹簧
    QVBoxLayout *vlayout0 = new QVBoxLayout;
    vlayout0->setSpacing(2);
    vlayout0->setMargin(0);
    vlayout0->addStretch();
    //flowPanel0->setLayout(vlayout0);

    QVBoxLayout *layout1 = new QVBoxLayout;
    layout1->setSpacing(2);
    layout1->setMargin(0);
    layout1->addStretch();
    //flowPanel1->setLayout(layout1);

    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->setSpacing(2);
    layout2->setMargin(0);
    layout2->addStretch();
    //flowPanel2->setLayout(layout2);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setSpacing(2);
    hlayout->setMargin(0);
    hlayout->addStretch();

    hlayout->addWidget(left_camera_Pannel);
    hlayout->addStretch();

    hlayout->addWidget(right_camera_Pannel);
    hlayout->addStretch();
//    this->setLayout(hlayout);


    //按钮集合名称,如果需要新增按钮则在这里增加即可
    QList<QString> btns;
    btns << "btnFlowVideo" << "btnFlowSound" << "btnFlowSnap" << "btnFlowClose";

    //有多种办法来设置图片,qt内置的图标+自定义的图标+图形字体
    //既可以设置图标形式,也可以直接图形字体设置文本
#if 0
    QList<QIcon> icons;
    icons << QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);
    icons << QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    icons << QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    icons << QApplication::style()->standardIcon(QStyle::SP_DialogOkButton);
    icons << QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton);
#else
    QList<QChar> chars;
    chars << 0xf03d << 0xf028 << 0xf030 << 0xf057;

    //判断图形字体是否存在,不存在则加入
    QFont iconFont;
    QFontDatabase fontDb;
    if (!fontDb.families().contains("iconfont")) {
        int fontId = fontDb.addApplicationFont(":/image/fontawesome-webfont.ttf");
        QStringList fontName = fontDb.applicationFontFamilies(fontId);
        if (fontName.count() == 0) {
            qDebug() << "load fontawesome-webfont.ttf error";
        }
    }

    if (fontDb.families().contains("fontawesome")) {
        iconFont = QFont("fontawesome");
        iconFont.setPixelSize(17);
#if (QT_VERSION >= QT_VERSION_CHECK(4,8,0))
        iconFont.setHintingPreference(QFont::PreferNoHinting);
#endif
    }
#endif

    //循环添加顶部按钮
    for (int i = 0; i < btns.count(); i++) {
        QPushButton *btn = new QPushButton;
        //绑定按钮单击事件,用来发出信号通知
        connect(btn, SIGNAL(clicked(bool)), this, SLOT(btnClicked()));
        //设置标识,用来区别按钮
        btn->setObjectName(btns.at(i));
        //设置固定宽度
        btn->setFixedWidth(20);
        //设置拉伸策略使得填充
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        //设置焦点策略为无焦点,避免单击后焦点跑到按钮上
        btn->setFocusPolicy(Qt::NoFocus);

#if 0
        //设置图标大小和图标
        btn->setIconSize(QSize(16, 16));
        btn->setIcon(icons.at(i));
#else
        btn->setFont(iconFont);
        btn->setText(chars.at(i));
#endif

        //将按钮加到布局中
        // layout->addWidget(btn);
    }

    copyImage = true;
    checkLive = true;
    drawImage = true;
    fillImage = true;

    flowEnable = false;
    flowBgColor = "#000000";
    flowPressColor = "#5EC7D9";

    timeout = 1;
    borderWidth = 2;
    borderHeight = 30;
    borderColor = "#000000";
    focusColor = "#22A3A9";
    bgText = "实时视频";
//    bgImage = QImage(":/image/logo.png");

    osd1Visible = false;
    osd1FontSize = 12;
    osd1Text = "时间";
    osd1Color = "#FFFF34";
    osd1Image = QImage();
    osd1Format = OSDFormat_DateTime;
    osd1Position = OSDPosition_Right_Top;

    osd2Visible = false;
    osd2FontSize = 12;
    osd2Text = "通道名称";
    osd2Color = "#FFFF34";
    osd2Image = QImage();
    osd2Format = OSDFormat_Text;
    osd2Position = OSDPosition_Left_Bottom;

    flowEnable = true;

    m_left_camera_geometry_x = 20;
    m_left_camera_geometry_y = 20;
    m_left_camera_geometry_w = 300;
    m_left_camera_geometry_h = 200;

    m_right_camera_geometry_x = 20;
    m_right_camera_geometry_y = 20;
    m_right_camera_geometry_w = 300;
    m_right_camera_geometry_h = 200;
//    m_codec_hardware_ = H264_HARDWARE_DECODER;

//    m_decoder = H264DecoderFactory::createH264Decoder(this, m_codec_hardware_);
//    m_decoder->init();
//    m_decoder->open();
//    connect(m_decoder, &H264HWDecoder::decodedImageData, this, &VideoWidget::updateImage);

//    m_h264_file_ptr_ = new H264FileProcessor();
//    m_h264_file_ptr_->init();
//    connect(m_h264_file_ptr_, SIGNAL(sendFrameData(const uint8_t *, int)), m_decoder, SLOT(slot_input_frame_data(const uint8_t *, int)));

    // init trtc cloud handle
//    trtc_handle_ptr_ = new liteav::trtc::TctcSendVideo(parent, 3);
//    connect(trtc_handle_ptr_, SIGNAL(sendFrameData(const uint8_t *, int)), m_decoder, SLOT(slot_input_frame_data(const uint8_t *, int)));


    this->initFlowStyle();

    if (checkLive) {
        lastTime = QDateTime::currentDateTime();
        timerCheck->start();
    }
}

VideoWidget::~VideoWidget()
{
    if (timerCheck->isActive())
    {
        timerCheck->stop();
    }

//    if (m_h264_file_ptr_)
//    {
//        delete m_h264_file_ptr_;
//    }

//    if (m_decoder)
//    {
//        delete m_decoder;
//    }

//    trtc_handle_ptr_->ExitRoom();
//    delete image;

    close();
}

void VideoWidget::resizeEvent(QResizeEvent *)
{
    //重新设置顶部工具栏的位置和宽高,可以自行设置顶部显示或者底部显示
    int height = 8;
//    flowPanel0->setGeometry(borderWidth, borderWidth, this->width() - (borderWidth * 2), height);
    //flowPanel->setGeometry(borderWidth, this->height() - height - borderWidth, this->width() - (borderWidth * 2), height);



    left_camera_Pannel->setGeometry(m_left_camera_geometry_x, m_left_camera_geometry_y, m_left_camera_geometry_w, m_left_camera_geometry_h);

    right_camera_Pannel->setGeometry(m_right_camera_geometry_x, m_right_camera_geometry_y, m_right_camera_geometry_w, m_right_camera_geometry_h);
//    qDebug() << "right camera:" << 10 << "," << this->height()/2 - 300 << ","<< 512 << "," << 288;
}

void VideoWidget::enterEvent(QEvent *)
{
    //这里还可以增加一个判断,是否获取了焦点的才需要显示
    //if (this->hasFocus()) {}
    if (flowEnable) {
        if (m_viewMode == 0) {
    //        m_viewMode = true;

    #if 0
            left_camera_Pannel->setVisible(true);
            right_camera_Pannel->setVisible(true);
    #endif
        }
        else if (m_viewMode == 1)
        {
    //        m_viewMode = false;

            left_camera_Pannel->setVisible(true);
            right_camera_Pannel->setVisible(false);

        } else if (m_viewMode == 2){
    //        m_viewMode = false;


            left_camera_Pannel->setVisible(false);
            right_camera_Pannel->setVisible(true);
        } else if (m_viewMode == 3) {
    //        m_viewMode = false;

        }
    } else {


        left_camera_Pannel->setVisible(false);
        right_camera_Pannel->setVisible(false);

    }
}

void VideoWidget::leaveEvent(QEvent *)
{
    if (flowEnable) {
        if (m_viewMode == 0) {
    //        m_viewMode = true;

    #if 0
            left_camera_Pannel->setVisible(true);
            right_camera_Pannel->setVisible(true);
    #endif
        }
        else if (m_viewMode == 1)
        {
    //        m_viewMode = false;


            left_camera_Pannel->setVisible(true);
            right_camera_Pannel->setVisible(false);

        } else if (m_viewMode == 2){
    //        m_viewMode = false;


            left_camera_Pannel->setVisible(false);
            right_camera_Pannel->setVisible(true);
        } else if (m_viewMode == 3) {
    //        m_viewMode = false;

        }
    } else {

#if 0
        left_camera_Pannel->setVisible(false);
        right_camera_Pannel->setVisible(false);
#endif
    }
}

void VideoWidget::dropEvent(QDropEvent *event)
{
    //拖放完毕鼠标松开的时候执行
    //判断拖放进来的类型,取出文件,进行播放
    if(event->mimeData()->hasUrls()) {
        QString url = event->mimeData()->urls().first().toLocalFile();
        this->close();
        this->setUrl(url);
        this->open();
        emit fileDrag(url);
    } else if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        QTreeWidget *treeWidget = (QTreeWidget *)event->source();
        if (treeWidget != nullptr) {
            QString url = treeWidget->currentItem()->data(0, Qt::UserRole).toString();
            this->close();
            this->setUrl(url);
            this->open();
            emit fileDrag(url);
        }
    }
}

void VideoWidget::dragEnterEvent(QDragEnterEvent *event)
{
    //拖曳进来的时候先判断下类型,非法类型则不处理
    if(event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else if(event->mimeData()->hasFormat("text/uri-list")) {
        event->setDropAction(Qt::LinkAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void VideoWidget::drawRightLane(QPainter *painter)
{
    int width = this->width();
    int height = this->height();

    QLine lines[] = {
        {width/2 - 400, height - 10, width/4+340 + 10, 450},
        {width/2 + 550, height - 10, width*3/4-340 + 60, 450}
    };

    painter->save();
    painter->setPen(QPen(QColor(51, 123, 35), 3));
    QTransform transfrom;
//    painter->setTransform(transfrom.rotate(-6.0, Qt::ZAxis), false);
    painter->drawLines(lines, sizeof(lines)/sizeof(lines[0]));
    painter->restore();

//    painter->save();
//    painter->setPen(QPen(Qt::red, 1));
//    QTransform transfrom1;
//    painter->setTransform(transfrom1.rotate(6.0, Qt::ZAxis), false);
//    painter->drawLines(&lines[1], 1);
//    painter->restore();
}

void VideoWidget::paintEvent(QPaintEvent *)
{
//    int width = this->width();
//    int height = this->height();

    //如果不需要绘制
    if (!drawImage) {
        return;
    }

    QStyleOption opt;
    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
//    painter.translate(width / 2, height / 2);

    //绘制边框
    drawBorder(&painter);

    if (!image.isNull()) {
        //绘制背景图片

        drawImg(&painter, image);

        //绘制标签1
//        if (osd1Visible) {
//            drawOSD(&painter, osd1FontSize, osd1Text, osd1Color, osd1Image, osd1Format, osd1Position);
//        }

        //绘制标签2
//        if (osd2Visible) {
//            drawOSD(&painter, osd2FontSize, osd2Text, osd2Color, osd2Image, osd2Format, osd2Position);
//        }
    } else {
        //绘制背景
        drawBg(&painter);
    }
    if (m_viewMode == 0)
    {
        drawRightLane(&painter);
    }
    // qss style
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void VideoWidget::drawBorder(QPainter *painter)
{
    painter->save();
    QPen pen;
    pen.setWidth(borderWidth);
    pen.setColor(hasFocus() ? focusColor : borderColor);
    painter->setPen(pen);
    painter->drawRect(rect());
    painter->restore();
}

void VideoWidget::drawBg(QPainter *painter)
{
    painter->save();

    //背景图片为空则绘制文字,否则绘制背景图片
    if (bgImage.isNull()) {
        painter->setPen(palette().foreground().color());
        painter->drawText(rect(), Qt::AlignCenter, bgText);
    } else {
        //居中绘制
        int pixX = rect().center().x() - bgImage.width() / 2;
        int pixY = rect().center().y() - bgImage.height() / 2;
        QPoint point(pixX, pixY);
        painter->drawImage(point, bgImage);
    }

    painter->restore();
}

void VideoWidget::drawImg(QPainter *painter, QImage img)
{
    painter->save();

    int offset = borderWidth * 1 + 0;
    if (fillImage) {
        QRect rect(offset / 2, offset / 2, width() - offset, height() - offset);
        painter->drawImage(rect, img);
    } else {
        //按照比例自动居中绘制
        img = img.scaled(width() - offset, height() - offset, Qt::KeepAspectRatio);
        int pixX = rect().center().x() - img.width() / 2;
        int pixY = rect().center().y() - img.height() / 2;
        QPoint point(pixX, pixY);
        painter->drawImage(point, img);
    }

    painter->restore();
}

void VideoWidget::drawOSD(QPainter *painter,
                          int osdFontSize,
                          const QString &osdText,
                          const QColor &osdColor,
                          const QImage &osdImage,
                          const VideoWidget::OSDFormat &osdFormat,
                          const VideoWidget::OSDPosition &osdPosition)
{
    painter->save();

    //标签位置尽量偏移多一点避免遮挡
    QRect osdRect(rect().x() + (borderWidth * 2), rect().y() + (borderWidth * 2), width() - (borderWidth * 5), height() - (borderWidth * 5));
    int flag = Qt::AlignLeft | Qt::AlignTop;
    QPoint point = QPoint(osdRect.x(), osdRect.y());

    if (osdPosition == OSDPosition_Left_Top) {
        flag = Qt::AlignLeft | Qt::AlignTop;
        point = QPoint(osdRect.x(), osdRect.y());
    } else if (osdPosition == OSDPosition_Left_Bottom) {
        flag = Qt::AlignLeft | Qt::AlignBottom;
        point = QPoint(osdRect.x(), osdRect.height() - osdImage.height());
    } else if (osdPosition == OSDPosition_Right_Top) {
        flag = Qt::AlignRight | Qt::AlignTop;
        point = QPoint(osdRect.width() - osdImage.width(), osdRect.y());
    } else if (osdPosition == OSDPosition_Right_Bottom) {
        flag = Qt::AlignRight | Qt::AlignBottom;
        point = QPoint(osdRect.width() - osdImage.width(), osdRect.height() - osdImage.height());
    }

    if (osdFormat == OSDFormat_Image) {
        painter->drawImage(point, osdImage);
    } else {
        QDateTime now = QDateTime::currentDateTime();
        QString text = osdText;
        if (osdFormat == OSDFormat_Date) {
            text = now.toString("yyyy-MM-dd");
        } else if (osdFormat == OSDFormat_Time) {
            text = now.toString("HH:mm:ss");
        } else if (osdFormat == OSDFormat_DateTime) {
            text = now.toString("yyyy-MM-dd HH:mm:ss:zzz");
        }

        //设置颜色及字号
        QFont font;
        font.setPixelSize(osdFontSize);
        painter->setPen(osdColor);
        painter->setFont(font);

        painter->drawText(osdRect, flag, text);
    }

    painter->restore();
}

unsigned int VideoWidget::getRoomId() const
{
    return this->m_room_id_;
}

QImage VideoWidget::getImage() const
{
    return this->image;
}

QDateTime VideoWidget::getLastTime() const
{
    return QDateTime::currentDateTime();
}

QString VideoWidget::getUrl() const
{
    return this->property("url").toString();
}

bool VideoWidget::getCopyImage() const
{
    return this->copyImage;
}

bool VideoWidget::getCheckLive() const
{
    return this->checkLive;
}

bool VideoWidget::getDrawImage() const
{
    return this->drawImage;
}

bool VideoWidget::getFillImage() const
{
    return this->fillImage;
}

bool VideoWidget::getFlowEnable() const
{
    return this->flowEnable;
}

QColor VideoWidget::getFlowBgColor() const
{
    return this->flowBgColor;
}

QColor VideoWidget::getFlowPressColor() const
{
    return this->flowPressColor;
}

int VideoWidget::getTimeout() const
{
    return this->timeout;
}

int VideoWidget::getBorderWidth() const
{
    return this->borderWidth;
}

QColor VideoWidget::getBorderColor() const
{
    return this->borderColor;
}

QColor VideoWidget::getFocusColor() const
{
    return this->focusColor;
}

QString VideoWidget::getBgText() const
{
    return this->bgText;
}

QImage VideoWidget::getBgImage() const
{
    return this->bgImage;
}

bool VideoWidget::getOSD1Visible() const
{
    return this->osd1Visible;
}

int VideoWidget::getOSD1FontSize() const
{
    return this->osd1FontSize;
}

QString VideoWidget::getOSD1Text() const
{
    return this->osd1Text;
}

QColor VideoWidget::getOSD1Color() const
{
    return this->osd1Color;
}

QImage VideoWidget::getOSD1Image() const
{
    return this->osd1Image;
}

VideoWidget::OSDFormat VideoWidget::getOSD1Format() const
{
    return this->osd1Format;
}

VideoWidget::OSDPosition VideoWidget::getOSD1Position() const
{
    return this->osd1Position;
}

bool VideoWidget::getOSD2Visible() const
{
    return this->osd2Visible;
}

int VideoWidget::getOSD2FontSize() const
{
    return this->osd2FontSize;
}

QString VideoWidget::getOSD2Text() const
{
    return this->osd2Text;
}

QColor VideoWidget::getOSD2Color() const
{
    return this->osd2Color;
}

QImage VideoWidget::getOSD2Image() const
{
    return this->osd2Image;
}

VideoWidget::OSDFormat VideoWidget::getOSD2Format() const
{
    return this->osd2Format;
}

VideoWidget::OSDPosition VideoWidget::getOSD2Position() const
{
    return this->osd2Position;
}

int VideoWidget::getLeftCameraGeometryX() const
{
    return this->left_camera_Pannel->geometry().x();
}

int VideoWidget::getLeftCameraGeometryY() const
{
    return this->left_camera_Pannel->geometry().y();
}

int VideoWidget::getLeftCameraGeometryWidth() const
{
    return this->left_camera_Pannel->geometry().width();
}

int VideoWidget::getLeftCameraGeometryHeight() const
{
    return this->left_camera_Pannel->geometry().height();
}

int VideoWidget::getRightCameraGeometryX() const
{
    return this->right_camera_Pannel->geometry().x();
}

int VideoWidget::getRightCameraGeometryY() const
{
    return this->right_camera_Pannel->geometry().y();
}

int VideoWidget::getRightCameraGeometryWidth() const
{
    return this->right_camera_Pannel->geometry().width();
}

int VideoWidget::getRightCameraGeometryHeight() const
{
    return this->right_camera_Pannel->geometry().height();
}


QSize VideoWidget::sizeHint() const
{
    return QSize(500, 350);
}

QSize VideoWidget::minimumSizeHint() const
{
    return QSize(50, 35);
}

void VideoWidget::initFlowStyle()
{
    //设置样式以便区分,可以自行更改样式,也可以不用样式
    QStringList qss;
    QString rgba = QString("rgba(%1,%2,%3,130)").arg(flowBgColor.red()).arg(flowBgColor.green()).arg(flowBgColor.blue());
    qss.append(QString("#flowPanel0{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel1{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel2{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel3{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel4{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel5{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel6{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel7{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel8{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel9{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel10{background:%1;border:none;}").arg(rgba));
    qss.append(QString("QPushButton{border:none;padding:0px;background:rgba(0,0,0,0);}"));
    qss.append(QString("QPushButton:pressed{color:%1;}").arg(flowPressColor.name()));

}

void VideoWidget::updateImage(int position, double timestamp, const QImage &image)
{
    switch (position)
    {
    case 0:
    //    QDateTime datetimestamp = QDateTime::currentDateTime();
    //    qint64 current_timestamp = datetimestamp.currentMSecsSinceEpoch();
    //    qint64 ts = current_timestamp - static_cast<qint64>(timestamp);
        osd1Text = QString("%1ms").arg(timestamp);
        osd1Format = OSDFormat_Text;
        //拷贝图片有个好处,当处理器比较差的时候,图片不会产生断层,缺点是占用时间
        //默认QImage类型是浅拷贝,可能正在绘制的时候,那边已经更改了图片的上部分数据
        if (m_viewMode == 0)
        {
            // flowPanel7->setVideoDelay(timestamp);
        } else {
            // flowPanel8->setRssi(timestamp);
        }

        this->image = copyImage ? image.copy() : image;
        lastTime = QDateTime::currentDateTime();
        this->update();
        break;
    case 1:
        if (left_camera_Pannel)
        {
            left_camera_Pannel->updateImage(timestamp, image);
        }
        break;
    case 2:
        if (right_camera_Pannel)
        {
            right_camera_Pannel->updateImage(timestamp, image);
        }
        break;
    }

}

void VideoWidget::checkVideo()
{
    QImage image(1920, 1080, QImage::Format_RGB888);
    QDateTime now = QDateTime::currentDateTime();

    for (int i=0; i<1080; i++)
    {
        for (int j=0; j<1920; j++)
        {
            if ((m_viewMode == 0) ||(m_viewMode == 3))
            {
                image.setPixel(j, i, 0xFF25496A);
            } else {
                image.setPixel(j, i, 0x0000006A);
            }
        }
    }

    int sec = static_cast<int>(lastTime.secsTo(now));
    if ((sec >= timeout) || (sec < 0))
    {
        // restart();
        updateImage(0, 0.0, image);
    }

}

void VideoWidget::btnClicked()
{
    QPushButton *btn = (QPushButton *)sender();
    emit btnclicked(btn->objectName());

    if (btn->objectName() == "btnFlowVideo")
    {
        this->open();
    } else if (btn->objectName() == "btnFlowClose")
    {
        this->close();
    }
}

void VideoWidget::setRoomId(unsigned int roomid)
{
    this->m_room_id_ = roomid;
}

void VideoWidget::setInterval(int interval)
{

}

void VideoWidget::setSleepTime(int sleepTime)
{

}

void VideoWidget::setCheckTime(int checkTime)
{

}

void VideoWidget::setCheckConn(bool checkConn)
{

}

void VideoWidget::setUrl(const QString &url)
{
    this->setProperty("url", url);
}

void VideoWidget::setHardware(const QString &hardware)
{

}

void VideoWidget::setSaveFile(bool saveFile)
{

}

void VideoWidget::setSaveInterval(int saveInterval)
{

}

void VideoWidget::setSavePath(const QString &savePath)
{
    //如果目录不存在则新建
    QDir dir(savePath);
    if (!dir.exists()) {
        dir.mkdir(savePath);
    }


}

void VideoWidget::setFileName(const QString &fileName)
{

}

void VideoWidget::setCopyImage(bool copyImage)
{
    this->copyImage = copyImage;
}

void VideoWidget::setCheckLive(bool checkLive)
{
    this->checkLive = checkLive;
}

void VideoWidget::setDrawImage(bool drawImage)
{
    this->drawImage = drawImage;
}

void VideoWidget::setFillImage(bool fillImage)
{
    this->fillImage = fillImage;
}

void VideoWidget::setFlowEnable(bool flowEnable)
{
    this->flowEnable = flowEnable;
}

void VideoWidget::setFlowBgColor(const QColor &flowBgColor)
{
    if (this->flowBgColor != flowBgColor) {
        this->flowBgColor = flowBgColor;
        this->initFlowStyle();
    }
}

void VideoWidget::setFlowPressColor(const QColor &flowPressColor)
{
    if (this->flowPressColor != flowPressColor) {
        this->flowPressColor = flowPressColor;
        this->initFlowStyle();
    }
}

void VideoWidget::setTimeout(int timeout)
{
    this->timeout = timeout;
}

void VideoWidget::setBorderWidth(int borderWidth)
{
    this->borderWidth = borderWidth;
}

void VideoWidget::setBorderColor(const QColor &borderColor)
{
    this->borderColor = borderColor;
}

void VideoWidget::setFocusColor(const QColor &focusColor)
{
    this->focusColor = focusColor;
}

void VideoWidget::setBgText(const QString &bgText)
{
    this->bgText = bgText;
}

void VideoWidget::setBgImage(const QImage &bgImage)
{
    this->bgImage = bgImage;
}

void VideoWidget::setOSD1Visible(bool osdVisible)
{
    this->osd1Visible = osdVisible;
}

void VideoWidget::setOSD1FontSize(int osdFontSize)
{
    this->osd1FontSize = osdFontSize;
}

void VideoWidget::setOSD1Text(const QString &osdText)
{
    this->osd1Text = osdText;
}

void VideoWidget::setOSD1Color(const QColor &osdColor)
{
    this->osd1Color = osdColor;
}

void VideoWidget::setOSD1Image(const QImage &osdImage)
{
    this->osd1Image = osdImage;
}

void VideoWidget::setOSD1Format(const VideoWidget::OSDFormat &osdFormat)
{
    this->osd1Format = osdFormat;
}

void VideoWidget::setOSD1Position(const VideoWidget::OSDPosition &osdPosition)
{
    this->osd1Position = osdPosition;
}

void VideoWidget::setOSD2Visible(bool osdVisible)
{
    this->osd2Visible = osdVisible;
}

void VideoWidget::setOSD2FontSize(int osdFontSize)
{
    this->osd2FontSize = osdFontSize;
}

void VideoWidget::setOSD2Text(const QString &osdText)
{
    this->osd2Text = osdText;
}

void VideoWidget::setOSD2Color(const QColor &osdColor)
{
    this->osd2Color = osdColor;
}

void VideoWidget::setOSD2Image(const QImage &osdImage)
{
    this->osd2Image = osdImage;
}

void VideoWidget::setOSD2Format(const VideoWidget::OSDFormat &osdFormat)
{
    this->osd2Format = osdFormat;
}

void VideoWidget::setOSD2Position(const VideoWidget::OSDPosition &osdPosition)
{
    this->osd2Position = osdPosition;
}

void VideoWidget::play(const QString &input)
{
//    m_h264_file_ptr_->open(input);
//    m_h264_file_ptr_->processFile();
}

void VideoWidget::open()
{
//    qDebug() << TIMEMS << "open video" << objectName();
//    qDebug() << "open video url:" << this->property("url").toString();
    clear();
#if 0
    //如果是图片则只显示图片就行
    image = QImage(this->property("url").toString());
    if (!image.isNull()) {
        this->update();
        return;
    }



#endif
    QString input = "/home/nvidia/Videos/camera.h264";

    // QtConcurrent::run(this, &VideoWidget::play, input);
#if 0
    // 准备 userid usersig, userid 要求每一个实例的 userid 不能相同，否则会冲突。
    std::string user = "nvidia_test";// + std::to_string(i);
    std::string signature = "eJwtzEELgjAYxvHvsmshb87pEjoUUSJSUO2QlzC25MU0c2sk0XfP1OPze*D-Iafk6FjVkJC4DpBpv1GqyuANezYvraqLUdqMr5ZFVtcoSTjzAALg4LPhUe8aG9U5Y8wFgEENln-zfcp4AJSPFcy7OKTtHLYiitdXet9M2kSed4KDSHC5ih5eaQt*2MdZZJ-5gnx-daIzEg__";
    // size_t sig_size = strlen(sig);
    // liteav::GenerateUserSig(sdk_app_id, key.c_str(), user.c_str(), sig, &sig_size);
    // 准备进房参数对象
    unsigned int sdk_app_id = 1400708065;
    liteav::trtc::EnterRoomParams params;
    params.room.user_id = user.c_str();
    params.room.sdk_app_id = sdk_app_id;
    params.room.user_sig = signature.c_str();
    // params.room.str_room_id = "888666";//room_id;
    params.room.room_id = m_room_id_;//room_id;
    // params.scene must be liteav::trtc::TRTC_SCENE_VIDEO_CALL
    params.scene = liteav::trtc::TRTC_SCENE_VIDEO_CALL;
    // params.role must be liteav::trtc::TRTC_ROLE_ANCHOR
    params.role = liteav::trtc::TRTC_ROLE_AUDIENCE;
    // 发送 yuv 数据 params.use_pixel_frame_input must be true
    params.use_pixel_frame_input = false;
    trtc_handle_ptr_->EnterRoom(params);
#endif

}

void VideoWidget::pause()
{

}

void VideoWidget::next()
{

}

void VideoWidget::close()
{
//    qDebug() << "close room";
    if (checkLive)
    {
        timerCheck->stop();
    }

    QTimer::singleShot(1, this, SLOT(clear()));

//    trtc_handle_ptr_->ExitRoom();
}

void VideoWidget::restart()
{
    //qDebug() << TIMEMS << "restart video" << objectName();
    close();
    QTimer::singleShot(10, this, SLOT(open()));
}

void VideoWidget::clear()
{
    image = QImage();

    this->update();
}

void VideoWidget::setSignalRssi(const double value)
{
//    if (flowPanel8)
//    {
//        flowPanel8->setRssi(value);
//    }
}

// 0:center 1:right 2:left
void VideoWidget::setPannelPosition(const SignalRssi::Position pos)
{
//    int width = this->width();
//    int height = this->height();

//    if (this->m_pannel_position != pos)
//    {
//        m_pannel_position = pos;

//        if (m_pannel_position == SignalRssi::POSITION_LEFT_TOP)
//        {
//            // left top
////            flowPanel8->setGeometry(this->width() - 320, height, 320, 50);
//            flowPanel8->setGeometry(-10, height, 320, 50);
//            flowPanel8->setPosition(SignalRssi::POSITION_LEFT_TOP);
//        } else {
//            // right top
//            flowPanel8->setGeometry(-10, height, 320, 50);
//            flowPanel8->setPosition(SignalRssi::POSITION_RIGHT_TOP);
//        }
//    }
//    this->update();
}

void VideoWidget::setTitle(const QString value)
{
//    if (flowPanel8)
//    {
//        flowPanel8->setTitle(value);
//    }
}

void VideoWidget::setViewMode(int mode)
{
    if (mode == 0) {
//        m_viewMode = true;

#if 0
        left_camera_Pannel->setVisible(true);
        right_camera_Pannel->setVisible(true);
#endif
    }
    else if (mode == 1)
    {
//        m_viewMode = false;


        left_camera_Pannel->setVisible(true);
        right_camera_Pannel->setVisible(false);

    } else if (mode == 2){
//        m_viewMode = false;


        left_camera_Pannel->setVisible(false);
        right_camera_Pannel->setVisible(true);
    } else if (mode == 3) {
//        m_viewMode = false;

    }
    m_viewMode = mode;
    this->update();
}

void VideoWidget::setLeftCameraGeometry(int x, int y, int width, int height)
{
    this->m_left_camera_geometry_x = x;
    this->m_left_camera_geometry_y = y;
    this->m_left_camera_geometry_w = width;
    this->m_left_camera_geometry_h = height;
    this->left_camera_Pannel->setGeometry(x, y, width, height);
    this->update();
}

void VideoWidget::setRightCameraGeometry(int x, int y, int width, int height)
{
    this->m_right_camera_geometry_x = x;
    this->m_right_camera_geometry_y = y;
    this->m_right_camera_geometry_w = width;
    this->m_right_camera_geometry_h = height;
    this->right_camera_Pannel->setGeometry(x, y, width, height);
    this->update();
}
