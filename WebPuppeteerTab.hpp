#include <QWebPage>
#include <QScriptValue>
#include <QWebElement>
#include <QSslError>
#include <QSet>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class WebPuppeteer;
class WebPuppeteerTabNetSpy;

class WebPuppeteerTab: public QWebPage {
	Q_OBJECT;
public:
	WebPuppeteerTab(WebPuppeteer *parent);

	WebPuppeteer *getParent();

	virtual bool supportsExtension(Extension e);
	virtual bool extension(Extension extension, ExtensionOption *option = 0, ExtensionReturn *output = 0);

public slots:
	// navigation
	bool load(const QString &url, int timeout = 60); // go to given url
	bool browse(const QString &url, int timeout = 60); // browse to given url
	bool post(const QString &url, const QString &post, const QString content_type="application/x-www-form-urlencoded", int timeout=60);
	void back(); // go back
	void reload(bool force_no_cache = false);
	bool wait(int timeout = 60); // wait for page to finish loading
	void waitFinish(int idle = 50); // 500ms default idle, if something loads again within those 500 ms, then wait until it finishes
	QScriptValue getNewWindow(); // get latest opened window

	bool saveNetwork(const QString &filename);

	void overrideUserAgent(const QString &ua); // change user agent
	void setDefaultCharset(const QString &charset);

	// download
	QScriptValue getDownloadedFile(); // get one downloaded file

	// screenshot / print
	bool screenshot(const QString &filename); // take screenshot at url
	bool fullshot(const QString &filename); // take full-size screenshot at url
	bool print(const QString &filename); // print page as PDF
	QString printBase64(); // print, return pdf as base64

	// js eval
	QScriptValue eval(const QString &js); // evaluate JS in the window context

	// access to DOM
	QScriptValue document();
	void interact(); // create a window alloing the user to interact with the page, wait until closed
	QScriptValue get(const QString &url);
	void type(const QString &text);
	void typeEnter();
	void typeTab();

	QString getHtml(); // return html

	void trustCertificate(const QString &hash);

	// for internal use
	void setReturnBool(bool);
	bool shouldInterruptJavaScript();
	void handleSslErrors(QNetworkReply*,const QList<QSslError>&);

	void downloadFile(QNetworkReply*);
	void downloadFileFinished(QNetworkReply*reply=0);

	void test(QNetworkReply*);

protected:
	virtual void javaScriptAlert(QWebFrame*, const QString &msg);
	virtual bool javaScriptConfirm(QWebFrame*, const QString &msg);
	virtual QWebPage *createWindow(WebWindowType type);
	virtual QString userAgentForUrl(const QUrl&) const;

private:
	bool return_bool;
	WebPuppeteer *parent;
	WebPuppeteerTabNetSpy *spy;

	QSet<QByteArray> trusted_certificates;
	QList<WebPuppeteerTab*> queue;
	QList<QScriptValue> file_queue;
	QString user_agent;
};

class WebPuppeteerTabNetSpy: public QNetworkAccessManager {
	Q_OBJECT;
public:
	WebPuppeteerTabNetSpy(QObject *parent = 0);
	int getCount() const;
	void setOutputFile(QFile *output);
signals:
	void started();
	void allFinished();
public slots:
	void spyConnectionData();
	void spyMetaData();
	void spyFinished(QNetworkReply*);
protected:
	virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = 0);
private:
	int cnx_count;
	qint64 cnx_index;
	QFile *data_output;
};
