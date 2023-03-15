#ifndef H264FILEPROCESSOR_H
#define H264FILEPROCESSOR_H

#include <QObject>
#include <QFile>

class H264FileProcessor : public QObject
{
    Q_OBJECT
public:
    explicit H264FileProcessor(QObject *parent = nullptr);
    ~H264FileProcessor();
    bool init();
    bool open(const QString &filename);

    void processFile();
    void processStream(const QString &input);
signals:
    void sendFrameData(const uint8_t *data_ptr, int data_len);
public slots:

private:
    bool m_run_flag_;
    QFile *m_file_ptr_;
};

#endif // H264FILEPROCESSOR_H
