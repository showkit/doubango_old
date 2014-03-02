/*
 * Copyright (C) 2010-2011 Mamadou Diop.
 *
 * Contact: Mamadou Diop <diopmamadou(at)doubango.org>
 *
 * This file is part of Open Source Doubango Framework.
 *
 * DOUBANGO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DOUBANGO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DOUBANGO.
 *
 */
#include "tinydav/audio/coreaudio/tdav_producer_audiounit.h"

// http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/AudioUnitHostingGuide_iOS/Introduction/Introduction.html%23//apple_ref/doc/uid/TP40009492-CH1-SW1

#if HAVE_COREAUDIO_AUDIO_UNIT

#include <mach/mach.h>
#import <sys/sysctl.h>

#include "tsk_string.h"
#include "tsk_memory.h"
#include "tsk_thread.h"
#include "tsk_debug.h"

#define kRingPacketCount				+10
// If the "ptime" value is less than "kMaxPtimeBeforeUsingCondVars", then we can use nonosleep() function instead of conditional
// variables for better performance.
// When the prodcuer's stop() function is called we will wait until the sender thread exist (using join()) this is
// why "kMaxPtimeBeforeUsingCondVars" should be small. This problem will not happen when using conditional variables: thanks to braodcast().
#define kMaxPtimeBeforeUsingCondVars	+500 /* milliseconds */

static void *__sender_thread(void *param);
static int __sender_thread_set_realtime(uint32_t ptime);

static OSStatus __handle_input_buffer(void *inRefCon,
                                      AudioUnitRenderActionFlags *ioActionFlags,
                                      const AudioTimeStamp *inTimeStamp,
                                      UInt32 inBusNumber,
                                      UInt32 inNumberFrames,
                                      AudioBufferList *ioData) {
	OSStatus status = noErr;
	tdav_producer_audiounit_t* producer = (tdav_producer_audiounit_t*)inRefCon;
	
	// holder
	AudioBuffer buffer;
	buffer.mData =  tsk_null;
	buffer.mDataByteSize = 0;
	buffer.mNumberChannels = TMEDIA_PRODUCER(producer)->audio.channels;
	
	// list of holders
	AudioBufferList buffers;
	buffers.mNumberBuffers = 1;
	buffers.mBuffers[0] = buffer;
	
	// render to get frames from the system
	status = AudioUnitRender(tdav_audiounit_handle_get_instance(producer->audioUnitHandle),
							 ioActionFlags,
							 inTimeStamp,
							 inBusNumber,
							 inNumberFrames,
							 &buffers);
	if(status == 0){
		tsk_mutex_lock(producer->ring.mutex);
		speex_buffer_write(producer->ring.buffer, buffers.mBuffers[0].mData, buffers.mBuffers[0].mDataByteSize);
		tsk_mutex_unlock(producer->ring.mutex);
	}
	
    return status;
}

static int __sender_thread_set_realtime(uint32_t ptime) {
    struct thread_time_constraint_policy policy;
	int params [2] = {CTL_HW, HW_BUS_FREQ};
	int ret;
	
	// get bus frequence
	int freq_ns, freq_ms;
	size_t size = sizeof (freq_ns);
	if((ret = sysctl (params, 2, &freq_ns, &size, NULL, 0))){
		// check errno for more information
		TSK_DEBUG_INFO("sysctl() failed with error code=%d", ret);
		return ret;
	}
	freq_ms = freq_ns/1000;
	
	/*
	 * THREAD_TIME_CONSTRAINT_POLICY:
	 *
	 * This scheduling mode is for threads which have real time
	 * constraints on their execution.
	 *
	 * Parameters:
	 *
	 * period: This is the nominal amount of time between separate
	 * processing arrivals, specified in absolute time units.  A
	 * value of 0 indicates that there is no inherent periodicity in
	 * the computation.
	 *
	 * computation: This is the nominal amount of computation
	 * time needed during a separate processing arrival, specified
	 * in absolute time units.
	 *
	 * constraint: This is the maximum amount of real time that
	 * may elapse from the start of a separate processing arrival
	 * to the end of computation for logically correct functioning,
	 * specified in absolute time units.  Must be (>= computation).
	 * Note that latency = (constraint - computation).
	 *
	 * preemptible: This indicates that the computation may be
	 * interrupted, subject to the constraint specified above.
	 */
	policy.period = (ptime/2) * freq_ms; // Half of the ptime
	policy.computation = 2 * freq_ms;
	policy.constraint = 3 * freq_ms;
	policy.preemptible = true;
	
	if ((ret = thread_policy_set(mach_thread_self(),
                                 THREAD_TIME_CONSTRAINT_POLICY, (int *)&policy,
                                 THREAD_TIME_CONSTRAINT_POLICY_COUNT)) != KERN_SUCCESS) {
		TSK_DEBUG_ERROR("thread_policy_set failed(period=%u,computation=%u,constraint=%u) failed with error code= %d",
						policy.period, policy.computation, policy.constraint,
						ret);
		return ret;
	}
	return 0;
}

