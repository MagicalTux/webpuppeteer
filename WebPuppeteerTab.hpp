#include <QWebPage>
#include <QScriptValue>
#include <QWebElement>
#include <QSslError>
#include <QSet>

class WebPuppeteer;

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
	QScriptValue getNewWindow(); // get latest opened window

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
	QString treeDump(); // for debug
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

protected:
	virtual void javaScriptAlert(QWebFrame*, const QString &msg);
	virtual QWebPage *createWindow(WebWindowType type);

private:
	bool return_bool;
	WebPuppeteer *parent;

	QSet<QByteArray> trusted_certificates;
	QList<WebPuppeteerTab*> queue;
	QList<QScriptValue> file_queue;
};

