//#pragma execution_character_set("utf-8")

#include "camerawidget.h"
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


//#include <QtConcurrent>
#include <QApplication>

#define EDGE_MARGIN 5
#define min(a,b) ((a)<(b)? (a) :(b))
#define max(a,b) ((a)>(b)? (a) :(b))

CameraWidget::CameraWidget(QWidget *parent) : QWidget(parent)
{

    //设置强焦点
    setFocusPolicy(Qt::StrongFocus);
    //设置支持拖放
    setAcceptDrops(true);

    m_viewMode = false;

    checkLive = true;
    timerCheck = new QTimer(this);
    timerCheck->setInterval(1 * 1000);
    connect(timerCheck, SIGNAL(timeout()), this, SLOT(checkVideo()));

    image = QImage();

    //顶部工具栏,默认隐藏,鼠标移入显示移除隐藏
//    flowPanel0 = new TurningLight(this);
//    flowPanel0->setObjectName("flowPanel0");
//    flowPanel0->setVisible(false);

//    flowPanel1 = new SpeedLimit(this);
//    flowPanel1->setObjectName("flowPanel1");
//    flowPanel1->setVisible(false);

//    flowPanel2 = new VehicleMode(this);
//    flowPanel2->setObjectName("flowPanel2");
//    flowPanel2->setVisible(false);

//    flowPanel3 = new VehicleSpeed(this);
//    flowPanel3->setObjectName("flowPanel3");
//    flowPanel3->setVisible(false);

//    flowPanel4 = new VehicleGear(this);
//    flowPanel4->setObjectName("flowPanel4");
//    flowPanel4->setVisible(false);

//    flowPanel5 = new VehicleSteer(this);
//    flowPanel5->setObjectName("flowPanel5");
//    flowPanel5->setVisible(false);

//    flowPanel6 = new ErrorCode(this);
//    flowPanel6->setObjectName("flowPanel6");
//    flowPanel6->setVisible(false);

//    flowPanel7 = new VehicleBoard(this);
//    flowPanel7->setObjectName("flowPanel7");
//    flowPanel7->setVisible(false);

    flowPanel8 = new SignalRssi(this);
    flowPanel8->setObjectName("flowPanel8");
    flowPanel8->setVisible(true);

//    flowPanel9 = new VehicleState(this);
//    flowPanel9->setObjectName("flowPanel9");
//    flowPanel9->setVisible(false);

//    flowPanel10 = new MessageBox(this);
//    flowPanel10->setObjectName("flowPanel10");
//    flowPanel10->setVisible(false);

//    flowPanel11 = new MessageBar(this);
//    flowPanel11->setObjectName("flowPanel11");
//    flowPanel11->setVisible(false);
    //用布局顶住,左侧弹簧
    QVBoxLayout *vlayout0 = new QVBoxLayout;
//    vlayout0->setSpacing(2);
//    vlayout0->setMargin(0);
//    vlayout0->addStretch();
//    flowPanel0->setLayout(vlayout0);

//    QVBoxLayout *layout1 = new QVBoxLayout;
//    layout1->setSpacing(2);
//    layout1->setMargin(0);
//    layout1->addStretch();
//    flowPanel1->setLayout(layout1);

//    QVBoxLayout *layout2 = new QVBoxLayout;
//    layout2->setSpacing(2);
//    layout2->setMargin(0);
//    layout2->addStretch();
//    flowPanel2->setLayout(layout2);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setSpacing(2);
    hlayout->setMargin(0);
    hlayout->addStretch();
//    hlayout->addWidget(flowPanel0);
//    hlayout->addStretch();
//    hlayout->addWidget(flowPanel1);
//    hlayout->addStretch();
//    hlayout->addWidget(flowPanel2);
//    hlayout->addStretch();
//    hlayout->addWidget(flowPanel3);
//    hlayout->addStretch();
//    hlayout->addWidget(flowPanel4);
//    hlayout->addStretch();
//    hlayout->addWidget(flowPanel5);
//    hlayout->addStretch();
//    hlayout->addWidget(flowPanel6);
//    hlayout->addStretch();
//    hlayout->addWidget(flowPanel7);
//    hlayout->addStretch();
    hlayout->addWidget(flowPanel8);
    hlayout->addStretch();
//    hlayout->addWidget(flowPanel9);
//    hlayout->addStretch();
//    hlayout->addWidget(flowPanel10);
//    hlayout->addStretch();
//    hlayout->addWidget(flowPanel11);
    hlayout->addStretch();
//    this->setLayout(hlayout);

    m_pannel_position = SignalRssi::POSITION_RIGHT_TOP;

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

//    m_codec_hardware_ = H264_HARDWARE_DECODER;

//    m_decoder = H264DecoderFactory::createH264Decoder(this, m_codec_hardware_);
//    m_decoder->init();
//    m_decoder->open();
//    connect(m_decoder, &H264HWDecoder::decodedImageData, this, &CameraWidget::updateImage);

//    m_h264_file_ptr_ = new H264FileProcessor();
//    m_h264_file_ptr_->init();
//    connect(m_h264_file_ptr_, SIGNAL(sendFrameData(const uint8_t *, int)), m_decoder, SLOT(slot_input_frame_data(const uint8_t *, int)));

    // init trtc cloud handle
//    trtc_handle_ptr_ = new liteav::trtc::TctcSendVideo(parent, 3);
//    connect(trtc_handle_ptr_, SIGNAL(sendFrameData(const uint8_t *, int)), m_decoder, SLOT(slot_input_frame_data(const uint8_t *, int)));


    this->initFlowStyle();

    resizeDir = nodir;
    setMouseTracking(true); //开启鼠标追踪

    m_canmove = false;
    this->installEventFilter(this);

    if (checkLive) {
        lastTime = QDateTime::currentDateTime();
        timerCheck->start();
    }

    geometry_x = this->geometry().x();
    geometry_y = this->geometry().y();
    geometry_width = this->geometry().width();
    geometry_height = this->geometry().height();
}

