#include "netservice.h"

NetService::NetService(const QString &name)
    : m_name(name), m_available(false), m_numWaiting(0) {}

NetService::~NetService() { stopService(); }

bool NetService::startService(const AddrPair &addr) {
    if (isRunning())
        return false;

    // copy addr only - port is set in service
    m_addrPair.myAddr = addr.myAddr;
    m_addrPair.c64Addr = addr.c64Addr;

    // reset flags
    m_available = false;

    // start thread
    initStartup();
    start();
    return waitStartup();
}

void NetService::stopService() {
    // try ten times to stop service thread gracefully
    for (int i = 0; i < 10; i++) {
        // if not running then quit
        if (!isRunning())
            return;
        // disable service available flag
        {
            m_mutex.lock();
            m_available = false;
            m_mutex.unlock();
        }
        // wait for thread to stop
        wait(1000);
    }

    // last chance: terminate service thread
    terminate();
    reportEvent("terminated service");
}

bool NetService::isServiceAvailable() {
    QMutexLocker locker(&m_mutex);
    return m_available;
}

// ----- Task Handling -----

bool NetService::addTask(NetTask *task) {
    // check if service is running
    if (!isServiceAvailable()) {
        delete task;
        return false;
    }

    // enqueue new task
    m_mutex.lock();
    m_tasks.enqueue(task);
    m_mutex.unlock();
    return true;
}

void NetService::waitForTask(NetTask *task) {
    bool active = true;
    while (active) {
        // check if task is active==enqueued
        active = false;
        m_mutex.lock();
        for (int i = 0; i < m_tasks.size(); i++) {
            if (m_tasks.at(i) == task) {
                active = true;
                break;
            }
        }
        m_mutex.unlock();

        if (active) {
            // wait for scheduler to wake me up
            m_waitMutex.lock();
            m_numWaiting++;
            m_waitCond.wait(&m_waitMutex);
            m_numWaiting--;
            m_waitMutex.unlock();
        }
    }
}

void NetService::stopAllTasks() {
    // tell the scheduler to stop all tasks ASAP
    m_mutex.lock();
    for (int i = 0; i < m_tasks.size(); i++) {
        m_tasks.at(i)->doStop();
    }
    m_mutex.unlock();
}

// ---------- NetService Main -----------------------------------------------

