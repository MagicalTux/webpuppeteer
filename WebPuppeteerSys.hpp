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
	QScriptValue newTab();
	bool filePutContents(QString filename, QString data);

	void quit();
	void abort();

private:
	WebPuppeteer *parent;
	QNetworkAccessManager net;
};

