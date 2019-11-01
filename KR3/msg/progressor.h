#pragma once

#include <KR3/wl/windows.h>
#include <KR3/mt/thread.h>
#include <KR3/util/time.h>

namespace kr
{
	class ProgressNotifier
	{
	public:
		ProgressNotifier() noexcept;
		~ProgressNotifier() noexcept;

		void set(int64_t pr) noexcept;
		void set(int64_t pr, int64_t length) noexcept;
		void setEnd() noexcept;
		int64_t progress() noexcept;
		int64_t length() noexcept;

		virtual void onProgress() noexcept = 0;

	protected:
		int64_t m_progress;
		int64_t m_length;

	private:
		timepoint m_start;

	};
	class Progressor:private ProgressNotifier
	{
	public:
		Progressor() noexcept;
		~Progressor() noexcept;

		void start() noexcept;
		bool closed() noexcept;
		template <typename LAMBDA>
		bool postL(LAMBDA lambda)
		{
			return m_thread->post(lambda);
		}
		void quit(int exitCode) noexcept;
		void join() noexcept;
		int64_t getProgress() noexcept;
		int64_t getLength() noexcept;
		void setProgress(int64_t progress) noexcept;
		void setProgress(int64_t progress, int64_t length) noexcept;
		void checkPoint() throws(QuitException);
		void checkPoint(long long nProgress) throws(QuitException);
		virtual int progressor() noexcept = 0;
		virtual void onProgress() noexcept override = 0;
		
	private:
		union
		{
			ThreadHandle * m_thread;
			EventHandle * m_readyEvent;
		};
		ThreadId m_threadId;

		static int CT_STDCALL _thread(Progressor* _this) noexcept;
	};

}