CameraWidget::~CameraWidget()
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

void CameraWidget::resizeEvent(QResizeEvent *)
{
    //重新设置顶部工具栏的位置和宽高,可以自行设置顶部显示或者底部显示
    int height = 8;
//    flowPanel0->setGeometry(borderWidth, borderWidth, this->width() - (borderWidth * 2), height);
    //flowPanel->setGeometry(borderWidth, this->height() - height - borderWidth, this->width() - (borderWidth * 2), height);

//    flowPanel0->setGeometry(10, height, 240, 80);
//    flowPanel1->setGeometry(260, height, 80, 80);
//    flowPanel2->setGeometry(this->width()/2 - 200, height, 60, 80);
//    flowPanel3->setGeometry(this->width()/2 - 120, height, 60, 80);
//    flowPanel4->setGeometry(this->width()/2 - 40, height, 60, 80);
//    flowPanel5->setGeometry(this->width()/2 + 40, height, 60, 80);
//    flowPanel6->setGeometry(this->width()/2 + 120, height, 60, 80);
//    flowPanel7->setGeometry(this->width() - 410, height, 400, 50);
    if (m_pannel_position == SignalRssi::POSITION_RIGHT_TOP)
    {
        flowPanel8->setGeometry(this->width() - 320, height, 320, 50);
    } else {
        flowPanel8->setGeometry(0, height, 320, 50);
    }
//    flowPanel9->setGeometry(this->width() - 410, height +50, 400, 30);
//    flowPanel10->setGeometry(this->width()/2 - 200, height +85, 400, 30);

//    flowPanel11->setGeometry(this->width()/2 - 200, height +200, 400, 100);
}

void CameraWidget::enterEvent(QEvent *)
{
    //这里还可以增加一个判断,是否获取了焦点的才需要显示
    //if (this->hasFocus()) {}

    if (flowEnable) {
        flowPanel8->setVisible(true);
    } else {        
        flowPanel8->setVisible(false);
    }
}

void CameraWidget::leaveEvent(QEvent *)
{
    if (flowEnable) {
        flowPanel8->setVisible(true);
    } else {        
        flowPanel8->setVisible(false);
    }

    QApplication::setOverrideCursor(Qt::IBeamCursor);
}

void CameraWidget::dropEvent(QDropEvent *event)
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

    QWidget::enterEvent(event);
}

void CameraWidget::dragEnterEvent(QDragEnterEvent *event)
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

    QApplication::setOverrideCursor(Qt::IBeamCursor);

    QWidget::leaveEvent(event);
}

