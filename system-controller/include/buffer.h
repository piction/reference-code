#ifndef BUFFER_H
#define BUFFER_H

#include "pch.h"

template<typename T>
class Buffer
{
    public:
        int QueueNewMessage(const T &msg)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                Messages.push(msg);
            }
            m_cv.notify_one();
            return 0;
        }

        int UnqueueMessage (T &msg)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (Messages.size() == 0) {
                return 0;
            }

            msg = Messages.front();
            Messages.pop();

            return 1;
        }
        unsigned int GetSize() {
            std::lock_guard<std::mutex> lock(m_mutex);
            return Messages.size();
        }

    public:
        std::condition_variable& conditionVariable()
        {
            return m_cv;
        }

    private:
        std::queue<T> Messages;

        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
};

#endif // BUFFER_H
