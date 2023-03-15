#include "signalrssi.h"

#include <QPainter>
#include <QDebug>
#include <QDateTime>
#include <qmath.h>


SignalRssi::SignalRssi(QWidget *parent, Position pos) : QWidget(parent), m_position_(pos)
{
    m_background_color_ = QColor(55,107,154, 30);
    m_rssi_ = 0.0;

    m_timer_ptr_ = new QTimer(this);
    m_timer_ptr_->setInterval(100);
    connect(m_timer_ptr_, SIGNAL(timeout()), this, SLOT(updateValue()));
}

SignalRssi::~SignalRssi()
{
    if (m_timer_ptr_->isActive())
    {
        m_timer_ptr_->stop();
    }
    delete m_timer_ptr_;
}

void SignalRssi::resizeEvent(QResizeEvent *)
{
    this->repaint();
}

void SignalRssi::paintEvent(QPaintEvent *)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    //绘制准备工作,启用反锯齿,平移坐标轴中心,等比例缩放
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.translate(width / 2, height / 2);
//    painter.scale(side / 200.0, side / 200.0);

    drawBackground(&painter);

    drawtitle(&painter);

    drawrssi(&painter);

    drawSystemTime(&painter);
}

void SignalRssi::drawBackground(QPainter *painter)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    painter->save();

    painter->setBrush(m_background_color_);
    painter->drawRect(-width, -height, width * 2, height * 2);

    painter->restore();
}

void SignalRssi::drawtitle(QPainter *painter)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);
    QString title_str = "";

    QFont font;
    painter->save();
    font.setPixelSize(15);
    painter->setFont(font);
    painter->setPen(QColor(51, 223, 35));

    title_str = m_title_;
    if (m_position_ == POSITION_RIGHT_TOP)
    {
        painter->drawText(QRect(-width/2+5, -height/2, 60, 40), Qt::AlignCenter, title_str);
    } else if (m_position_ == POSITION_LEFT_TOP) {
        painter->drawText(QRect(width/2-65, -height/2, 60, 40), Qt::AlignCenter, title_str);
    } else {
        painter->drawText(QRect(-width/2+5, -height/2, 60, 40), Qt::AlignCenter, title_str);
    }
    painter->restore();
}

void SignalRssi::drawrssi(QPainter *painter)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);
    int draw_signal_strenght_value = 5;

    QString video_timecost_str = "";

    if (m_position_ == POSITION_RIGHT_TOP)
    {
        QLine lines[] = {
            {-width/2 + 80, 0, -width/2 + 80, - 5},
            {-width/2 + 90, 0, -width/2 + 90, - 10},
            {-width/2 + 100, 0, -width/2 + 100, - 15},
            {-width/2 + 110, 0, -width/2 + 110, - 20},
            {-width/2 + 120, 0, -width/2 + 120, - 25},
        };

        if ((m_rssi_ > 0) && (m_rssi_ < 100.0))
        {
            draw_signal_strenght_value = 5;
        } else if ((m_rssi_ > 100.0) && (m_rssi_ < 200.0)) {
            draw_signal_strenght_value = 4;
        } else if ((m_rssi_ > 200.0) && (m_rssi_ < 500.0)) {
            draw_signal_strenght_value = 3;
        } else if (m_rssi_ > 1000.0) {
            draw_signal_strenght_value = 1;
        } else {
            draw_signal_strenght_value = 0;
        }

        painter->save();
        painter->setPen(QPen(Qt::white,4));
        painter->drawLines(lines, draw_signal_strenght_value);
        painter->restore();

        painter->save();
        painter->setPen(QPen(Qt::gray,4));
        painter->drawLines(&lines[draw_signal_strenght_value], ((sizeof(lines)/ sizeof(lines[0])) - draw_signal_strenght_value));
        painter->restore();


        QFont font;
        painter->save();
        font.setPixelSize(15);
        painter->setFont(font);
        painter->setPen(QColor(51, 223, 35));
        if (m_rssi_ != 0)
        {
            video_timecost_str = QString("%1ms").arg(m_rssi_, 0, 'f', 0);
        } else {
            video_timecost_str = QString("N/A");
        }
        painter->drawText(QRect(-width/2+80, 0, 60, 40), Qt::AlignLeft, video_timecost_str);
        painter->restore();
    } else {
        QLine lines[] = {
            {width/2 - 80, 0, width/2 - 80, - 5},
            {width/2 - 90, 0, width/2 - 90, - 10},
            {width/2 - 100, 0, width/2 - 100, - 15},
            {width/2 - 110, 0, width/2 - 110, - 20},
            {width/2 - 120, 0, width/2 - 120, - 25},
        };

        if ((m_rssi_ > 0) && (m_rssi_ < 100.0))
        {
            draw_signal_strenght_value = 5;
        } else if ((m_rssi_ > 100.0) && (m_rssi_ < 200.0)) {
            draw_signal_strenght_value = 4;
        } else if ((m_rssi_ > 200.0) && (m_rssi_ < 500.0)) {
            draw_signal_strenght_value = 3;
        } else if (m_rssi_ > 1000.0) {
            draw_signal_strenght_value = 1;
        } else {
            draw_signal_strenght_value = 0;
        }


        painter->save();
        painter->setPen(QPen(Qt::white,4));
        painter->drawLines(lines,  draw_signal_strenght_value);
        painter->restore();

        painter->save();
        painter->setPen(QPen(Qt::gray,4));
        painter->drawLines(&lines[draw_signal_strenght_value], ((sizeof(lines)/sizeof(lines[0])) - draw_signal_strenght_value));
        painter->restore();



        QFont font;
        painter->save();
        font.setPixelSize(15);
        painter->setFont(font);
        painter->setPen(QColor(51, 223, 35));
        if (m_rssi_ != 0)
        {
            video_timecost_str = QString("%1ms").arg(m_rssi_, 0, 'f', 0);
        } else {
            video_timecost_str = QString("N/A");
        }
        painter->drawText(QRect(width/2-120, 0, 60, 40), Qt::AlignLeft, video_timecost_str);
        painter->restore();
    }
}

