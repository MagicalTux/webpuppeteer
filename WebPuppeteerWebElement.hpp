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
	bool hasAttribute(const QString &name);
	QString xml(); // return element as xml
	QString textContent();
	QScriptValue eval(const QString &js);
	bool click();
	bool onblur();
	bool onchange();

	// CSS
	void setStyleProperty(const QString &name, const QString &value);
	QString getComputedStyle(const QString &name);

	// details
	QString tagName();

	// fidning elements
	QScriptValue find(QScriptValue);
	QScriptValue findFirst(const QString &selector);
	QScriptValue findAll(const QString &selector);
	QScriptValue findAllContaining(const QString &text);
	QScriptValue getElementById(const QString &id);
	QScriptValue getElementsByTagName(const QString &tag);
	QScriptValue getElementsByName(const QString &name);
	QScriptValue parentNode();
	QScriptValue firstChild();
	QScriptValue nextSibling();
	QScriptValue frameDocument(QString framename);

	// focus
	void setFocus();
	bool hasFocus();

private:
	QWebElement e;
	WebPuppeteerTab *parent;

	QList<QWebElement> allChildren();
};

