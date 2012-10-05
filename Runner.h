#ifndef RUNNER_H
#define RUNNER_H

namespace ntu {
	class Thread;

	class Runner {
		public:
			/**
			 * Execute the Run() function on the given
			 * thread.
			 */
			void Run(Thread* thread);

		protected:
			/**
			 * Abstract running logic, to be implemented
			 * by different types of Runner subclasses.
			 *
			 * This function will be executed on the Thread
			 * given in the above Run(Thread*) function.
			 */
			virtual void DoRun() = 0;

		private:
			static void* run_function(void* runner_instance) {
				Runner* runner = static_cast<Runner*>(runner_instance);
				runner->DoRun();
			}
	};
}

#endif /* RUNNER_H */
