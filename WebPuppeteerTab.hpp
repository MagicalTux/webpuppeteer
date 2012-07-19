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
	bool browse(const QString &url); // browse to given url
	bool screenshot(const QString &filename); // take screenshot at url
	bool fullshot(const QString &filename); // take full-size screenshot at url
	bool print(const QString &filename); // print page as PDF
	QScriptValue eval(const QString &js); // evaluate JS in the window context
	QScriptValue document();
	QString treeDump(); // for debug
	void interact(); // create a window alloing the user to interact with the page, wait until closed
	QScriptValue get(const QString &url);
	bool wait(); // wait for page to finish loading
	void type(const QString &text);
	void typeEnter();

	void trustCertificate(const QString &hash);

	// for internal use
	void setReturnBool(bool);
	bool shouldInterruptJavaScript();
	void handleSslErrors(QNetworkReply*,const QList<QSslError>&);

	void downloadFile(QNetworkReply*);

protected:
	virtual void javaScriptAlert(QWebFrame*, const QString &msg);

private:
	bool return_bool;
	WebPuppeteer *parent;

	QSet<QByteArray> trusted_certificates;
};

