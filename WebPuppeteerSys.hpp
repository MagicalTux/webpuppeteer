#include <QObject>

class WebPuppeteer;

class WebPuppeteerSys: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerSys(WebPuppeteer *parent);

public slots:
	void log(QString msg);
	void sleep(int msecs);

private:
	WebPuppeteer *parent;
};

