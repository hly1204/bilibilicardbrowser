#ifndef COLLECTION_EXPORT_WORKER_HH
#define COLLECTION_EXPORT_WORKER_HH

#include <QObject>
#include <QString>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QFile;
QT_END_NAMESPACE

struct MyDecomposeData;
class BilibiliRequestManager;

class CollectionExportWorker : public QObject
{
    Q_OBJECT

public:
    explicit CollectionExportWorker(QObject *parent = nullptr);

public slots:
    void exportToCsvFile(const QString &file_name, const QString &cookie);
    void stopAction();

private slots:
    void onMyDecomposeDataReceived(int scene, const QByteArray &json);
    void onAssetBagDataReceived(int act_id, const QString &act_name, int lottery_id, int ruid,
                                const QByteArray &json);

signals:
    void finished();
    void progressChanged(int current, int total);

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    BilibiliRequestManager *manager_;
    QFile *file_;
    Qt::TimerId timer_id_;
    int current_;
    int total_;
    QScopedPointer<MyDecomposeData> my_decompose_data_;
};

#endif