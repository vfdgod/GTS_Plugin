#include "Systems/Misc/Tasks.hpp"

namespace {

	struct QueuedTask {
		std::string name;
		std::shared_ptr<GTS::BaseTask> task;
	};

	class TaskUpdateGuard {
		public:
		explicit TaskUpdateGuard(GTS::BaseTask& a_task) : task(a_task) {}
		~TaskUpdateGuard() {
			task.EndUpdate();
		}

		TaskUpdateGuard(const TaskUpdateGuard&) = delete;
		TaskUpdateGuard& operator=(const TaskUpdateGuard&) = delete;

		private:
		GTS::BaseTask& task;
	};

	std::vector<QueuedTask> CollectTasksForUpdate(auto& taskings, std::mutex& taskingsLock, GTS::UpdateKind kind) {
		std::vector<QueuedTask> queued;

		{
			std::scoped_lock lock(taskingsLock);
			queued.reserve(taskings.size());

			for (auto& [name, task] : taskings) {
				if (task && task->UpdateOn() == kind) {
					queued.push_back({
						name,
						task,
					});
				}
			}
		}

		return queued;
	}

	void RemoveCompletedTasks(auto& taskings, std::mutex& taskingsLock, const std::vector<QueuedTask>& toRemove) {

		if (toRemove.empty()) {
			return;
		}

		std::scoped_lock lock(taskingsLock);

		for (const auto& queued : toRemove) {
			auto it = taskings.find(queued.name);
			if (it != taskings.end() && it->second == queued.task) {
				taskings.erase(it);
			}
		}
	}

	void UpdateTasksForKind(auto& taskings, std::mutex& taskingsLock, GTS::UpdateKind kind) {
		auto queued = CollectTasksForUpdate(taskings, taskingsLock, kind);

		std::vector<QueuedTask> toRemove;
		toRemove.reserve(queued.size());

		for (const auto& entry : queued) {
			if (!entry.task || !entry.task->TryBeginUpdate()) {
				continue;
			}

			TaskUpdateGuard updateGuard(*entry.task);
			try {
				if (entry.task->Update()) {
					continue;
				}
			}
			catch (const std::exception& e) {
				logger::error("Task '{}' failed: {}", entry.name, e.what());
			}
			catch (...) {
				logger::error("Task '{}' failed with an unknown exception", entry.name);
			}

			toRemove.push_back(entry);
		}

		RemoveCompletedTasks(taskings, taskingsLock, toRemove);
	}
}

namespace GTS {

	//-----------
	// TASK
	//-----------

	Task::Task(const std::function<bool(const TaskUpdate&)>& tasking) : 
		startTime(Time::WorldTimeElapsed()), lastRunTime(Time::WorldTimeElapsed()), tasking(tasking) {}

	bool Task::Update() {
		TaskUpdate update;
		double currentTime = Time::WorldTimeElapsed();

		if (this->initRun) {
			update = TaskUpdate{
				.runtime = currentTime - this->startTime,
				.delta = currentTime - this->lastRunTime,
			};
		}
		else {
			update = TaskUpdate{
				.runtime = 0.0,
				.delta = 0.0,
			};
			this->initRun = true;
		}

		this->lastRunTime = currentTime;
		return this->tasking(update);
	}

	//-----------
	// TASK FOR
	//-----------

	TaskFor::TaskFor(double duration, const std::function<bool(const TaskForUpdate&)>& tasking)
		: startTime(Time::WorldTimeElapsed()),lastRunTime(Time::WorldTimeElapsed()), tasking(tasking), duration(duration) {}

	bool TaskFor::Update() {
		double currentTime = Time::WorldTimeElapsed();
		double currentRuntime = currentTime - this->startTime;

		double currentProgress = 0.0;
		if (this->duration > 0.0) {
			currentProgress = std::clamp(currentRuntime / this->duration, 0.0, 1.0);
		}

		TaskForUpdate update;

		if (this->initRun) {
			update = TaskForUpdate{
				.runtime = currentRuntime,
				.delta = currentTime - this->lastRunTime,
				.progress = currentProgress,
				.progressDelta = currentProgress - this->lastProgress,
			};
		}
		else {
			update = TaskForUpdate{
				.runtime = 0.0,
				.delta = 0.0,
				.progress = 0.0,
				.progressDelta = 0.0,
			};
			this->initRun = true;
		}

		this->lastRunTime = currentTime;
		this->lastProgress = currentProgress;

		bool shouldContinue = this->tasking(update);
		return shouldContinue && currentRuntime <= this->duration;
	}

