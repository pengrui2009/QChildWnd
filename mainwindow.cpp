#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFile>
#include <QSettings>
#include <unordered_map>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<int8_t>("int8_t");

    this->initStyle();

    this->initForm();

    this->initData();

    this->initView();

    layout_str = "LeftMainWindow";
    this->changeLayout("LeftMainWindow", true);
}

MainWindow::~MainWindow()
{
    closeAll();

    delete ui;
}

void MainWindow::closeAll()
{
//    qDebug() << "closeAll";
    saveLayout(layout_str, 2);
//    QUIHelper::sleep(100);
    exit(0);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    closeAll();
}

void MainWindow::drawBorder(QPainter *painter)
{
    painter->save();
    QPen pen;
    pen.setWidth(m_border_width_);
    pen.setColor(hasFocus() ? m_focus_color_ : m_border_color_);
    painter->setPen(pen);
//    painter->setBrush(QColor(20, 34, 45));
    painter->drawRect(rect());
    painter->restore();
}

void MainWindow::drawBackgroud(QPainter *painter)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    painter->save();
#if 0
    QRadialGradient radialGradient(QPointF(0, 0), side/4);
//    QRadialGradient radialGradient(0.5, 0.5, 1.0, 0.5, 0.5);
//    radialGradient.setRadius(1.0);
//    radialGradient.setColorAt(0.0, QColor(55,107,154));
//    radialGradient.setColorAt(1.0, QColor(0,0,0));
    QGradientStops gradientstops;
    gradientstops.push_back({0.0, QColor(55,107,154)});
    gradientstops.push_back({1.0, QColor(0,0,0)});
    radialGradient.setStops(gradientstops);
    painter->setBrush(radialGradient);
    painter->drawEllipse(-width, -height, width * 2, height * 2);
#endif
    painter->setBrush(Qt::black);
    painter->drawEllipse(-width, -height, width * 2, height * 2);
    painter->restore();
}

void MainWindow::paintEvent(QPaintEvent *)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    QPainter painter(this);
//    painter.setRenderHints(QPainter::Antialiasing);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.translate(width / 2, height / 2);
    painter.scale(side / 200.0, side / 200.0);

    drawBackgroud(&painter);
}


void MainWindow::changeLayout(const QString &layout, bool init)
{
    //首次加载不需要比较是否和配置文件一样
    bool needLoad = init;
    if (!init && layout_str!= layout) {
        needLoad = true;
        //先保存原有布局
        saveLayout(layout_str, 2);
    }

    if (needLoad) {
        layout_str = layout;
//        App::Layout = layout;
//        App::writeConfig();
//        this->clearWidget();
//        this->initWidget();
        this->initLayout(layout_str);

        //全屏+QWebEngineView控件一起会产生右键菜单无法弹出的BUG,需要上移一个像素
//        QRect rect = qApp->desktop()->screenGeometry();
#if 1
//        rect.setY(-1);
//        rect.setHeight(rect.height());
#else
        rect.setX(30);
        rect.setY(40);
        rect.setWidth(1370);
        rect.setHeight(795);
#endif
//        this->setGeometry(rect);
    }
}

void MainWindow::initLayout(const QString &layout)
{
    QString file = QString("%1/%2.ini").arg(layoutPath).arg(layout);
    QSettings set(file, QSettings::IniFormat);
    set.beginGroup("MainWindow");
    restoreState(set.value("State").toByteArray());

    m_camera_geometry_x = set.value("Camera_X").toInt();
    m_camera_geometry_y = set.value("Camera_Y").toInt();
    m_camera_geometry_w = set.value("Camera_W").toInt();
    m_camera_geometry_h = set.value("Camera_H").toInt();
    set.endGroup();

    if ((m_camera_geometry_w == 0) ||(m_camera_geometry_h == 0))
    {

        m_camera_geometry_w = 300;
        m_camera_geometry_h = 200;
    }
    ui->left_camera_widget->setLeftCameraGeometry(m_camera_geometry_x, m_camera_geometry_y, m_camera_geometry_w, m_camera_geometry_h);
}

