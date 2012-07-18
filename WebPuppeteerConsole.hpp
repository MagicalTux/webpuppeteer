#include <QObject>

class WebPuppeteer;

class WebPuppeteerConsole: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerConsole(WebPuppeteer *parent);

public slots:
	void log(QString msg);
	void sleep(int msecs);

private:
	WebPuppeteer *parent;
};

