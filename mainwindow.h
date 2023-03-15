#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>

#include <atomic>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


protected:
    void drawBorder(QPainter *painter);
    void drawBackgroud(QPainter *painter);

    void closeEvent(QCloseEvent *);
    void paintEvent(QPaintEvent *);

private:
    Ui::MainWindow *ui;

    QString m_bg_text_;
    QImage m_bg_image_;

    QColor m_focus_color_;
    QColor m_border_color_;

    int m_border_width_;
    double m_lane_start_offset_;
    QTimer *m_vehicle_runing_timer_;

    // ilive video handle
    std::atomic_bool run_status_{false};
    // std::shared_ptr<PIMediaPlayer> media_player_handler_ = {nullptr};

    QString layout_str;
    QString layoutPath{"."};

    int m_camera_geometry_x{10};
    int m_camera_geometry_y{600};
    int m_camera_geometry_w{512};
    int m_camera_geometry_h{288};
    void changeLayout(const QString &layout, bool init);
    void initLayout(const QString &layout);
    void saveLayout(const QString &layout, int type);

    void initForm();
    void initStyle();
    void initData();
    void initView();

    void closeAll();
signals:
    void updateImage(int position, double timecost_ms, const QImage &image);


public slots:
    void slot_timeout();
    void slot_update_timerout();
    void slot_go_remotedrive(QString vehicleid, bool state);
};


#endif // MAINWINDOW_H
