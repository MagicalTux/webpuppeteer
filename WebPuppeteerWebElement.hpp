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
	bool click();

	// CSS
	void setStyleProperty(const QString &name, const QString &value);
	QString getComputedStyle(const QString &name);

	// details
	QString tagName();

	// fidning elements
	QScriptValue findFirst(const QString &selector);
	QScriptValue findAll(const QString &selector);
	QScriptValue findAllContaining(const QString &text);
	QScriptValue getElementById(const QString &id);
	QScriptValue parentNode();
	QScriptValue firstChild();
	QScriptValue nextSibling();

private:
	QWebElement e;
	WebPuppeteerTab *parent;

	QList<QWebElement> allChildren();
};

