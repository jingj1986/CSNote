#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <list>
#include <unistd.h>
#include <condition_variable>
using namespace std;

class ProCons {
public:
    ProCons() 
        : m_max(3)
        , m_stop(false) {}

    ~ProCons() = default;

    void produce() {
        for (int i = 0; i < 5; i++) {
            {
                unique_lock<mutex> lk(m_mtx);
                m_cnd.wait(lk, [this]{return this->m_prod.size() < this->m_max;});  // condition_variable only match unique_lock
                m_prod.push_back(i);
                m_cnd.notify_one();
            }
            //sleep(1);
            this_thread::sleep_for(chrono::milliseconds(500));
        }
        m_stop = true;
        //m_cnd.notify_one();
    }

    void consume() {
        while (1) {
            int val ;
            {
                unique_lock<mutex> lk(m_mtx);
                m_cnd.wait_for(lk, std::chrono::milliseconds(100), [this]{return !m_prod.empty();});
                // 如果不使用wait_for，在product设置结束后，需要notify
                //m_cnd.wait(lk, [this]{ return m_stop || !m_prod.empty();});       
                if (m_stop) {
                    return;
                }
                if (m_prod.empty()) {
                    continue;
                }
                val = m_prod.front();
                m_prod.pop_front();
                m_cnd.notify_one();     // 在这个例子中，该语句可以没有，因为都是消费者在等待生产者
                                        // 如果有生产者等待消费者的场景，则需要通知的，甚至是notify_all
                                        // 还有使用两个条件变量，A 与 B， 生产者等待A的条件，由消费者执行A.notify_one;
                                        // 消费者等待B条件，有生产者执行B.notify_one
            }
            cout << "Get val:" << val << endl; 
        }
    }

private: 
    ProCons(const ProCons&) = delete;
    ProCons& operator=(const ProCons&) = delete;
    ProCons(ProCons&&) = delete;
    ProCons& operator=(ProCons&&) = delete ;

private:
    bool m_stop;
    mutex m_mtx;
    condition_variable m_cnd;
    int m_max = 2;
    list<int> m_prod; 
};

int main() {
    ProCons prodconsume;

    thread product(&ProCons::produce, &prodconsume);
    thread consume(&ProCons::consumer, &prodconsume);

    product.join();
    consume.join();

    return 0;
}
