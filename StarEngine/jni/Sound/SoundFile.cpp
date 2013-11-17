#include "SoundFile.h"
#include "SoundService.h"
#include "../Logger.h"
#include "../Assets/Resource.h"
#include "../Helpers/Helpers.h"
#include "../Helpers/Filepath.h"

#ifdef ANDROID
#include "../StarEngine.h"
#endif

namespace star
{
	SoundFile::SoundFile(const tstring& path)
		: BaseSound()
		, mLoopTimes(0)
		, mbQueuedPlay(false)
#ifdef ANDROID
		, mPlayerObj(nullptr)
		, mPlayer(nullptr)
		, mPlayerSeek(nullptr)
#else
		, mpSound(nullptr)
#endif
	{
#ifdef ANDROID
		SLEngineItf engine = SoundService::GetInstance()->GetEngine();
		CreateSound(mPlayerObj, engine, mPlayer, path);
#else
		Filepath real_path(path);
		sstring sound_path = string_cast<sstring>(real_path.GetAssetsPath());
		mpSound = Mix_LoadMUS(sound_path.c_str());
		if(!mpSound)
		{
			star::Logger::GetInstance()->Log(star::LogLevel::Error,
				_T("SoundFile: Could not load sound, reason : ")
				+ string_cast<tstring>(Mix_GetError()));
		}
#endif
	}

	SoundFile::~SoundFile()
	{
#ifdef DESKTOP
		if(mpSound != nullptr)
		{	
			Mix_FreeMusic(mpSound);
			mpSound = nullptr;
		}
#else
		DestroySound(mPlayerObj, mPlayer);
#endif
	}

	void SoundFile::Play(int32 looptimes)
	{
		mLoopTimes = looptimes;
		star::Logger::GetInstance()->Log(star::LogLevel::Info,
			_T("Sound File: Playing File , Looptimes = ") +
			star::string_cast<tstring>(mLoopTimes));
#ifdef DESKTOP
		Mix_HookMusicFinished(NULL);
		Mix_PlayMusic(mpSound, mLoopTimes);
#else
		SLresult lRes;
		if(mLoopTimes == -1)
		{
			lRes = (*mPlayerSeek)->SetLoop(
				mPlayerSeek,
				SL_BOOLEAN_TRUE,
				0,
				SL_TIME_UNKNOWN
				);

			if (lRes != SL_RESULT_SUCCESS)
			{
				star::Logger::GetInstance()->Log(
					star::LogLevel::Error,
					_T("Sound File: Can't set audio loop")
					);
				Stop();
				return;
			}
		}

		lRes = (*mPlayer)->SetPlayState(mPlayer,SL_PLAYSTATE_PLAYING);
		if (lRes != SL_RESULT_SUCCESS)
		{
			star::Logger::GetInstance()->Log(star::LogLevel::Error,
				_T("Sound File: Can't play audio"));
			Stop();
			return;
		};
#endif
	}
	
	void SoundFile::PlayQueued(int32 looptimes)
	{
		mLoopTimes = looptimes;
		mbQueuedPlay = true;

#ifdef DESKTOP
		Mix_HookMusicFinished(MusicStoppedCallback);
		Mix_PlayMusic(mpSound, mLoopTimes);
#else
		Play(mLoopTimes);
#endif
	}

	void SoundFile::Stop()
	{		
#ifdef DESKTOP
		Mix_PauseMusic();
		Mix_RewindMusic();
#else	
		(*mPlayer)->SetPlayState(mPlayer, SL_PLAYSTATE_STOPPED);
#endif
	}

	void SoundFile::Pause()
	{
#ifdef DESKTOP
		Mix_PauseMusic();
#else
		(*mPlayer)->SetPlayState(mPlayer, SL_PLAYSTATE_PAUSED);
#endif
	}

	void SoundFile::Resume()
	{
#ifdef DESKTOP
		Mix_ResumeMusic();
#else
		ResumeSound(mPlayer);
#endif
	}

#ifdef ANDROID
	void SoundFile::SetVolume(float volume)
	{
		SetSoundVolume(mPlayerObj, mPlayer, volume);
	}
#endif
	float SoundFile::GetVolume() const
	{
#ifdef ANDROID
		return GetSoundVolume(mPlayerObj, mPlayer);
#else
		if(mIsMuted)
		{
			return mVolume;
		}
		else
		{
			float volume = float(Mix_VolumeMusic(-1));
			return volume / float(MIX_MAX_VOLUME);
		}
#endif
	}

	void SoundFile::SetMuted(bool muted)
	{
#ifdef ANDROID
		SetSoundMuted(mPlayerObj, mPlayer, muted);
#else
		SetSoundMuted(muted);
#endif
	}

	bool SoundFile::IsMuted() const
	{
#ifdef ANDROID
		return GetSoundMuted(mPlayerObj, mPlayer);
#else
		return mIsMuted;
#endif
	}

#ifdef DESKTOP
	void SoundFile::SetSoundVolume(int volume)
	{
		Mix_VolumeMusic(volume);
	}

	void SoundFile::MusicStoppedCallback()
	{
		SoundService::GetInstance()->PlayNextSongInQueue();
	}
#else
	void SoundFile::CreateSoundDetails()
	{
		SLresult lRes =
			(*mPlayerObj)->GetInterface(
				mPlayerObj,
				SL_IID_SEEK,
				&mPlayerSeek
				);
		if (lRes != SL_RESULT_SUCCESS)
		{
			star::Logger::GetInstance()->Log(star::LogLevel::Error,
					_T("SoundFile : Can't get audio seek interface"));
			Stop();
			return;
		}
	}

	void SoundFile::RegisterCallback(SLPlayItf & player)
	{
		if((*player)->RegisterCallback(
			player, MusicStoppedCallback,
			&player) != SL_RESULT_SUCCESS)
		{
			star::Logger::GetInstance()->Log(star::LogLevel::Error,
				_T("SoundFile: Can't set callback"));
		}
	}

	void SoundFile::MusicStoppedCallback(
		SLPlayItf caller,
		void *pContext,SLuint32 event
		)
	{
		SoundFile* file =
			reinterpret_cast<SoundFile*>(pContext);

		star::Logger::GetInstance()->Log(star::LogLevel::Info,
			_T("Sound File: Callback Entered, Looptimes = ") +
			star::string_cast<tstring>(file->mLoopTimes)
			);

		if(file->mLoopTimes == 0)
		{
			SLPlayItf pPlay = file->mPlayer;
			(*pPlay)->SetPlayState(pPlay, SL_PLAYSTATE_STOPPED);
			file->mbStopped = true;
			if(file->mbQueuedPlay)
			{
				star::SoundService::GetInstance()->PlayNextSongInQueue();
			}
		}
		else
		{
			SLPlayItf pPlay = file->mPlayer;
			(*pPlay)->SetPlayState(pPlay, SL_PLAYSTATE_STOPPED);
			file->mLoopTimes -= 1;
			file->Play(file->mLoopTimes);
		}
	}
#endif
}