static void *__sender_thread(void *param)
{
	TSK_DEBUG_INFO("__sender_thread::ENTER");
	
	tdav_producer_audiounit_t* producer = (tdav_producer_audiounit_t*)param;
	uint32_t ptime = TMEDIA_PRODUCER(producer)->audio.ptime;
	tsk_ssize_t avail;
	
	// interval to sleep when using nonosleep() instead of conditional variable
	struct timespec interval;
	interval.tv_sec = (long)(ptime/1000);
	interval.tv_nsec = (long)(ptime%1000) * 1000000;
	
	// change thread priority
    //#if TARGET_OS_IPHONE
	__sender_thread_set_realtime(TMEDIA_PRODUCER(producer)->audio.ptime);
    //#endif
	
	// starts looping
	for (;;) {
		// wait for "ptime" milliseconds
		if(ptime <= kMaxPtimeBeforeUsingCondVars){
			nanosleep(&interval, 0);
		}
		else {
			tsk_condwait_timedwait(producer->senderCondWait, (uint64_t)ptime);
		}
		// check state
		if(!producer->started){
			break;
		}
		// read data and send them
		if(TMEDIA_PRODUCER(producer)->enc_cb.callback) {
			tsk_mutex_lock(producer->ring.mutex);
			avail = speex_buffer_get_available(producer->ring.buffer);
			while (producer->started && avail >= producer->ring.chunck.size) {
				avail -= speex_buffer_read(producer->ring.buffer, producer->ring.chunck.buffer, producer->ring.chunck.size);
				TMEDIA_PRODUCER(producer)->enc_cb.callback(TMEDIA_PRODUCER(producer)->enc_cb.callback_data,
														   producer->ring.chunck.buffer, producer->ring.chunck.size);
			}
			tsk_mutex_unlock(producer->ring.mutex);
		}
		else;
	}
	TSK_DEBUG_INFO("__sender_thread::EXIT");
	return tsk_null;
}

/* ============ Media Producer Interface ================= */
int tdav_producer_audiounit_set(tmedia_producer_t* self, const tmedia_param_t* param)
{
    tdav_producer_audiounit_t* producer = (tdav_producer_audiounit_t*)self;
	if(param->plugin_type == tmedia_ppt_producer){
		if(param->value_type == tmedia_pvt_int32){
			if(tsk_striequals(param->key, "mute")){
				producer->muted = TSK_TO_INT32((uint8_t*)param->value);
				return tdav_audiounit_handle_mute(((tdav_producer_audiounit_t*)self)->audioUnitHandle, producer->muted);
			}
		}
	}
	return tdav_producer_audio_set(TDAV_PRODUCER_AUDIO(self), param);
}