void CameraWidget::drawRightLane(QPainter *painter)
{
    int width = this->width();
    int height = this->height();

    QLine lines[] = {
        {width/4 + 320, height - 10, width/4+470 + 10, 450},
        {width*3/4 + 80, height - 10, width*3/4-460 + 60, 450}
    };

    painter->save();
    painter->setPen(QPen(Qt::red, 1));
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

bool CameraWidget::eventFilter(QObject *watched, QEvent *event)
{

    if (m_canmove) {
        static QPoint lastPoint;
        static bool pressed = false;
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->type() == QEvent::MouseButtonPress) {
            if (this->rect().contains(mouseEvent->pos()) && (mouseEvent->button() == Qt::LeftButton)) {
                lastPoint = mouseEvent->pos();
                pressed = true;
            }
        } else if (mouseEvent->type() == QEvent::MouseMove && pressed) {
            int dx = mouseEvent->pos().x() - lastPoint.x();
            int dy = mouseEvent->pos().y() - lastPoint.y();
            this->move(this->x() + dx, this->y() + dy);

        } else if (mouseEvent->type() == QEvent::MouseButtonRelease && pressed) {
            pressed = false;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void CameraWidget::detect_edge(QMouseEvent *event)
{
    int diffLeft = /*event->globalPos().x()*/event->x()/* - frameGeometry().left()*/;      //计算鼠标距离窗口上下左右有多少距离
    int diffRight =/*event->globalPos().x()*/this->geometry().width() - event->x() /*frameGeometry().right()*/;
    int diffTop =  /*event->globalPos().y()*/event->y() /*- frameGeometry().top()*/;
    int diffBottom=/*event->globalPos().y()*/event->y() - this->geometry().height() /*frameGeometry().bottom()*/;

    Qt::CursorShape cursorShape;

    if(diffTop < EDGE_MARGIN && diffTop>=0)//根据 边缘距离 分类改变尺寸的方向
    {
        if(diffLeft < EDGE_MARGIN && diffLeft>=0)
        {
            resizeDir = topLeft;
            cursorShape = Qt::SizeFDiagCursor;
        }
        else if(diffRight > -EDGE_MARGIN && diffRight<=0)
        {
            resizeDir = topRight;
            cursorShape = Qt::SizeBDiagCursor;
        }
        else
        {
            resizeDir = top;
            cursorShape = Qt::SizeVerCursor;
        }
    }
    else if(abs(diffBottom) < EDGE_MARGIN && diffBottom<=0)
    {
        if(diffLeft < EDGE_MARGIN && diffLeft>=0)
        {
            resizeDir = bottomLeft;
            cursorShape = Qt::SizeBDiagCursor;
        }
        else if(diffRight > -EDGE_MARGIN && diffRight<=0)
        {
            resizeDir = bottomRight;
            cursorShape = Qt::SizeFDiagCursor;
        }
        else
        {
            resizeDir = bottom;
            cursorShape = Qt::SizeVerCursor;
        }
    }
    else if(abs(diffLeft) < EDGE_MARGIN)
    {
        resizeDir = left;
        cursorShape = Qt::SizeHorCursor;
    }
    else if(abs(diffRight) < EDGE_MARGIN)
    {
        resizeDir = right;
        cursorShape = Qt::SizeHorCursor;
    }
    else
    {
        resizeDir = nodir;
        cursorShape = Qt::ArrowCursor;
    }

    QApplication::setOverrideCursor(cursorShape);
}


void CameraWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (event->buttons() & Qt::LeftButton) //如果左键是按下的
    {
        if(resizeDir == nodir)
        {                             //如果鼠标不是放在边缘那么说明这是在拖动窗口
            move(event->globalPos() - dragPosition);
        }
        else
        {
            int ptop,pbottom,pleft,pright;                   //窗口上下左右的值
//            ptop = frameGeometry().top();
//            pbottom = frameGeometry().bottom();
//            pleft = frameGeometry().left();
//            pright = frameGeometry().right();

            ptop = this->geometry().top();
            pbottom = this->geometry().bottom();
            pleft = this->geometry().left();
            pright = this->geometry().right();

            if(resizeDir & top)
            {                               //检测更改尺寸方向中包含的上下左右分量
                if(height() == minimumHeight())
                {
                    ptop = min(event->globalY(),ptop);
                }
                else if(height() == maximumHeight())
                {
                    ptop = max(event->globalY(),ptop);
                }
                else
                {
                    ptop = event->globalY();
                }
            }
            else if(resizeDir & bottom)
            {
                if(height() == minimumHeight()){
                    pbottom = max(event->globalY(),ptop);
                }
                else if(height() == maximumHeight())
                {
                    pbottom = min(event->globalY(),ptop);
                }
                else
                {
                    pbottom = event->globalY();
                }
            }

            if(resizeDir & left)
            {                        //检测左右分量
                if(width() == minimumWidth())
                {
                    pleft = min(event->globalX(),pleft);
                }
                else if(width() == maximumWidth())
                {
                    pleft = max(event->globalX(),pleft);
                }
                else
                {
                    pleft = event->globalX();
                }
            }
            else if(resizeDir & right){
                if(width() == minimumWidth())
                {
                    pright = max(event->globalX(),pright);
                }
                else if(width() == maximumWidth())
                {
                    pright = min(event->globalX(),pright);
                }
                else
                {
                    pright = event->globalX();
                }
            }
            setGeometry(QRect(QPoint(pleft,ptop),QPoint(pright,pbottom)));
        }
    }
    else
    {
        detect_edge(event);   //当不拖动窗口、不改变窗口大小尺寸的时候  检测鼠标边缘
    }


}

void CameraWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)  //每当按下鼠标左键就记录一下位置
    {
        dragPosition = event->globalPos() - frameGeometry().topLeft();  //获得鼠标按键位置相对窗口左上面的位置
    }
}

