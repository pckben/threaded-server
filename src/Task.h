#ifndef TASK_H
#define TASK_H
namespace pckben {
  class TaskObserver;

  // Abstract Task that can be carried out by a Worker. Tasks are observable by
  // TaskObserver classes.
  //
  // Implementation of this class should include the concrete information of the
  // Task to be carried out.
  class Task {
   public:
    Task() : observer_(0) { }

    virtual ~Task() { }

    void SetObserver(TaskObserver* observer) { observer_ = observer; }

    // Call this to notify the observer that the task has been done.
    //
    // Reference: BackgroundWorker
    void Done();

   private:
    TaskObserver* observer_;
  };

  // TaskObserver interface. Implementation of this interface will be notified
  // by Task-related events.
  class TaskObserver {
   public:
    virtual ~TaskObserver() { }
    virtual void OnTaskDone(Task* task) = 0;
  };
}
#endif  // TASK_H
