#ifndef NETSERVICE_H
#define NETSERVICE_H

#include "nethost.h"

#include <QObject>

class NetTask
{
public:
  //! return result of task execution
  enum Result {
    READY,ERROR,RETRY,KEEP
  };

  //! create a new task
  NetTask() : m_stop(false),m_lastResult(READY),m_retries(3) {}
  //! clean
  virtual ~NetTask() {}

  //! execute task and return result and result event
  virtual Result run(NetHost &host) = 0;

  //! set a status message
  void setStatusMessage(const QString &msg) { m_statusMsg = msg; }
  //! get status message
  QString statusMessage() const { return m_statusMsg; }

  //! is task stopped?
  bool isStopped() const { return m_stop; }
  //! stop task
  void doStop() { m_stop = true; }

  //! last result
  Result lastResult() const { return m_lastResult; }
  //! set the last result
  void setLastResult(Result result) { m_lastResult = result; }

  //! set the retries
  void setRetries(int retries) { m_retries = retries; }
  //! get the retries
  int retries() const { return m_retries; }

protected:
  //! stop flag
  bool m_stop;
  //! last result
  Result m_lastResult;
  //! status msg
  QString m_statusMsg;
  //! retries
  int m_retries;
};

class NetService : public QThread
{
  Q_OBJECT

public:
  NetService(const QString &name);
  ~NetService();

  //! return service name
  QString name() const { return m_name; }

  //! start service - returns false if binding failed
  bool startService(const AddrPair &addrPair);
  //! tell the service to stop and cancel all scheduled tasks
  void stopService();
  //! is the service available? (in the main loop)
  bool isServiceAvailable();

  /** @name Task Management */
  //@{
  //! add a task (returns false and deletes task if service is not available)
  bool addTask(NetTask *task);
  //! wait for a task
  void waitForTask(NetTask *task);
  //! stop all running tasks
  void stopAllTasks();
  //@}

signals:
  //! emit a message
  void networkEvent(const QString &);
  //! sent packet
  void sentPacket(int size);
  //! received packet
  void receivedPacket(int size);

protected:
  QString  m_name;
  AddrPair m_addrPair;

  QMutex m_mutex;
  bool m_available;
  QQueue<NetTask *> m_tasks;
  QQueue<NetTask *> m_finishedTasks;

  QWaitCondition m_waitCond;
  QMutex m_waitMutex;
  int m_numWaiting;
  bool m_startupOk;
  bool m_startupWait;

  //! thread execution
  void run();

  void failFatal();
  void flushTasks();
  void flushPackets(NetHost *host);
  void snooze(NetHost *host);
  void wakeUpWaitingThreads();

  virtual void finishedTask(NetTask *task);
  virtual void keepTask(NetTask *task);
  void reportEvent(const QString &msg);

  void initStartup();
  bool waitStartup();
  void signalStartup(bool ok);
};

#endif