	//-----------
	// ONE SHOT
	//-----------

	Oneshot::Oneshot(const std::function<void(const OneshotUpdate&)>& tasking) 
		: creationTime(Time::WorldTimeElapsed()), tasking(tasking) {}

	bool Oneshot::Update() {
		double currentTime = Time::WorldTimeElapsed();

		OneshotUpdate update{
			.timeToLive = currentTime - this->creationTime,
		};

		this->tasking(update);
		return false;
	}

	//---------------
	// TASK MANAGER
	//---------------

	

	std::string TaskManager::DebugName() {
		return "::TaskManager";
	}

	std::string TaskManager::GenerateName(void* ptr) {
		return std::format("UNNAMED_{}", reinterpret_cast<std::uintptr_t>(ptr));
	}

	void TaskManager::UpdateTasks(UpdateKind kind) {
		UpdateTasksForKind(m_taskings, m_taskingsLock, kind);
	}

	void TaskManager::InsertTask(std::unique_ptr<BaseTask> task) {
		InsertTask(GenerateName(task.get()), std::move(task));
	}

	void TaskManager::InsertTask(std::string_view name, std::unique_ptr<BaseTask> task) {
		std::scoped_lock lock(m_taskingsLock);
		auto sharedTask = std::shared_ptr<BaseTask>(std::move(task));
		auto [it, inserted] = m_taskings.try_emplace(std::string(name), std::move(sharedTask));
		if (!inserted) {
			//logger::warn("Task '{}' already exists", name);
		}
	}

	void TaskManager::Update() {
		UpdateTasks(UpdateKind::Main);
	}

	void TaskManager::CameraUpdate() {
		UpdateTasks(UpdateKind::Camera);
	}

	void TaskManager::HavokUpdate() {
		UpdateTasks(UpdateKind::Havok);
	}

	void TaskManager::ChangeUpdate(std::string_view name, UpdateKind updateOn) {
		std::scoped_lock lock(m_taskingsLock);

		auto it = m_taskings.find(std::string(name));
		if (it != m_taskings.end()) {
			it->second->SetUpdateOn(updateOn);
		}
	}

	void TaskManager::Cancel(std::string_view name) {
		std::scoped_lock lock(m_taskingsLock);
		m_taskings.erase(std::string(name));
	}

	void TaskManager::Run(const std::function<bool(const TaskUpdate&)>& tasking) {
		InsertTask(std::make_unique<Task>(tasking));
	}

	void TaskManager::Run(std::string_view name, const std::function<bool(const TaskUpdate&)>& tasking) {
		InsertTask(name, std::make_unique<Task>(tasking));
	}

	void TaskManager::RunFor(float duration, const std::function<bool(const TaskForUpdate&)>& tasking) {
		InsertTask(std::make_unique<TaskFor>(duration, tasking));
	}

	void TaskManager::RunFor(std::string_view name, float duration, const std::function<bool(const TaskForUpdate&)>& tasking) {
		InsertTask(name, std::make_unique<TaskFor>(duration, tasking));
	}

	void TaskManager::RunOnce(const std::function<void(const OneshotUpdate&)>& tasking) {
		InsertTask(std::make_unique<Oneshot>(tasking));
	}

	void TaskManager::RunOnce(std::string_view name, const std::function<void(const OneshotUpdate&)>& tasking) {
		InsertTask(name, std::make_unique<Oneshot>(tasking));
	}

	void TaskManager::CancelAllTasks() {
		{
			std::scoped_lock lock(m_taskingsLock);
			m_taskings.clear();
		}

		logger::info("Canceled all task manager tasks");
	}
	void TaskManager::Reset() {
		CancelAllTasks();
	}
	void TaskManager::OnGameLoaded() {
		CancelAllTasks();
	}
}
