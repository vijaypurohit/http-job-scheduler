#include <iostream>
#include <string>
#include <functional>
#include <queue>
#include <thread>
#include <unordered_map>
#include <stdexcept>
#include <chrono>
#include <mutex>
#include <condition_variable>
using namespace std;

enum class PRIORITY {
    LOW,
    MEDIUM,
    HIGH
};

enum class STATUS {
    PENDING,
    RUNNING,
    COMPLETED
};

class Job {
    public:
    int jobId;
    PRIORITY priority;
    string jobName;
    STATUS status;
    function<void()> jobFunction;
    long sequence = 0; // input order, set by the scheduler

    bool operator < (const Job& other) const {
        if (priority != other.priority) {
            return static_cast<int>(priority) < static_cast<int>(other.priority);
        }
        return sequence > other.sequence; // same priority -> earlier job first
    }
    Job() : jobId(0), priority(PRIORITY::LOW), jobName(""), status(STATUS::PENDING), jobFunction(nullptr) {}
    Job(int id, PRIORITY prio, const string& name, function<void()> func)
        : jobId(id), priority(prio), jobName(name), jobFunction(func) {
            status = STATUS::PENDING;
        }

    Job(const Job& other)
        : jobId(other.jobId), priority(other.priority), jobName(other.jobName), status(other.status), jobFunction(other.jobFunction), sequence(other.sequence) {}
    Job& operator=(const Job& other) = default;
};


class JobScheduler {
    private:
        priority_queue<Job> jobQueue;
        unordered_map<int, Job> jobMap; // to keep track of jobs by their ID
        bool isRunning = false;
        thread executeThread;
        mutex mtx;              // protects jobQueue, jobMap, isRunning
        condition_variable cv;  // wakes the worker when a job is added / on shutdown
        long nextSequence = 0;
    public:

    JobScheduler() {
       isRunning = true;
       executeThread = thread(&JobScheduler::executeJob, this);
    }

    ~JobScheduler() {
        {
            lock_guard<mutex> lock(mtx);
            isRunning = false;
        }
        cv.notify_all();
        if (executeThread.joinable()) {
            executeThread.join();
        }
    }

    int addJob(Job& job){
        {
            lock_guard<mutex> lock(mtx);
            if(jobMap.find(job.jobId) != jobMap.end()){
                throw invalid_argument("Job ID already exists");
            }
            job.status = STATUS::PENDING;
            job.sequence = nextSequence++;
            jobQueue.push(job);
            jobMap[job.jobId] = job;
        }
        cv.notify_one();
        return job.jobId;
    }

    STATUS getStatus(int jobId){
        lock_guard<mutex> lock(mtx);
        auto it = jobMap.find(jobId);
        if(it == jobMap.end()){
            throw invalid_argument("Job ID not found");
        }
        return it->second.status;
    }

    private:
        void executeJob() {
            unique_lock<mutex> lock(mtx);
            while(true){
                cv.wait(lock, [this]{ return !isRunning || !jobQueue.empty(); });
                if(jobQueue.empty()) break; // stopped and nothing left to run

                Job job = jobQueue.top(); jobQueue.pop();
                jobMap[job.jobId].status = STATUS::RUNNING;
                cout<<"job "<<job.jobId<<" status: "<<(int)STATUS::RUNNING<<"\n";

                lock.unlock();          // don't hold the lock while the job runs
                if (job.jobFunction) job.jobFunction();
                lock.lock();

                jobMap[job.jobId].status = STATUS::COMPLETED;
            }
        }
};


int main()
{
    Job job3(3,
            PRIORITY::LOW,
            "Job 3",
            [](){ cout << "Executing Job 3 LOW" << endl; }
        );
    Job job2(2,
            PRIORITY::MEDIUM,
            "Job 2",
            [](){ cout << "Executing Job 2 MEDIUM" << endl; });
    Job job1(1,
            PRIORITY::HIGH,
            "Job 1",
            [](){ cout << "Executing Job 1 HIGH" << endl; });

    JobScheduler scheduler;
    scheduler.addJob(job3);
    scheduler.addJob(job2);
    scheduler.addJob(job1);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    for (int id : {1, 2, 3}) {
        cout << "job " << id << " final status: " << (int)scheduler.getStatus(id) << "\n";
    }
    return 0;
}
