#include <QObject>
#include <QWebPage>
#include <QScriptValue>
#include <QWebElement>

class WebPuppeteer;

class WebPuppeteerTab: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerTab(WebPuppeteer *parent);

	WebPuppeteer *getParent();

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
	bool click(const QString &text); // click at first occurence of text

	// for internal use
	void setReturnBool(bool);

	void downloadFile(QNetworkReply*);

private:
	QWebPage *page;
	bool return_bool;
	WebPuppeteer *parent;
};

