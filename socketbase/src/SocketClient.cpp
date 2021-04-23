#include <alloca.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "SocketLog.h"

#include "SocketClient.h"

SocketClient::SocketClient(int socket, bool owned) {
    init(socket, owned, false);
}

SocketClient::SocketClient(int socket, bool owned, bool useCmdNum) {
    init(socket, owned, useCmdNum);
}

void SocketClient::init(int socket, bool owned, bool useCmdNum) {
    mSocket = socket;
    mSocketOwned = owned;
    mUseCmdNum = useCmdNum;
    pthread_mutex_init(&mWriteMutex, NULL);
    pthread_mutex_init(&mRefCountMutex, NULL);
    mPid = -1;
    mUid = -1;
    mGid = -1;
    mRefCount = 1;
    mCmdNum = 0;

    struct ucred creds;
    socklen_t szCreds = sizeof(creds);
    memset(&creds, 0, szCreds);

    int err = getsockopt(socket, SOL_SOCKET, SO_PEERCRED, &creds, &szCreds);
    if (err == 0) {
        mPid = creds.pid;
        mUid = creds.uid;
        mGid = creds.gid;
    }
}

SocketClient::~SocketClient() {
    if (mSocketOwned) {
        close(mSocket);
    }
}



int SocketClient::sendData(const void *data, int len) {
    struct iovec vec[1];
    vec[0].iov_base = (void *) data;
    vec[0].iov_len = len;

    pthread_mutex_lock(&mWriteMutex);
    int rc = sendDataLockedv(vec, 1);
    pthread_mutex_unlock(&mWriteMutex);

    return rc;
}

int SocketClient::sendDatav(struct iovec *iov, int iovcnt) {
    pthread_mutex_lock(&mWriteMutex);
    int rc = sendDataLockedv(iov, iovcnt);
    pthread_mutex_unlock(&mWriteMutex);

    return rc;
}

int SocketClient::sendDataLockedv(struct iovec *iov, int iovcnt) {

    if (mSocket < 0) {
        errno = EHOSTUNREACH;
        return -1;
    }

    if (iovcnt <= 0) {
        return 0;
    }

    int ret = 0;
    int e = 0; // SLOGW and sigaction are not inert regarding errno
    int current = 0;

    struct sigaction new_action, old_action;
    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &new_action, &old_action);

    for (;;) {
        ssize_t rc = TEMP_FAILURE_RETRY(
            writev(mSocket, iov + current, iovcnt - current));

        if (rc > 0) {
            size_t written = rc;
            while ((current < iovcnt) && (written >= iov[current].iov_len)) {
                written -= iov[current].iov_len;
                current++;
            }
            if (current == iovcnt) {
                break;
            }
            iov[current].iov_base = (char *)iov[current].iov_base + written;
            iov[current].iov_len -= written;
            continue;
        }

        if (rc == 0) {
            e = EIO;
            SLOGW("0 length write :(");
        } else {
            e = errno;
            SLOGW("write error (%s)", strerror(e));
        }
        ret = -1;
        break;
    }

    sigaction(SIGPIPE, &old_action, &new_action);

    if (e != 0) {
        errno = e;
    }
    return ret;
}

void SocketClient::incRef() {
    pthread_mutex_lock(&mRefCountMutex);
    mRefCount++;
    pthread_mutex_unlock(&mRefCountMutex);
}

bool SocketClient::decRef() {
    bool deleteSelf = false;
    pthread_mutex_lock(&mRefCountMutex);
    mRefCount--;
    if (mRefCount == 0) {
        deleteSelf = true;
    } else if (mRefCount < 0) {
        SLOGE("SocketClient refcount went negative!");
    }
    pthread_mutex_unlock(&mRefCountMutex);
    if (deleteSelf) {
        delete this;
    }
    return deleteSelf;
}
