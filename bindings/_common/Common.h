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
#ifndef TINYWRAP_COMMON_H
#define TINYWRAP_COMMON_H

#if ANDROID
#	define dyn_cast	static_cast
#	define __JNIENV JNIEnv
#else
#	define dyn_cast	dynamic_cast
#	define __JNIENV void
#endif

typedef enum twrap_media_type_e
{
	// because of Java don't use OR
	twrap_media_none = 0x00,

	twrap_media_audio = 0x01,
	twrap_media_video = 0x02,
	twrap_media_audiovideo = 0x03,

	twrap_media_msrp = 0x04,
    twrap_media_data = 0x08,
    twrap_media_audiovideodata = 0xB
}
twrap_media_type_t;

#endif /* TINYWRAP_COMMON_H */

