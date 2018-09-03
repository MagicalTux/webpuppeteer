#include <QObject>
#include <QScriptValue>
#include <QNetworkAccessManager>
#include <QEventLoop>

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
	QScriptValue post(const QString &url, const QString &post, const QString content_type="application/x-www-form-urlencoded");
	QScriptValue newTab();
	QScriptValue getenv(const QString &var) const;
	QString fileGetContents(QString filename);
	QString fileGetContentsB64(QString filename);
	bool filePutContents(QString filename, QString data);
	bool filePutContentsB64(QString filename, QString data);
	QScriptValue include(QString filename); // include a file
	void alert(QString string);
	void alertcb(QString string, QScriptValue cb);
	bool confirm(QString string);
	QString exec(QString str);

	void quit();
	void abort();

	void alertcb_cb(); // used by alertcb internally

private:
	WebPuppeteer *parent;
	QNetworkAccessManager net;
	QScriptValue tmp_cb;
	QEventLoop *tmp_e;
};