static int tdav_producer_audiounit_prepare(tmedia_producer_t* self, const tmedia_codec_t* codec)
{
	static UInt32 flagOne = 1;
	UInt32 param;
	// static UInt32 flagZero = 0;
#define kInputBus  1
	
	tdav_producer_audiounit_t* producer = (tdav_producer_audiounit_t*)self;
	OSStatus status = noErr;
	AudioStreamBasicDescription audioFormat;
	AudioStreamBasicDescription	deviceFormat;
	
	if(!producer || !codec || !codec->plugin){
		TSK_DEBUG_ERROR("Invalid parameter");
		return -1;
	}
	if(!producer->audioUnitHandle){
		if(!(producer->audioUnitHandle = tdav_audiounit_handle_create(TMEDIA_PRODUCER(producer)->session_id))){
			TSK_DEBUG_ERROR("Failed to get audio unit instance for session with id=%lld", TMEDIA_PRODUCER(producer)->session_id);
			return -3;
		}
	}
	
	// enable
	status = AudioUnitSetProperty(tdav_audiounit_handle_get_instance(producer->audioUnitHandle),
								  kAudioOutputUnitProperty_EnableIO,
								  kAudioUnitScope_Input,
								  kInputBus,
								  &flagOne,
								  sizeof(flagOne));
	if(status != noErr){
		TSK_DEBUG_ERROR("AudioUnitSetProperty(kAudioOutputUnitProperty_EnableIO) failed with status=%ld", (signed long)status);
		return -4;
	}
	else {
#if !TARGET_OS_IPHONE // strange: TARGET_OS_MAC is equal to '1' on Smulator
		// disable output
		param = 0;
		status = AudioUnitSetProperty(tdav_audiounit_handle_get_instance(producer->audioUnitHandle),
									  kAudioOutputUnitProperty_EnableIO,
									  kAudioUnitScope_Output,
									  0,
									  &param,
									  sizeof(UInt32));
		if(status != noErr){
			TSK_DEBUG_ERROR("AudioUnitSetProperty(kAudioOutputUnitProperty_EnableIO) failed with status=%ld", (signed long)status);
			return -4;
		}
		
		// set default audio device
		param = sizeof(AudioDeviceID);
		AudioDeviceID inputDeviceID;
		status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &param, &inputDeviceID);
		if(status != noErr){
			TSK_DEBUG_ERROR("AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice) failed with status=%ld", (signed long)status);
			return -4;
		}
		
		// set the current device to the default input unit
		status = AudioUnitSetProperty(tdav_audiounit_handle_get_instance(producer->audioUnitHandle),
									  kAudioOutputUnitProperty_CurrentDevice,
									  kAudioUnitScope_Output,
									  0,
									  &inputDeviceID,
									  sizeof(AudioDeviceID));
		if(status != noErr){
			TSK_DEBUG_ERROR("AudioUnitSetProperty(kAudioOutputUnitProperty_CurrentDevice) failed with status=%ld", (signed long)status);
			return -4;
		}
#endif /* TARGET_OS_MAC */
		
		/* codec should have ptime */
		TMEDIA_PRODUCER(producer)->audio.channels = codec->plugin->audio.channels;
		TMEDIA_PRODUCER(producer)->audio.rate = codec->plugin->rate;
		TMEDIA_PRODUCER(producer)->audio.ptime = codec->plugin->audio.ptime;
		
		// get device format
		param = sizeof(AudioStreamBasicDescription);
		status = AudioUnitGetProperty(tdav_audiounit_handle_get_instance(producer->audioUnitHandle),
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Input,
                                      kInputBus,
                                      &deviceFormat, &param);
		if(status == noErr && deviceFormat.mSampleRate){
#if TARGET_OS_IPHONE
			// iOS support 8Khz, 16kHz and 32kHz => do not override the sampleRate
#elif TARGET_OS_MAC
			// For example, iSight supports only 48kHz
			TMEDIA_PRODUCER(producer)->audio.rate = deviceFormat.mSampleRate;
#endif
		}
		
		// set format
		audioFormat.mSampleRate = TMEDIA_PRODUCER(producer)->audio.rate;
		audioFormat.mFormatID = kAudioFormatLinearPCM;
		audioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
		audioFormat.mChannelsPerFrame = TMEDIA_PRODUCER(producer)->audio.channels;
		audioFormat.mFramesPerPacket = 1;
		audioFormat.mBitsPerChannel = TMEDIA_PRODUCER(producer)->audio.bits_per_sample;
		audioFormat.mBytesPerPacket = audioFormat.mBitsPerChannel / 8 * audioFormat.mChannelsPerFrame;
		audioFormat.mBytesPerFrame = audioFormat.mBytesPerPacket;
		audioFormat.mReserved = 0;
		if(audioFormat.mFormatID == kAudioFormatLinearPCM && audioFormat.mChannelsPerFrame  == 1){
			audioFormat.mFormatFlags &= ~kLinearPCMFormatFlagIsNonInterleaved;
		}
		status = AudioUnitSetProperty(tdav_audiounit_handle_get_instance(producer->audioUnitHandle),
									  kAudioUnitProperty_StreamFormat,
									  kAudioUnitScope_Output,
									  kInputBus,
									  &audioFormat,
                                      sizeof(audioFormat));
		if(status){
			TSK_DEBUG_ERROR("AudioUnitSetProperty(kAudioUnitProperty_StreamFormat) failed with status=%ld", (signed long)status);
			return -5;
		}
		else {
			
			// configure
			if(tdav_audiounit_handle_configure(producer->audioUnitHandle, tsk_false, TMEDIA_PRODUCER(producer)->audio.ptime, &audioFormat)){
				TSK_DEBUG_ERROR("tdav_audiounit_handle_set_rate(%d) failed", TMEDIA_PRODUCER(producer)->audio.rate);
				return -4;
			}
			
			// set callback function
			AURenderCallbackStruct callback;
			callback.inputProc = __handle_input_buffer;
			callback.inputProcRefCon = producer;
			status = AudioUnitSetProperty(tdav_audiounit_handle_get_instance(producer->audioUnitHandle),
										  kAudioOutputUnitProperty_SetInputCallback,
										  kAudioUnitScope_Output,
										  kInputBus,
										  &callback,
										  sizeof(callback));
			if(status){
				TSK_DEBUG_ERROR("AudioUnitSetProperty(kAudioOutputUnitProperty_SetInputCallback) failed with status=%ld", (signed long)status);
				return -6;
			}
			else {
				// disbale buffer allocation as we will provide ours
				//status = AudioUnitSetProperty(tdav_audiounit_handle_get_instance(producer->audioUnitHandle),
				//							  kAudioUnitProperty_ShouldAllocateBuffer,
				//							  kAudioUnitScope_Output,
				//							  kInputBus,
				//							  &flagZero,
				//							  sizeof(flagZero));
				
				int packetperbuffer = (1000 / codec->plugin->audio.ptime);
				producer->ring.chunck.size = audioFormat.mSampleRate * audioFormat.mBytesPerFrame / packetperbuffer;
				// allocate our chunck buffer
				if(!(producer->ring.chunck.buffer = tsk_realloc(producer->ring.chunck.buffer, producer->ring.chunck.size))){
					TSK_DEBUG_ERROR("Failed to allocate new buffer");
					return -7;
				}
				// create mutex for ring buffer
				if(!producer->ring.mutex && !(producer->ring.mutex = tsk_mutex_create_2(tsk_false))){
					TSK_DEBUG_ERROR("Failed to create new mutex");
					return -8;
				}
				// create ringbuffer
				producer->ring.size = kRingPacketCount * producer->ring.chunck.size;
				if(!producer->ring.buffer){
					producer->ring.buffer = speex_buffer_init(producer->ring.size);
				}
				else {
					int ret;
					if((ret = speex_buffer_resize(producer->ring.buffer, producer->ring.size)) < 0){
						TSK_DEBUG_ERROR("speex_buffer_resize(%d) failed with error code=%d", producer->ring.size, ret);
						return ret;
					}
				}
				if(!producer->ring.buffer){
					TSK_DEBUG_ERROR("Failed to create a new ring buffer with size = %d", producer->ring.size);
					return -9;
				}
			}
            
		}
	}
	
	TSK_DEBUG_INFO("AudioUnit producer prepared");
	return tdav_audiounit_handle_signal_producer_prepared(producer->audioUnitHandle);;
}