void SignalRssi::drawSystemTime(QPainter *painter)
{
    int width = this->width();
    int height = this->height();
    QFont font;
    QString current_datetime_str = "";
    QDateTime datetimestamp = QDateTime::currentDateTime();

    painter->save();
    font.setPixelSize(15);
    painter->setFont(font);
    painter->setPen(QColor(51, 223, 35));
    current_datetime_str = datetimestamp.toString("yyyy-MM-dd hh:mm:ss");
    if (m_position_ == POSITION_RIGHT_TOP)
    {
        painter->drawText(QRect(-width/4 + 10, -height/2, width *3/4, height), Qt::AlignCenter, current_datetime_str);
    } else if (m_position_ == POSITION_LEFT_TOP) {
        painter->drawText(QRect(-width/2 + 10, -height/4, width *3/4, height/2), Qt::AlignLeft, current_datetime_str);
    } else {
        painter->drawText(QRect(-width/4 + 10, -height/2, width *3/4, height), Qt::AlignCenter, current_datetime_str);
    }
    painter->restore();
}

void SignalRssi::updateValue()
{
    this->update();
}

void SignalRssi::updateValue(double value)
{
    this->m_rssi_ = value;
    this->update();
}

QColor SignalRssi::getBackgroundColor() const
{
    return this->m_background_color_;
}

QString SignalRssi::getTitle() const
{
    return this->m_title_;
}

double SignalRssi::getRssi() const
{
    return this->m_rssi_;
}

QSize SignalRssi::sizeHint() const
{
    return QSize(150, 150);
}

QSize SignalRssi::minimumSizeHint() const
{
    return QSize(30, 30);
}

void SignalRssi::setBackgroundColor(const QColor value)
{
    if (this->m_background_color_ != value)
    {
        this->m_background_color_ = value;
        this->update();
    }
}

void SignalRssi::setTitle(const QString value)
{
    if (this->m_title_ != value)
    {
        this->m_title_ = value;
        this->update();
    }
}

void SignalRssi::setRssi(const double value)
{
    if (this->m_rssi_ != value)
    {
        this->m_rssi_ = value;
        this->update();
    }
}

void SignalRssi::setPosition(const Position value)
{
    if (this->m_position_ != value)
    {
        this->m_position_ = value;
        this->update();
    }
}