//type: 0-新建布局 1-恢复布局 2-保存布局 3-布局另存
void MainWindow::saveLayout(const QString &layout, int type)
{
    int camera_geometry_x = this->ui->left_camera_widget->getLeftCameraGeometryX();
    int camera_geometry_y = this->ui->left_camera_widget->getLeftCameraGeometryY();
    int camera_geometry_w = this->ui->left_camera_widget->getLeftCameraGeometryWidth();
    int camera_geometry_h = this->ui->left_camera_widget->getLeftCameraGeometryHeight();
    //如果为空则表示是恢复布局
    if (type == 0) {
        layout_str = layout;
        this->changeLayout(layout_str, true);
        return;
    } else if (type == 1) {
        this->changeLayout(layout_str, true);
        return;
    }

    QString file = QString("%1/%2.ini").arg(layoutPath).arg(layout);
    QSettings set(file, QSettings::IniFormat);
    qDebug() << "file:" << file << " saved!";
    set.beginGroup("MainWindow");
    set.setValue("State", saveState());
    set.setValue("Camera_X", camera_geometry_x);
    set.setValue("Camera_Y", camera_geometry_y);
    set.setValue("Camera_W", camera_geometry_w);
    set.setValue("Camera_H", camera_geometry_h);
    set.endGroup();

    layout_str = layout;
//    App::writeConfig();
}

void MainWindow::initForm()
{

}

void MainWindow::initStyle()
{
    //加载样式表
    QFile file(":/qss/lightblue.css");
    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(QColor(paletteColor)));
        qApp->setStyleSheet(qss);
        file.close();
    }
}

void MainWindow::initData()
{
    m_border_width_ = 2;
    m_focus_color_ = QColor(0, 255, 0);
    m_border_color_ = QColor(11, 17, 20);
    m_lane_start_offset_ = -10;

    m_bg_image_ = QImage(":/image/logo.png");

    m_vehicle_runing_timer_ = new QTimer();
    m_vehicle_runing_timer_->setInterval(100);
    connect(m_vehicle_runing_timer_, SIGNAL(timeout()), this, SLOT(slot_timeout()));
    m_vehicle_runing_timer_->start();

    QTimer::singleShot(1000, this, SLOT(slot_update_timerout()));


    ui->left_camera_widget->setRoomId(30520);
    ui->left_camera_widget->setPannelPosition(SignalRssi::POSITION_RIGHT_TOP);
    ui->left_camera_widget->setViewMode(1);
    ui->left_camera_widget->setOSD1Visible(true);
    ui->left_camera_widget->setOSD1Position(VideoWidget::OSDPosition_Left_Top);

    ui->left_camera_widget->setOSD2Text("前");
    ui->left_camera_widget->setTitle("左后视角");
    ui->left_camera_widget->setOSD2Visible(true);
    ui->left_camera_widget->setOSD2Position(VideoWidget::OSDPosition_Left_Bottom);
#if 0
    ui->rear_camera_widget->setOSD1Visible(true);
    ui->rear_camera_widget->setOSD1Position(VideoWidget::OSDPosition_Left_Top);

    ui->rear_camera_widget->setRoomId(30521);
    ui->rear_camera_widget->setOSD2Text("内");
    ui->rear_camera_widget->setOSD2Visible(true);
    ui->rear_camera_widget->setOSD2Position(VideoWidget::OSDPosition_Left_Bottom);

    ui->left_camera_widget->setOSD1Visible(true);
    ui->left_camera_widget->setOSD1Position(VideoWidget::OSDPosition_Left_Top);

    ui->left_camera_widget->setRoomId(30522);
    ui->left_camera_widget->setOSD2Text("左");
    ui->left_camera_widget->setOSD2Visible(true);
    ui->left_camera_widget->setOSD2Position(VideoWidget::OSDPosition_Left_Bottom);

    ui->right_camera_widget->setOSD1Visible(true);
    ui->right_camera_widget->setOSD1Position(VideoWidget::OSDPosition_Right_Top);

    ui->left_camera_widget->setRoomId(30523);
    ui->right_camera_widget->setOSD2Text("右");
    ui->right_camera_widget->setOSD2Visible(true);
    ui->right_camera_widget->setOSD2Position(VideoWidget::OSDPosition_Right_Bottom);
#endif
//    ui->left_camera_widget->setOSD2Visible(true);
//    ui->left_camera_widget->setOSD2Position(VideoWidget::OSDPosition::OSDPosition_Left_Top);

//    ui->comboBox_vehicleid->addItem("Foton-3051", QVariant(3051));
//    ui->comboBox_vehicleid->addItem("Foton-3052", QVariant(3052));
//    ui->comboBox_vehicleid->addItem("Foton-3053", QVariant(3053));
//    ui->comboBox_vehicleid->addItem("Foton-3054", QVariant(3054));
//    ui->comboBox_vehicleid->addItem("Foton-3055", QVariant(3055));
//    ui->comboBox_vehicleid->addItem("Foton-3056", QVariant(3056));

    connect(this, SIGNAL(updateImage(int, double, const QImage &)), ui->left_camera_widget, SLOT(updateImage(int, double, const QImage &)));

    std::string log_path = "./log";
    std::unordered_map<std::string, std::string> data_source = {
        {"left", "pzsp://remote.video.com:7788/live/ld/trans/test/mlinkm/Test_0006_0?ndselect=2&linkmode=8&fstusrd=1&us=1&rcvmd=2"},
        {"right", "pzsp://remote.video.com::7788/live/ld/trans/test/mlinkm/Test_0006_1?ndselect=2&linkmode=8&fstusrd=1&us=1&rcvmd=2"},
        {"front", "pzsp://remote.video.com::7788/live/ld/trans/test/mlinkm/Test_0006_2?ndselect=2&linkmode=8&fstusrd=1&us=1&rcvmd=2"},
        {"inner", "pzsp://remote.video.com::7788/live/ld/trans/test/mlinkm/Test_0006_3?ndselect=2&linkmode=8&fstusrd=1&us=1&rcvmd=2"}
    };

     // media_player_handler_ = std::make_shared<PIMediaPlayer>(log_path, data_source["left"], "video");

    int index = 0;
    run_status_.store(true, std::memory_order_release);


}