static int tdav_producer_audiounit_start(tmedia_producer_t* self)
{
	tdav_producer_audiounit_t* producer = (tdav_producer_audiounit_t*)self;
	
    if(!producer){
		TSK_DEBUG_ERROR("Invalid parameter");
		return -1;
	}
	if(producer->paused){
		producer->paused = tsk_false;
		return tsk_false;
	}
	
	int ret;
	if(producer->started){
		TSK_DEBUG_WARN("Already started");
		return 0;
	}
	else {
		ret = tdav_audiounit_handle_start(producer->audioUnitHandle);
		if(ret){
			TSK_DEBUG_ERROR("tdav_audiounit_handle_start failed with error code=%d", ret);
			return ret;
		}
	}
	producer->started = tsk_true;
    
    // apply parameters (because could be lost when the producer is restarted -handle recreated-)
    ret = tdav_audiounit_handle_mute(producer->audioUnitHandle, producer->muted);
	
	// create conditional variable
	if(!(producer->senderCondWait = tsk_condwait_create())){
		TSK_DEBUG_ERROR("Failed to create conditional variable");
		return -2;
	}
	// start the reader thread
	ret = tsk_thread_create(&producer->senderThreadId[0], __sender_thread, producer);
	if(ret){
		TSK_DEBUG_ERROR("Failed to start the sender thread. error code=%d", ret);
		return ret;
	}
    
	TSK_DEBUG_INFO("AudioUnit producer started");
	return 0;
}

