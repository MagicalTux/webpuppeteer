#include <QObject>
#include <QScriptValue>

class WebPuppeteer;
class WebPuppeteerTab;

class WebPuppeteerSys: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerSys(WebPuppeteer *parent);

public slots:
	void log(QString msg);
	void sleep(int msecs);
	QScriptValue newTab();

private:
	WebPuppeteer *parent;
};