void MainWindow::slot_update_timerout()
{
    QString str_time = "";

    QDateTime datetime = QDateTime::currentDateTime();
    str_time = datetime.toString("YYYY-MM-dd HH:mm:ss");

    QTimer::singleShot(1000, this, SLOT(slot_update_timerout()));
}

void MainWindow::slot_timeout()
{
    if (m_lane_start_offset_ < -5)
    {
        m_lane_start_offset_++;
    } else {
        m_lane_start_offset_ = -10;
    }
    this->update();
}

void MainWindow::initView()
{
//    ui->label_datetime->setAlignment(Qt::AlignCenter);
//    ui->label_softwareversion->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
//    ui->label_vehicleid->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
#if 0
    ui->label_ipaddress->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    ui->label_datarate->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    ui->label_satelite->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    ui->label_rssi->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    ui->label_netdelay->setAlignment(Qt::AlignCenter);

    ui->left_gauge_widget->setMinValue(0);
    ui->left_gauge_widget->setMaxValue(7000);
    ui->left_gauge_widget->setScaleMinor(5);
    ui->left_gauge_widget->setScaleMajor(7);
//    ui->left_gauge_widget->setPointerStyle(GaugeCar::PointerStyle_IndicatorR);
    ui->left_gauge_widget->setPrecision(0);
    ui->left_gauge_widget->setCircleWidth(22);
    ui->left_gauge_widget->setAnimation(false);
    ui->left_gauge_widget->setShowOverlay(false);
    ui->left_gauge_widget->setUnitName("r/min");
    ui->left_gauge_widget->setValue(3000.0);

    ui->right_gauge_widget->setMinValue(0);
    ui->right_gauge_widget->setMaxValue(140);
    ui->right_gauge_widget->setScaleMinor(10);
    ui->right_gauge_widget->setScaleMajor(7);
//    ui->right_gauge_widget->setCircleWidth(20);
    ui->right_gauge_widget->setShowOverlay(false);
    ui->right_gauge_widget->setValue(25);
#endif
    ui->left_camera_widget->setFlowEnable(true);
    ui->left_camera_widget->setFocusPolicy(Qt::StrongFocus);
//    ui->front_camera_widget->open();
#if 0
    ui->rear_camera_widget->setFlowEnable(true);
    ui->rear_camera_widget->setFocusPolicy(Qt::StrongFocus);

    ui->left_camera_widget->setFlowEnable(true);
    ui->left_camera_widget->setFocusPolicy(Qt::StrongFocus);

    ui->right_camera_widget->setFlowEnable(true);
    ui->right_camera_widget->setFocusPolicy(Qt::StrongFocus);
#endif
}

void MainWindow::slot_go_remotedrive(QString vehicleid, bool state)
{
    qDebug() << "LeftMainWindow slot_go_remotedrive vehicleid:" << vehicleid << " state:" << state;

    if (state)
    {
        char *data_source = "pzsp://remote.video.com::7788/live/ld/trans/test/mlinkm/Test_0006_0?ndselect=2&linkmode=8&fstusrd=1&us=1&rcvmd=2";
        // media_player_handler_->start(data_source);
    } else {
        // media_player_handler_->stop();
    }

}
