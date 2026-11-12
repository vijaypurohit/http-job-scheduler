Design and implement a small HTTP service to manage background jobs.
 
The service should allow clients to submit jobs that need to be processed asynchronously. 
Jobs can have different priorities, and the system should decide how to handle them based on their priority.
 
HIGH: Process the job immediately using a mock worker.
MEDIUM: Record the job for later processing.
LOW: Record the job without triggering immediate processing.
 
The system should maintain the state of submitted jobs so that their progress and outcomes can be tracked.



task, priority -> 
based on input order.

Entities
Job
    function:
    priority
    status

JobScheduler
    list of submitted job - sorted through priority
    map<jobId, Job> -> status

POST /api/v1/job 
{
    name:
    type:   PAYROLL,   
               HR
              LEAVE        
}
-> jobId

GET /api/v1/job/id=?
-> {id, name, status}