void NetService::run() {
    // create host
    NetHost *host = new NetHost;
    connect(host, SIGNAL(sentPacket(int)), this, SIGNAL(sentPacket(int)));
    connect(host, SIGNAL(receivedPacket(int)), this,
            SIGNAL(receivedPacket(int)));

    // try to bind host
    bool ok = host->bind(m_addrPair);
    if (ok) {
        // set service available
        m_mutex.lock();
        m_available = true;
        m_mutex.unlock();
    } else {
        reportEvent(tr("ERROR: ") + host->errorString());
        delete host;
        host = 0;
    }

    // notify starter
    signalStartup(ok);

    // shut down due to error
    if (!ok)
        return;

    // we are bound and ready to proceed
    reportEvent(tr("Bound service: %1:%2")
                    .arg(m_addrPair.myAddr.toString())
                    .arg(QString::number(m_addrPair.myPort)));
    reportEvent(tr("  C64 Address: %1:%2")
                    .arg(m_addrPair.c64Addr.toString())
                    .arg(QString::number(m_addrPair.c64Port)));

    // main loop
    int idleCount = 0;
    forever {
        NetTask *currentTask;
        QList<NetTask *> oldTasks;
        {
            // ----- begin critical section -----
            QMutexLocker locker(&m_mutex);

            // stop service?
            if (!m_available) {
                break; // -> leave main loop!
            }

            // remove stopped tasks
            while (!m_tasks.empty() && m_tasks.head()->isStopped()) {
                NetTask *oldTask = m_tasks.dequeue();
                oldTasks << oldTask;
            }

            // get current task
            if (m_tasks.empty())
                currentTask = 0;
            else {
                currentTask = m_tasks.head();
            }
            // ----- end critical section -----
        }

        // remove old tasks?
        if (!oldTasks.isEmpty()) {
            foreach (NetTask *oldTask, oldTasks) {
                // report a finished task
                finishedTask(oldTask);
                // remove task
                delete oldTask;
            }

            // wake up all the threads waiting for a task
            wakeUpWaitingThreads();
        }

        // interface is bound: we can handle tasks!
        if (host->isBound()) {
            if (currentTask != 0) {
                idleCount = 0;

                // perform task
                NetTask::Result result;
                currentTask->setStatusMessage("");
                result = currentTask->run(*host);
                currentTask->setLastResult(result);
                QString statusMsg = currentTask->statusMessage();
                if (statusMsg != "")
                    reportEvent(statusMsg);

                // retry command?
                if (result == NetTask::RETRY) {
                    int retriesLeft = currentTask->retries();
                    if (retriesLeft == 0) {
                        result = NetTask::ERROR;
                        reportEvent(tr("Retries failed... time out? no C64?!"));
                    } else {
                        retriesLeft--;
                        currentTask->setRetries(retriesLeft);
                        reportEvent(tr("Retrying task (%1 retries left)")
                                        .arg(QString::number(retriesLeft)));
                        snooze(host);
                    }
                }
                // an error flushes the tasks
                if (result == NetTask::ERROR) {
                    flushTasks();
                    // flushPackets(host);
                    snooze(host);
                }
                // ready removes the current entry and goes on with the next one
                else if (result == NetTask::READY) {
                    currentTask->doStop();
                }
                // keep task
                else if (result == NetTask::KEEP) {
                    keepTask(currentTask);
                }
            } else {
                // idle too long -> close service
                if (idleCount == 100) {
                    reportEvent(tr("Nothing to do..."));
                    break;
                }

                // no task currently available... we are idle
                idleCount++;
                snooze(host);
            }
        }
        // not bound port -> try binding and sleep
        else {
            host->close();
            host->bind(m_addrPair);
            reportEvent(tr("Waiting for binding..."));
            msleep(2000);
        }
    }

    // flush all unfinished tasks
    flushTasks();

    // close host
    host->close();
    reportEvent(tr("Closed service"));
    delete host;

    // ensure available flag state
    m_mutex.lock();
    m_available = false;
    m_mutex.unlock();
}

void NetService::wakeUpWaitingThreads() {
    bool doWakeUp = false;
    m_waitMutex.lock();
    if (m_numWaiting > 0)
        doWakeUp = true;
    m_waitMutex.unlock();
    if (doWakeUp)
        m_waitCond.wakeAll();
}

void NetService::snooze(NetHost *host) {
    // either sleep
    if (host->hasPackets()) {
        reportEvent(tr("Packets pending..."));
        msleep(500);
    } else {
        host->waitForPacket(500);
    }
}

void NetService::flushTasks() {
    // clean up task list if service was stopped
    int unfinishedTasks = 0;
    {
        QMutexLocker locker(&m_mutex);
        while (!m_tasks.empty()) {
            NetTask *task = m_tasks.dequeue();
            finishedTask(task);
            delete task;
            unfinishedTasks++;
        }
    }
    if (unfinishedTasks > 0)
        reportEvent(
            tr("Flushed %1 tasks").arg(QString::number(unfinishedTasks)));

    wakeUpWaitingThreads();
}

void NetService::flushPackets(NetHost *host) {
    // flush packets
    int num = host->flushPackets(1000);
    if (num > 0)
        reportEvent(tr("Flushed %1 packets").arg(QString::number(num)));
}

void NetService::finishedTask(NetTask *) {
    // callbacks for own services
}

void NetService::keepTask(NetTask *) {
    // callbacks for own services
}

void NetService::reportEvent(const QString &msg) {
    QString event = m_name + ": " + msg;
    emit networkEvent(event);
}

// ----- Threading -----

void NetService::initStartup() {
    m_startupWait = true;
    m_startupOk = false;
}

bool NetService::waitStartup() {
    m_waitMutex.lock();
    if (m_startupWait) {
        m_startupWait = false;
        m_waitCond.wait(&m_waitMutex);
    }
    bool ok = m_startupOk;
    m_waitMutex.unlock();
    return ok;
}

void NetService::signalStartup(bool ok) {
    bool wakeUp = false;
    m_waitMutex.lock();
    if (m_startupWait)
        m_startupWait = false;
    else
        wakeUp = true;
    m_startupOk = ok;
    m_waitMutex.unlock();
    if (wakeUp)
        m_waitCond.wakeAll();
}