void CameraWidget::mouseReleaseEvent(QMouseEvent * event)
{
    if(resizeDir != nodir)//还原鼠标样式
    {
        detect_edge(event);
        this->geometry_x = this->geometry().x();
        this->geometry_y = this->geometry().y();
        this->geometry_width = this->geometry().width();
        this->geometry_height = this->geometry().height();
    }
}

void CameraWidget::paintEvent(QPaintEvent *)
{
//    int width = this->width();
//    int height = this->height();

    //如果不需要绘制
    if (!drawImage) {
        return;
    }

    QStyleOption opt;
    QPainter painter(this);
    //绘制准备工作,启用反锯齿
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
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
    if (m_viewMode)
    {
        drawRightLane(&painter);
    }
    // qss style
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void CameraWidget::drawBorder(QPainter *painter)
{
    painter->save();
    QPen pen;
    pen.setWidth(borderWidth);
    pen.setColor(hasFocus() ? focusColor : borderColor);
    painter->setPen(pen);
    painter->drawRect(rect());
    painter->restore();
}

void CameraWidget::drawBg(QPainter *painter)
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

void CameraWidget::drawImg(QPainter *painter, QImage img)
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

void CameraWidget::drawOSD(QPainter *painter,
                          int osdFontSize,
                          const QString &osdText,
                          const QColor &osdColor,
                          const QImage &osdImage,
                          const CameraWidget::OSDFormat &osdFormat,
                          const CameraWidget::OSDPosition &osdPosition)
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

unsigned int CameraWidget::getRoomId() const
{
    return this->m_room_id_;
}

QImage CameraWidget::getImage() const
{
    return this->image;
}

QDateTime CameraWidget::getLastTime() const
{
    return QDateTime::currentDateTime();
}

QString CameraWidget::getUrl() const
{
    return this->property("url").toString();
}

bool CameraWidget::getCopyImage() const
{
    return this->copyImage;
}

bool CameraWidget::getCheckLive() const
{
    return this->checkLive;
}

bool CameraWidget::getDrawImage() const
{
    return this->drawImage;
}

bool CameraWidget::getFillImage() const
{
    return this->fillImage;
}

bool CameraWidget::getFlowEnable() const
{
    return this->flowEnable;
}

QColor CameraWidget::getFlowBgColor() const
{
    return this->flowBgColor;
}

QColor CameraWidget::getFlowPressColor() const
{
    return this->flowPressColor;
}

int CameraWidget::getTimeout() const
{
    return this->timeout;
}

int CameraWidget::getBorderWidth() const
{
    return this->borderWidth;
}

QColor CameraWidget::getBorderColor() const
{
    return this->borderColor;
}

QColor CameraWidget::getFocusColor() const
{
    return this->focusColor;
}

QString CameraWidget::getBgText() const
{
    return this->bgText;
}

QImage CameraWidget::getBgImage() const
{
    return this->bgImage;
}

bool CameraWidget::getOSD1Visible() const
{
    return this->osd1Visible;
}

int CameraWidget::getOSD1FontSize() const
{
    return this->osd1FontSize;
}

QString CameraWidget::getOSD1Text() const
{
    return this->osd1Text;
}

QColor CameraWidget::getOSD1Color() const
{
    return this->osd1Color;
}

QImage CameraWidget::getOSD1Image() const
{
    return this->osd1Image;
}

CameraWidget::OSDFormat CameraWidget::getOSD1Format() const
{
    return this->osd1Format;
}

CameraWidget::OSDPosition CameraWidget::getOSD1Position() const
{
    return this->osd1Position;
}

bool CameraWidget::getOSD2Visible() const
{
    return this->osd2Visible;
}

int CameraWidget::getOSD2FontSize() const
{
    return this->osd2FontSize;
}

QString CameraWidget::getOSD2Text() const
{
    return this->osd2Text;
}

QColor CameraWidget::getOSD2Color() const
{
    return this->osd2Color;
}

QImage CameraWidget::getOSD2Image() const
{
    return this->osd2Image;
}

CameraWidget::OSDFormat CameraWidget::getOSD2Format() const
{
    return this->osd2Format;
}

CameraWidget::OSDPosition CameraWidget::getOSD2Position() const
{
    return this->osd2Position;
}

QSize CameraWidget::sizeHint() const
{
    return QSize(500, 350);
}

QSize CameraWidget::minimumSizeHint() const
{
    return QSize(50, 35);
}

void CameraWidget::initFlowStyle()
{
    //设置样式以便区分,可以自行更改样式,也可以不用样式
    QStringList qss;
    QString rgba = QString("rgba(%1,%2,%3,130)").arg(flowBgColor.red()).arg(flowBgColor.green()).arg(flowBgColor.blue());
//    qss.append(QString("#flowPanel0{background:%1;border:none;}").arg(rgba));
//    qss.append(QString("#flowPanel1{background:%1;border:none;}").arg(rgba));
//    qss.append(QString("#flowPanel2{background:%1;border:none;}").arg(rgba));
//    qss.append(QString("#flowPanel3{background:%1;border:none;}").arg(rgba));
//    qss.append(QString("#flowPanel4{background:%1;border:none;}").arg(rgba));
//    qss.append(QString("#flowPanel5{background:%1;border:none;}").arg(rgba));
//    qss.append(QString("#flowPanel6{background:%1;border:none;}").arg(rgba));
//    qss.append(QString("#flowPanel7{background:%1;border:none;}").arg(rgba));
    qss.append(QString("#flowPanel8{background:%1;border:none;}").arg(rgba));
//    qss.append(QString("#flowPanel9{background:%1;border:none;}").arg(rgba));
//    qss.append(QString("#flowPanel10{background:%1;border:none;}").arg(rgba));
    qss.append(QString("QPushButton{border:none;padding:0px;background:rgba(0,0,0,0);}"));
    qss.append(QString("QPushButton:pressed{color:%1;}").arg(flowPressColor.name()));
//    flowPanel0->setStyleSheet(qss.join(""));
//    flowPanel1->setStyleSheet(qss.join(""));
//    flowPanel2->setStyleSheet(qss.join(""));
//    flowPanel3->setStyleSheet(qss.join(""));
//    flowPanel4->setStyleSheet(qss.join(""));
//    flowPanel5->setStyleSheet(qss.join(""));
//    flowPanel6->setStyleSheet(qss.join(""));
//    flowPanel7->setStyleSheet(qss.join(""));
    flowPanel8->setStyleSheet(qss.join(""));
//    flowPanel9->setStyleSheet(qss.join(""));
//    flowPanel10->setStyleSheet(qss.join(""));
}

void CameraWidget::updateImage(double timestamp, const QImage &image)
{
//    QDateTime datetimestamp = QDateTime::currentDateTime();
//    qint64 current_timestamp = datetimestamp.currentMSecsSinceEpoch();
//    qint64 ts = current_timestamp - static_cast<qint64>(timestamp);
    osd1Text = QString("%1ms").arg(timestamp);
    osd1Format = OSDFormat_Text;
    //拷贝图片有个好处,当处理器比较差的时候,图片不会产生断层,缺点是占用时间
    //默认QImage类型是浅拷贝,可能正在绘制的时候,那边已经更改了图片的上部分数据
    if (m_viewMode == 1)
    {
        // flowPanel7->setVideoDelay(timestamp);
    } else {
        flowPanel8->setRssi(timestamp);
    }

    this->image = copyImage ? image.copy() : image;
    lastTime = QDateTime::currentDateTime();
    this->update();
}

void CameraWidget::checkVideo()
{
    QImage image(1920, 1080, QImage::Format_RGB888);
    QDateTime now = QDateTime::currentDateTime();

    for (int i=0; i<1080; i++)
    {
        for (int j=0; j<1920; j++)
        {
            image.setPixel(j, i, 0xFF25496A);
        }
    }

    int sec = static_cast<int>(lastTime.secsTo(now));
    if ((sec >= timeout) || (sec < 0))
    {
        // restart();
        updateImage(0.0, image);
    }
}

void CameraWidget::btnClicked()
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

void CameraWidget::setRoomId(unsigned int roomid)
{
    this->m_room_id_ = roomid;
}

void CameraWidget::setInterval(int interval)
{

}

void CameraWidget::setSleepTime(int sleepTime)
{

}

void CameraWidget::setCheckTime(int checkTime)
{

}

void CameraWidget::setCheckConn(bool checkConn)
{

}

void CameraWidget::setUrl(const QString &url)
{
    this->setProperty("url", url);
}

void CameraWidget::setHardware(const QString &hardware)
{

}

void CameraWidget::setSaveFile(bool saveFile)
{

}

void CameraWidget::setSaveInterval(int saveInterval)
{

}

void CameraWidget::setSavePath(const QString &savePath)
{
    //如果目录不存在则新建
    QDir dir(savePath);
    if (!dir.exists()) {
        dir.mkdir(savePath);
    }


}

void CameraWidget::setFileName(const QString &fileName)
{

}

void CameraWidget::setCopyImage(bool copyImage)
{
    this->copyImage = copyImage;
}

void CameraWidget::setCheckLive(bool checkLive)
{
    this->checkLive = checkLive;
}

void CameraWidget::setDrawImage(bool drawImage)
{
    this->drawImage = drawImage;
}

void CameraWidget::setFillImage(bool fillImage)
{
    this->fillImage = fillImage;
}

void CameraWidget::setFlowEnable(bool flowEnable)
{
    this->flowEnable = flowEnable;
}

void CameraWidget::setFlowBgColor(const QColor &flowBgColor)
{
    if (this->flowBgColor != flowBgColor) {
        this->flowBgColor = flowBgColor;
        this->initFlowStyle();
    }
}

void CameraWidget::setFlowPressColor(const QColor &flowPressColor)
{
    if (this->flowPressColor != flowPressColor) {
        this->flowPressColor = flowPressColor;
        this->initFlowStyle();
    }
}

void CameraWidget::setTimeout(int timeout)
{
    this->timeout = timeout;
}

void CameraWidget::setBorderWidth(int borderWidth)
{
    this->borderWidth = borderWidth;
}

void CameraWidget::setBorderColor(const QColor &borderColor)
{
    this->borderColor = borderColor;
}

void CameraWidget::setFocusColor(const QColor &focusColor)
{
    this->focusColor = focusColor;
}

void CameraWidget::setBgText(const QString &bgText)
{
    this->bgText = bgText;
}

void CameraWidget::setBgImage(const QImage &bgImage)
{
    this->bgImage = bgImage;
}

void CameraWidget::setOSD1Visible(bool osdVisible)
{
    this->osd1Visible = osdVisible;
}

void CameraWidget::setOSD1FontSize(int osdFontSize)
{
    this->osd1FontSize = osdFontSize;
}

void CameraWidget::setOSD1Text(const QString &osdText)
{
    this->osd1Text = osdText;
}

void CameraWidget::setOSD1Color(const QColor &osdColor)
{
    this->osd1Color = osdColor;
}

void CameraWidget::setOSD1Image(const QImage &osdImage)
{
    this->osd1Image = osdImage;
}

void CameraWidget::setOSD1Format(const CameraWidget::OSDFormat &osdFormat)
{
    this->osd1Format = osdFormat;
}

void CameraWidget::setOSD1Position(const CameraWidget::OSDPosition &osdPosition)
{
    this->osd1Position = osdPosition;
}

void CameraWidget::setOSD2Visible(bool osdVisible)
{
    this->osd2Visible = osdVisible;
}

void CameraWidget::setOSD2FontSize(int osdFontSize)
{
    this->osd2FontSize = osdFontSize;
}

void CameraWidget::setOSD2Text(const QString &osdText)
{
    this->osd2Text = osdText;
}

void CameraWidget::setOSD2Color(const QColor &osdColor)
{
    this->osd2Color = osdColor;
}

void CameraWidget::setOSD2Image(const QImage &osdImage)
{
    this->osd2Image = osdImage;
}

void CameraWidget::setOSD2Format(const CameraWidget::OSDFormat &osdFormat)
{
    this->osd2Format = osdFormat;
}

void CameraWidget::setOSD2Position(const CameraWidget::OSDPosition &osdPosition)
{
    this->osd2Position = osdPosition;
}

void CameraWidget::play(const QString &input)
{
//    m_h264_file_ptr_->open(input);
//    m_h264_file_ptr_->processFile();
}

void CameraWidget::open()
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

    // QtConcurrent::run(this, &CameraWidget::play, input);
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

void CameraWidget::pause()
{

}

void CameraWidget::next()
{

}

void CameraWidget::close()
{
//    qDebug() << "close room";
    if (checkLive)
    {
        timerCheck->stop();
    }

    QTimer::singleShot(1, this, SLOT(clear()));

//    trtc_handle_ptr_->ExitRoom();
}

void CameraWidget::restart()
{
    //qDebug() << TIMEMS << "restart video" << objectName();
    close();
    QTimer::singleShot(10, this, SLOT(open()));
}

void CameraWidget::clear()
{
    image = QImage();

    this->update();
}

void CameraWidget::setViewMode(int mode)
{
    if (mode == 1)
    {
        m_viewMode = true;

        flowPanel8->setVisible(false);

    } else {
        m_viewMode = false;

        flowPanel8->setVisible(true);

    }

    this->update();
}

// 0:unactive 1:turn left active 2:turn right active 3:emergency active
void CameraWidget::seTurningLightState(int state)
{
//    if (flowPanel0)
//    {
//        switch (state)
//        {
//        case 1:
//            flowPanel0->setTurningLightLeftState(true);
//            flowPanel0->setTurningLightRightState(false);
//            break;
//        case 2:
//            flowPanel0->setTurningLightLeftState(false);
//            flowPanel0->setTurningLightRightState(true);
//            break;
//        default:
//            flowPanel0->setTurningLightLeftState(false);
//            flowPanel0->setTurningLightRightState(false);
//            break;
//        }
//    }
}



/*
    SYSTEMSTATE_DEFAULT = 0,
    SYSTEMSTATE_DISABLE = 1,
    SYSTEMSTATE_READY   = 2,
    SYSTEMSTATE_AUTO    = 3,
    SYSTEMSTATE_TAKEOVER= 4,
    SYSTEMSTATE_ERROR   = 5,
    SYSTEMSTATE_SELFCHECKING = 6,
    SYSTEMSTATE_STARTING= 7,
    SYSTEMSTATE_MRC     = 8,
    SYSTEMSTATE_END = 11,
    SYSTEMSTATE_REMOTE = 12,
    SYSTEMSTATE_REMOTE_ERROR = 13,
    SYSTEMSTATE_MRC1     = 100,
    SYSTEMSTATE_MRC3     = 101,
    SYSTEMSTATE_MRC2     = 102,
*/


void CameraWidget::setTitle(const QString value)
{
    if (flowPanel8)
    {
        flowPanel8->setTitle(value);
    }
}

// 0:center 1:right 2:left
void CameraWidget::setPannelPosition(const SignalRssi::Position pos)
{
    int width = this->width();
    int height = this->height();

    if (this->m_pannel_position != pos)
    {
        m_pannel_position = pos;

        if (m_pannel_position == SignalRssi::POSITION_LEFT_TOP)
        {
            // left top
//            flowPanel8->setGeometry(this->width() - 320, height, 320, 50);
            flowPanel8->setGeometry(-10, height, 320, 50);
            flowPanel8->setPosition(SignalRssi::POSITION_LEFT_TOP);
        } else {
            // right top
            flowPanel8->setGeometry(-10, height, 320, 50);
            flowPanel8->setPosition(SignalRssi::POSITION_RIGHT_TOP);
        }
    }
    this->update();
}

