#include <QObject>
#include <QWebElement>

class WebPuppeteerWebElement: public QObject {
	Q_OBJECT;
public:
	WebPuppeteerWebElement(QWebElement el);

public slots:
	QString attribute(const QString &name);
	void setAttribute(const QString &name, const QString &value);
	QString xml(); // return element as xml
	void eval(const QString &js);

	// CSS
	void setStyleProperty(const QString &name, const QString &value);
	QString getComputedStyle(const QString &name);

private:
	QWebElement e;
};

