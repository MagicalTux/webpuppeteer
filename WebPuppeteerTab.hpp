#include <QObject>
#include <QWebPage>

class WebPuppeteer;

class WebPuppeteerTab: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerTab(WebPuppeteer *parent);

public slots:
	bool browse(const QString &url);

	// for internal use
	void setReturnBool(bool);

private:
	QWebPage *page;
	bool return_bool;
};

