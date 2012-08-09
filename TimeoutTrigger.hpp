#include <QObject>
#include <QTimer>

// really simple class. Invoke it. If start() is called, then it'll wait. If end() is called, then after timeout ms it'll fire timeout() unless start() is called again

class TimeoutTrigger: public QObject {
	Q_OBJECT;
public:
	TimeoutTrigger(int timeout = 500);

public slots:
	void start();
	void end();
	void timer_timeout();

signals:
	void timeout();

private:
	QTimer t;
	bool in_op;
};

