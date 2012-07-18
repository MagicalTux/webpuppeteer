#include <QObject>

class WebPuppeteer;

class WebPuppeteerConsole: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerConsole(WebPuppeteer *parent);

public slots:
	void log(QString msg);

private:
	WebPuppeteer *parent;
};