static int tdav_producer_audiounit_pause(tmedia_producer_t* self)
{
    tdav_producer_audiounit_t* producer = (tdav_producer_audiounit_t*)self;
    if(!producer){
		TSK_DEBUG_ERROR("Invalid parameter");
		return -1;
	}
	producer->paused = tsk_true;
	TSK_DEBUG_INFO("AudioUnit producer paused");
	return 0;
}

static int tdav_producer_audiounit_stop(tmedia_producer_t* self)
{
    tdav_producer_audiounit_t* producer = (tdav_producer_audiounit_t*)self;
	
    if(!producer){
		TSK_DEBUG_ERROR("Invalid parameter");
		return -1;
	}
	if(!producer->started){
		TSK_DEBUG_INFO("Not started");
		return 0;
	}
	else {
		int ret = tdav_audiounit_handle_stop(producer->audioUnitHandle);
		if(ret){
			TSK_DEBUG_ERROR("tdav_audiounit_handle_stop failed with error code=%d", ret);
			// do not return even if failed => we MUST stop the thread!
		}
#if TARGET_OS_IPHONE
		//https://devforums.apple.com/thread/118595
		if(producer->audioUnitHandle){
			tdav_audiounit_handle_destroy(&producer->audioUnitHandle);
		}
#endif
	}
	producer->started = tsk_false;
	// signal
	if(producer->senderCondWait){
		tsk_condwait_broadcast(producer->senderCondWait);
	}
	// stop thread
	if(producer->senderThreadId[0]){
		tsk_thread_join(&(producer->senderThreadId[0]));
	}
	TSK_DEBUG_INFO("AudioUnit producer stoppped");
	return 0;
}


//
//	CoreAudio producer object definition
//
/* constructor */
static tsk_object_t* tdav_producer_audiounit_ctor(tsk_object_t * self, va_list * app)
{
	tdav_producer_audiounit_t *producer = self;
	if(producer){
		/* init base */
		tdav_producer_audio_init(TDAV_PRODUCER_AUDIO(producer));
		/* init self */
	}
	return self;
}
/* destructor */
static tsk_object_t* tdav_producer_audiounit_dtor(tsk_object_t * self)
{
	tdav_producer_audiounit_t *producer = self;
	if(producer){
		// Stop the producer if not done
		if(producer->started){
			tdav_producer_audiounit_stop(self);
		}
		
		// Free all buffers and dispose the queue
        if (producer->audioUnitHandle) {
			tdav_audiounit_handle_destroy(&producer->audioUnitHandle);
        }
		if(producer->ring.mutex){
			tsk_mutex_destroy(&producer->ring.mutex);
		}
        TSK_FREE(producer->ring.chunck.buffer);
		if(producer->ring.buffer){
			speex_buffer_destroy(producer->ring.buffer);
		}
		if(producer->senderCondWait){
			tsk_condwait_destroy(&producer->senderCondWait);
		}
		/* deinit base */
		tdav_producer_audio_deinit(TDAV_PRODUCER_AUDIO(producer));
	}
	
	return self;
}
/* object definition */
static const tsk_object_def_t tdav_producer_audiounit_def_s =
{
	sizeof(tdav_producer_audiounit_t),
	tdav_producer_audiounit_ctor,
	tdav_producer_audiounit_dtor,
	tdav_producer_audio_cmp,
};
/* plugin definition*/
static const tmedia_producer_plugin_def_t tdav_producer_audiounit_plugin_def_s =
{
	&tdav_producer_audiounit_def_s,
	
	tmedia_audio,
	"Apple CoreAudio producer (AudioUnit)",
	
	tdav_producer_audiounit_set,
	tdav_producer_audiounit_prepare,
	tdav_producer_audiounit_start,
	tdav_producer_audiounit_pause,
	tdav_producer_audiounit_stop
};
const tmedia_producer_plugin_def_t *tdav_producer_audiounit_plugin_def_t = &tdav_producer_audiounit_plugin_def_s;


#endif /* HAVE_COREAUDIO_AUDIO_UNIT */
