#include <QObject>
#include <QScriptValue>
#include <QNetworkAccessManager>

class WebPuppeteer;
class WebPuppeteerTab;

class WebPuppeteerSys: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerSys(WebPuppeteer *parent);

public slots:
	void log(const QString &msg);
	void sleep(int msecs);
	QScriptValue get(const QString &url);
	QScriptValue signedPost(const QString &url, const QString &post, const QString &api_key, const QString &api_secret);
	QScriptValue newTab();
	QString fileGetContents(QString filename);
	bool filePutContents(QString filename, QString data);
	bool filePutContentsB64(QString filename, QString data);
	QScriptValue include(QString filename); // include a file
	void alert(QString string);
	QString exec(QString str);

	void quit();
	void abort();

private:
	WebPuppeteer *parent;
	QNetworkAccessManager net;
};

