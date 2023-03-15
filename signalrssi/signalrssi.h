#ifndef SIGNALRSSI_H
#define SIGNALRSSI_H

/**
 * 汽车仪表盘控件
 * 1:可设置范围值,支持负数值
 * 2:可设置精确度,最大支持小数点后3位
 * 3:可设置大刻度数量/小刻度数量
 * 4:可设置开始旋转角度/结束旋转角度
 * 5:可设置是否启用动画效果以及动画效果每次移动的步长
 * 6:可设置外圆背景/内圆背景/饼圆三种颜色/刻度尺颜色/文字颜色
 * 7:自适应窗体拉伸,刻度尺和文字自动缩放
 * 8:可自由拓展各种渐变色,各圆的半径
 * 9:三色圆环按照比例设置范围角度 用户可以自由设置三色占用比例
 * 10:圆环样式可选择 三色圆环 当前圆环
 * 11:指示器样式可选择 圆形指示器 指针指示器 圆角指针指示器 三角形指示器
 */

#include <QWidget>
#include <QTimer>


#ifdef quc
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
#include <QtDesigner/QDesignerExportWidget>
#else
#include <QtUiPlugin/QDesignerExportWidget>
#endif

class QDESIGNER_WIDGET_EXPORT VehicleGauge : public QWidget
#else
class SignalRssi : public QWidget
#endif

{
    Q_OBJECT
    Q_PROPERTY(double m_rssi_ READ getRssi WRITE setRssi)
    Q_PROPERTY(QString m_title_ READ getTitle WRITE setTitle)
    Q_PROPERTY(QColor m_background_color_ READ getBackgroundColor WRITE setBackgroundColor)
public:
    enum Position {
        POSITION_LEFT_TOP = 0,
        POSITION_RIGHT_TOP = 1,
        POSITION_CENTER_TOP = 2,
    };
    explicit SignalRssi(QWidget *parent = 0, Position pos = POSITION_RIGHT_TOP);
    ~SignalRssi();

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

    void drawBackground(QPainter *painter);
    void drawtitle(QPainter *painter);
    void drawrssi(QPainter *painter);
    void drawSystemTime(QPainter *painter);
private slots:
    void updateValue();
    void updateValue(double value);
private:
    QTimer *m_timer_ptr_;

    QColor m_background_color_;
    QString m_title_{""};
    double m_rssi_{0.0};     // unit:1ms
    Position m_position_{POSITION_RIGHT_TOP};
public:
    QColor getBackgroundColor()     const;
    QString getTitle() const;
    double getRssi()          const;

    QSize sizeHint()                const;
    QSize minimumSizeHint()         const;    
public Q_SLOTS:
    void setBackgroundColor(const QColor value);
    void setTitle(const QString value);
    void setRssi(const double value);
    void setPosition(const Position value);
Q_SIGNALS:
    void valueChanged(int value);
};

#endif //VEHICLEGAUGE_H

