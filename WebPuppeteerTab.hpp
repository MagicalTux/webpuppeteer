#include <QObject>

class WebPuppeteer;

class WebPuppeteerTab: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerTab(WebPuppeteer *parent);

public slots:
	void browse(const QString &url);
};

