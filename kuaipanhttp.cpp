#include "kuaipanhttp.h"

using namespace Kuaipan;

Http::Http(QObject *parent) :
    QObject(parent)
{
    ;
}

QString Http::get(QString url,QMap<QString, QString> params)
{
    _buffer = "";
    QNetworkAccessManager manager(0);
    url = _makeStdBaseUrl(url) + _makeQuery(params);
    _reply = manager.get(QNetworkRequest(QUrl(url)));
    QObject::connect(_reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
    QObject::connect(_reply, SIGNAL(finished()), this, SLOT(httpFinished()));
    _readLoop.exec();
    return _buffer;
}

bool Http::downloadGet(QString url,QMap<QString, QString> params, QString filePath)
{
    _downloadfile = new QFile(filePath);
    _downloadfile->open(QFile::ReadWrite);
    //QNetworkAccessManager manager(0);
    url = _makeStdBaseUrl(url) + _makeQuery(params);
    QString cmd = "wget -c -O \"" + filePath + "\" \"" + url + "\"";
    system(cmd.toUtf8());
    //_reply = manager.get(QNetworkRequest(QUrl(url)));
    //QObject::connect(_reply, SIGNAL(readyRead()), this, SLOT(downFileReadyRead()));
    //QObject::connect(_reply, SIGNAL(finished()), this, SLOT(downFileFinished()));
    return true;
}

bool Http::uploadPost(QString url,QMap<QString, QString> params,QString filePath)
{
    url = _makeStdBaseUrl(url) + _makeQuery(params);
    QString cmd = "curl -F \"file=@" + filePath + "\" \"" + url + "\"";
    system(cmd.toUtf8());
    return true;
}

QString Http::post(QString url,QMap<QString, QString> params)
{
    _buffer = "";
    QNetworkAccessManager manager(0);
    QString query = _makeQuery(params);
    _reply = manager.post(QNetworkRequest(QUrl(url)),query.toUtf8());
    QObject::connect(_reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
    QObject::connect(_reply, SIGNAL(finished()), this, SLOT(httpFinished()));
    _readLoop.exec();
    return _buffer;
}

QString Http::_makeQuery(QMap<QString, QString> params)
{
    QStringList paramList;
    QMap<QString,QString>::iterator it;
    for ( it = params.begin(); it != params.end(); ++it ) {
        paramList << QUrl::toPercentEncoding(it.key()) + "=" + QUrl::toPercentEncoding(it.value());
    }
    return paramList.join('&');
}

QString Http::_makeStdBaseUrl(QString url)
{
    if(url.indexOf("?")>0){
        if(url.right(1)!="?" && url.right(1)!="&"){
            return url+"&";
        }
    }else{
        return url+"?";
    }
    return url;
}

void Http::httpReadyRead()
{
    QString string(_reply->readAll());
    _buffer+=string;
}

void Http::httpFinished()
{
    _readLoop.exit();
    _reply->deleteLater();
}

void Http::downFileReadyRead()
{
    _downloadfile->write(_reply->readAll());
}

void Http::downFileFinished()
{
    _downloadfile->close();
}
