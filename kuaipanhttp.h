#ifndef KUAIPANHTTP_H
#define KUAIPANHTTP_H

#include <QObject>
#include <QtNetwork>
#include <QString>

namespace Kuaipan {

    class Http : public QObject
    {
        Q_OBJECT
    private:
        QEventLoop _readLoop;
        QNetworkReply *_reply;
        QString _buffer;

        QFile *_downloadfile;
    public:
        explicit Http(QObject *parent = 0);
        QString get(QString url,QMap<QString, QString> params);
        QString post(QString url,QMap<QString, QString> params);

        bool downloadGet(QString url,QMap<QString, QString> params,QString filePath);
        bool uploadPost(QString url,QMap<QString, QString> params,QString filePath);
    private:
        QString _makeQuery(QMap<QString, QString> params);
        QString _makeStdBaseUrl(QString url);
    signals:

    public slots:
        void httpReadyRead();
        void httpFinished();

        void downFileReadyRead();
        void downFileFinished();
    };

}

#endif // KUAIPANHTTP_H
