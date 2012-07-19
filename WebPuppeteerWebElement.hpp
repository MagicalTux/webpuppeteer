#include <QObject>
#include <QWebElement>
#include <QScriptValue>

class WebPuppeteerTab;

class WebPuppeteerWebElement: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerWebElement(WebPuppeteerTab *parent, QWebElement el);

public slots:
	QString attribute(const QString &name);
	void setAttribute(const QString &name, const QString &value);
	QString xml(); // return element as xml
	void eval(const QString &js);

	// CSS
	void setStyleProperty(const QString &name, const QString &value);
	QString getComputedStyle(const QString &name);

	// fidning elements
	QScriptValue findFirst(const QString &selector);
	QScriptValue findAll(const QString &selector);

private:
	QWebElement e;
	WebPuppeteerTab *parent;
};

