#include <QObject>
#include <QWebPage>
#include <QScriptValue>

class WebPuppeteer;

class WebPuppeteerTab: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerTab(WebPuppeteer *parent);

public slots:
	bool browse(const QString &url); // browse to given url
	bool screenshot(const QString &filename); // take screenshot at url
	bool fullshot(const QString &filename); // take full-size screenshot at url
	bool print(const QString &filename); // print page as PDF
	QScriptValue eval(const QString &js); // evaluate JS in the window context

	// for internal use
	void setReturnBool(bool);

private:
	QWebPage *page;
	bool return_bool;
	WebPuppeteer *parent;
};

