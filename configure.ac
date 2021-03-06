#
# Copyright (C) 2010-2013 Mamadou DIOP
# Copyright (C) 2011-2013 Doubango Telecom <http://www.doubango.org>
# License: GPLv3 or commercial
# This file is part of Doubango VoIP framework <https://code.google.com/p/doubango/>
#

AC_PREREQ([2.0])
AC_INIT(libdoubango, 2.0.1007, doubango(at)googlegroups(dot)com)
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([tinySAK/src/tsk.c])

dnl find suitable C and C++ compilers
AC_PROG_CC
AC_PROG_CXX

AC_PROG_INSTALL

AC_PROG_LIBTOOL
AC_LIBTOOL_WIN32_DLL

AC_PROG_SED

AC_PROG_MAKE_SET

PKG_PROG_PKG_CONFIG

## Are we using Windows?
#dnl detecting WIN32
#case "$host" in
#	*cygwin* | *mingw*)
#		AC_DEFINE([WIN32], [], "Using Windows as target os.")
#		;;
#	*)
#		AC_MSG_RESULT(${host} is not Windows... I will do my best.)
#		;;
#esac

#echo Setting up build environment for ${target_cpu}${target_os}

### ENABLE-DEBUG
AC_MSG_CHECKING(whether to enable debugging)
debug_default="no"
AC_ARG_ENABLE(debug, 
[  --enable-debug[=no/yes] turn on/off debugging 
                       [[default=$debug_default]]],
[ if test "x$enableval" = "xyes" ; then
	CFLAGS="$CFLAGS -O0 -g3 -DDEBUG"
	AC_MSG_RESULT(yes)
  else
	CFLAGS="$CFLAGS -O3 -NDEBUG"
	AC_MSG_RESULT(no)
  fi
],
[])

### DEBUG-LEVEL
AC_ARG_WITH(debug-level, 
[  --with-debug-level[=level] Set debug-level to level [[default=info]]
                       info: informational
                       warn: warnings
                       error [default]: errors
                       fatal: fatal],
[ if test "x$withval" = "xinfo" ; then
	AC_DEFINE(DEBUG_LEVEL, DEBUG_LEVEL_INFO, [info debug])
	AC_MSG_RESULT(info)
  elif test "x$withval" = "xwarn" ; then
	AC_DEFINE(DEBUG_LEVEL, DEBUG_LEVEL_INFO, [warn debug])
	AC_MSG_RESULT(warn)
  elif test "x$withval" = "xerror" ; then
	AC_DEFINE(DEBUG_LEVEL, DEBUG_LEVEL_ERROR, [error debug])
	AC_MSG_RESULT(error)
  elif test "x$withval" = "xfatal" ; then
	AC_DEFINE(DEBUG_LEVEL, DEBUG_LEVEL_FATAL, [fatal debug])
	AC_MSG_RESULT(fatal)
  fi
])

AC_CHECK_FUNCS([stricmp strnicmp])

AC_TRY_COMPILE([#include <pthread.h>],
    [static int k = PTHREAD_MUTEX_RECURSIVE;],
    [AC_DEFINE(TSK_RECURSIVE_MUTEXATTR, PTHREAD_MUTEX_RECURSIVE,
        [Check whether PTHREAD_MUTEX_RECURSIVE is defined])],
[AC_TRY_COMPILE([#include <pthread.h>],
    [static int k = PTHREAD_MUTEX_RECURSIVE_NP;],
        [AC_DEFINE(TSK_RECURSIVE_MUTEXATTR, PTHREAD_MUTEX_RECURSIVE_NP,
            [Check whether PTHREAD_MUTEX_RECURSIVE_NP is defined])])])


AC_DEFINE(USE_POLL, 1, [Setting USE_POLL to 1 for backward compatibility])
AC_CHECK_FUNCS([poll getdtablesize opendir closedir getpid])

AC_CHECK_HEADERS([net/if_types.h net/if_dl.h poll.h unistd.h dirent.h fcntl.h sys/param.h sys/resource.h])

AC_CHECK_FUNC(getifaddrs, AC_DEFINE(HAVE_GETIFADDRS, 1 ,[Define to 1 if you have the 'getifaddrs' function]))
AH_TEMPLATE([HAVE_IFADDRS_H], [Define if <ifaddrs.h> header exist])
AC_CHECK_HEADER([ifaddrs.h], AC_DEFINE(HAVE_IFADDRS_H, 1), AC_DEFINE(HAVE_IFADDRS_H, 0), [])

AH_TEMPLATE([HAVE_NET_IF_DL_H], [Define if <net/if_dl.h> header exist])
AC_CHECK_HEADER([net/if_dl.h], AC_DEFINE(HAVE_NET_IF_DL_H, 1), AC_DEFINE(HAVE_NET_IF_DL_H, 0), [])

AH_TEMPLATE([HAVE_NET_IF_TYPES_H], [Define if <net/if_types> header exist])
AC_CHECK_HEADER([net/if_types.h], AC_DEFINE(HAVE_NET_IF_TYPES_H, 1), AC_DEFINE(HAVE_NET_IF_TYPES_H, 0), [])

AH_TEMPLATE([HAVE_NETPACKET_PACKET_H], [Define if <netpacket/packet.h> header exist])
AC_CHECK_HEADER([netpacket/packet.h], AC_DEFINE(HAVE_NETPACKET_PACKET_H, 1), AC_DEFINE(HAVE_NETPACKET_PACKET_H, 0), [])

AH_TEMPLATE([HAVE_NET_ROUTE_H], [Define if <net/route.h> header exist])
AC_CHECK_HEADER([net/route.h], AC_DEFINE(HAVE_NET_ROUTE_H, 1), AC_DEFINE(HAVE_NET_ROUTE_H, 0), [])
AC_CHECK_TYPES([struct rt_metrics], [], [],
[
#if HAVE_NET_ROUTE_H
# include <net/route.h>
#endif
])
AC_CHECK_TYPES([struct sockaddr_dl], [], [],
[
#if HAVE_NET_IF_DL_H
# include <net/if_dl.h>
#endif
])
AC_CHECK_TYPES([struct rlimit], [AC_CHECK_FUNCS([getrlimit setrlimit])], [],
[
#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
])
AC_CHECK_TYPES([struct dirent], [], [],
[
#if HAVE_DIRENT_H
# include <dirent.h>
#endif
])

have_resolv=no
AH_TEMPLATE([HAVE_DNS_H], [Define if <dns.h> header exist])
AM_CONDITIONAL(USE_RESOLV, false)
AC_CHECK_LIB([resolv], [dns_search],
    AC_SUBST(LIBRESOLV_LIBADD, "-lresolv")
    AM_CONDITIONAL(USE_RESOLV, true)
    [have_resolv=yes], 
    AC_SUBST(LIBRESOLV_LIBADD, "")
    )
# check for <dns.h> only if libresolv exist
test $have_resolv = yes &&  
	AC_CHECK_HEADER([dns.h], AC_DEFINE(HAVE_DNS_H, 1), AC_DEFINE(HAVE_DNS_H, 0), [])

AM_CONDITIONAL(USE_RT, false)
have_rt=no
AC_CHECK_LIB([rt], [clock_gettime], 
    AC_SUBST(LIBRT_LIBADD, "-lrt")
    AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [Define to 1 if we have the `clock_gettime' function.])
    AM_CONDITIONAL(USE_RT, true)
    [have_rt=yes], 
    AC_SUBST(LIBRT_LIBADD, "")
    )

AH_TEMPLATE([TNET_HAVE_SS_LEN], [Define if sockaddr_storage.ss_len exists])
AC_CHECK_MEMBER([struct sockaddr_storage.ss_len], AC_DEFINE(TNET_HAVE_SS_LEN, 1), AC_DEFINE(TNET_HAVE_SS_LEN,0), [#include <sys/socket.h>])

AH_TEMPLATE([TNET_HAVE_SA_LEN], [Define if sockaddr.sa_len exists])
AC_CHECK_MEMBER([struct sockaddr.sa_len], AC_DEFINE(TNET_HAVE_SA_LEN, 1), AC_DEFINE(TNET_HAVE_SA_LEN, 0), [#include <netinet/in.h>])

########################################################
# SRTP
# --with-srtp / --without-srtp as argument to configure
########################################################
AH_TEMPLATE([HAVE_APPEND_SALT_TO_KEY], [Checks if the installed libsrtp version support append_salt_to_key() function])
AH_TEMPLATE([HAVE_SRTP_PROFILE_GET_MASTER_KEY_LENGTH], [Checks if the installed libsrtp version support srtp_profile_get_master_key_length() function])
AH_TEMPLATE([HAVE_SRTP_PROFILE_GET_MASTER_SALT_LENGTH], [Checks if the installed libsrtp version support srtp_profile_get_master_salt_length() function])
have_srtp=check
want_srtp=check
path_srtp=undef
AC_SUBST(LIBSRTP_LIBADD, "")
AC_ARG_WITH(srtp,
[  --with-srtp=PATH             Build with SRTP (requires libsrtp). PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_srtp=yes
      path_srtp=undef
    elif test "x$withval" = "xno"; then
      want_srtp=no
      path_srtp=undef
    elif test "$withval"; then
      want_srtp=yes
      path_srtp=$withval
    fi,
)
# only if --without-srtp not used
if test $want_srtp != no; then
	# check for lib and headers
	AC_CHECK_HEADERS(srtp/srtp.h,
       	AC_CHECK_LIB(srtp, srtp_create, AC_DEFINE_UNQUOTED(HAVE_SRTP, 1, HAVE_SRTP) [have_srtp=yes] LIBSRTP_LIBADD="-lsrtp"
		AC_CHECK_LIB(srtp, append_salt_to_key, AC_DEFINE(HAVE_APPEND_SALT_TO_KEY, 1), AC_DEFINE(HAVE_APPEND_SALT_TO_KEY, 0))
 		AC_CHECK_LIB(srtp, srtp_profile_get_master_key_length, AC_DEFINE(HAVE_SRTP_PROFILE_GET_MASTER_KEY_LENGTH, 1), AC_DEFINE(HAVE_SRTP_PROFILE_GET_MASTER_KEY_LENGTH, 0))
		AC_CHECK_LIB(srtp, srtp_profile_get_master_salt_length, AC_DEFINE(HAVE_SRTP_PROFILE_GET_MASTER_SALT_LENGTH, 1), AC_DEFINE(HAVE_SRTP_PROFILE_GET_MASTER_SALT_LENGTH, 0))
		, 
		AC_DEFINE_UNQUOTED(HAVE_SRTP, 0, HAVE_SRTP) [have_srtp=no]
	))

	# if srtp not found and requested then, die.
  	test $have_srtp:$want_srtp = no:yes &&  
		AC_MSG_ERROR([You requested SRTP (requires libsrtp) but not found...die])
fi

AM_CONDITIONAL([USE_SRTP], [test $have_srtp = yes])



########################################################
# SSL
# --with-ssl / --without-ssl as argument to configure
########################################################
have_ssl=check
have_dtls_srtp=check
have_dtls=check
want_ssl=check
path_ssl=undef
AC_SUBST(LIBSSL_LIBADD, "")
AC_ARG_WITH(ssl,
[  --with-ssl=PATH             Build with SSL (requires OpenSSL). PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_ssl=yes
      path_ssl=undef
    elif test "x$withval" = "xno"; then
      want_ssl=no
      path_ssl=undef
    elif test "$withval"; then
      want_ssl=yes
      path_ssl=$withval
    fi,
)
# only if --without-ssl not used
if test $want_ssl != no; then
	# check for lib and headers
	AC_CHECK_HEADERS(openssl/ssl.h,
       	AC_CHECK_LIB(ssl, SSL_library_init, AC_DEFINE_UNQUOTED(HAVE_OPENSSL, 1, HAVE_OPENSSL) [have_ssl=yes] LIBSSL_LIBADD="${LIBSSL_LIBADD} -lssl", 
		AC_DEFINE_UNQUOTED(HAVE_OPENSSL, 0, HAVE_OPENSSL) [have_ssl=no]
	))
	AC_CHECK_LIB(crypto, BN_init, LIBSSL_LIBADD="${LIBSSL_LIBADD} -lcrypto")
	AC_CHECK_LIB(ssl, SSL_CTX_set_tlsext_use_srtp, AC_DEFINE_UNQUOTED(HAVE_OPENSSL_DTLS_SRTP, 1, HAVE_OPENSSL_DTLS_SRTP) [have_dtls_srtp=yes])
 	AC_CHECK_LIB(ssl, DTLSv1_method, AC_DEFINE_UNQUOTED(HAVE_OPENSSL_DTLS, 1, HAVE_OPENSSL_DTLS) [have_dtls=yes])
	# if opus not found and requested then, die.
  	test $have_ssl:$want_ssl = no:yes &&  
		AC_MSG_ERROR([You requested SSL (requires OpenSSL) but not found...die])
fi
AM_CONDITIONAL([USE_SSL], [test $have_ssl = yes])



########################################################
# FFmpeg
# --with-ffmpeg / --without-ffmpeg as argument to configure
########################################################
have_ffmpeg=check
want_ffmpeg=check
path_ffmpeg=undef
AC_SUBST(LIBFFMPEG_LIBADD, "")
AC_ARG_WITH(ffmpeg,
[  --with-ffmpeg=PATH             Build with FFmpeg (h264, h263, theora, mp4v-es). PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_ffmpeg=yes
      path_ffmpeg=undef
    elif test "x$withval" = "xno"; then
      want_ffmpeg=no
      path_ffmpeg=undef
    elif test "$withval"; then
      want_ffmpeg=yes
      path_ffmpeg=$withval
    fi,
)
# only if --without-opus not used
if test $want_ffmpeg != no; then
	have_ffmpeg=yes
	have_avutil=check
	AC_CHECK_HEADERS(libavutil/avutil.h, 
		AC_CHECK_LIB(avutil, av_gettime, [have_avutil=yes] LIBFFMPEG_LIBADD="${LIBFFMPEG_LIBADD} -lavutil", [have_avutil=no] [have_ffmpeg=no])
	)
	have_swscale=check
	AC_CHECK_HEADERS(libswscale/swscale.h, 
		AC_CHECK_LIB(swscale, sws_scale, [have_swscale=yes] LIBFFMPEG_LIBADD="${LIBFFMPEG_LIBADD} -lswscale", [have_swscale=no] [have_ffmpeg=no])
	)
	have_avcodec=check
	AC_CHECK_HEADERS(libavcodec/avcodec.h, 
		AC_CHECK_LIB(avcodec, avcodec_find_encoder, [have_avcodec=yes] LIBFFMPEG_LIBADD="${LIBFFMPEG_LIBADD} -lavcodec", [have_avcodec=no] [have_ffmpeg=no])
	)
	
	test $have_ffmpeg = yes &&
		AC_DEFINE_UNQUOTED(HAVE_FFMPEG, 1, HAVE_FFMPEG)
	
	# if ffmpeg not found and requested then, die.
  	test $have_ffmpeg:$want_ffmpeg = no:yes &&  
		AC_MSG_ERROR([You requested FFmpeg (h264, h263, theora, mp4v-es) but not found...die])
fi
AM_CONDITIONAL([USE_FFMPEG], [test $have_ffmpeg = yes])


AM_CONDITIONAL(USE_H264, false)
AC_SUBST(LIBH264_LIBADD, "")
AC_ARG_WITH(h264, [  --with-h264 IMPORTANT: deprecated to be removed ],[], [])

########################################################
# VPX (VP8, VP9)
# --with-vpx / --without-vpx as argument to configure
########################################################
have_vpx=check
want_vpx=check
path_vpx=undef
AC_SUBST(LIBVPX_LIBADD, "")
AC_ARG_WITH(vpx,
[  --with-vpx=PATH             Build with VPX(VP8, VP9) video codecs. PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_vpx=yes
      path_vpx=undef
    elif test "x$withval" = "xno"; then
      want_vpx=no
      path_vpx=undef
    elif test "$withval"; then
      want_vpx=yes
      path_vpx=$withval
    fi,
)
# only if --without-vpx not used
if test $want_vpx != no; then
	# check for lib and headers
	AC_CHECK_HEADERS(vpx/vpx_encoder.h vpx/vpx_decoder.h,
       	AC_CHECK_LIB(vpx, vpx_codec_encode, AC_DEFINE_UNQUOTED(HAVE_LIBVPX, 1, HAVE_LIBVPX) [have_vpx=yes] LIBVPX_LIBADD="-lvpx", 
		AC_DEFINE_UNQUOTED(HAVE_LIBVPX, 0, HAVE_LIBVPX) [have_vpx=no]
	))	  
	# if opus not found and requested then, die.
  	test $have_vpx:$want_vpx = no:yes &&  
		AC_MSG_ERROR([You requested VPX(VP8, VP9) video codecs but not found...die])
fi
AM_CONDITIONAL([USE_VPX], [test $have_vpx = yes])



########################################################
# YUV 
# --with-yuv / --without-yuv as argument to configure
########################################################
have_yuv=check
want_yuv=check
path_yuv=undef
AC_SUBST(LIBYUV_INCLUDE, "")
AC_SUBST(LIBYUV_LIB, "")
AC_ARG_WITH(yuv,
[  --with-yuv=PATH             Build with LIBYUV. PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_yuv=yes
      path_yuv=undef
    elif test "x$withval" = "xno"; then
      want_yuv=no
      path_yuv=undef
    elif test "$withval"; then
      want_yuv=yes
      path_yuv=$withval
    fi,
)
# only if --without-yuv not used
if test $want_yuv != no; then
	# check for lib and headers
	AC_CHECK_HEADERS(libyuv/libyuv.h,
       	AC_CHECK_LIB(yuv, I420Scale, AC_DEFINE_UNQUOTED(HAVE_LIBYUV, 1, HAVE_LIBYUV) [have_yuv=yes] 
		if test "x$path_yuv" = xundef; then
  			LIBYUV_INCLUDE="-I/usr/local/include"
  			LIBYUV_LIB="-L/usr/local/lib -lyuv"
		else
  			LIBYUV_INCLUDE="-I${path_yuv}/include"
  			LIBYUV_LIB="-L${path_yuv}/lib -lyuv"
		fi
		, 
		AC_DEFINE_UNQUOTED(HAVE_LIBYUV, 0, HAVE_LIBYUV) [have_yuv=no]
	))	  
	# if opus not found and requested then, die.
  	test $have_yuv:$want_yuv = no:yes &&  
		AC_MSG_ERROR([You requested LIBYUV but not found...die])
fi
AM_CONDITIONAL([USE_YUV], [test $have_yuv = yes])


########################################################
# AMR 
# --with-amr / --without-amr as argument to configure
########################################################
have_amr=check
want_amr=check
path_amr=undef
AC_SUBST(LIBAMR_LIBADD, "")
AC_ARG_WITH(amr,
[  --with-amr=PATH             Build with AMR audio codec (requires opencore-amr library). PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_amr=yes
      path_amr=undef
    elif test "x$withval" = "xno"; then
      want_amr=no
      path_amr=undef
    elif test "$withval"; then
      want_amr=yes
      path_amr=$withval
    fi,
)
# only if --without-amr not used
if test $want_amr != no; then
	# check for lib and headers
	AC_CHECK_HEADERS(opencore-amrnb/interf_dec.h opencore-amrnb/interf_enc.h,
       	AC_CHECK_LIB(opencore-amrnb, Encoder_Interface_init, AC_DEFINE_UNQUOTED(HAVE_OPENCORE_AMR, 1, HAVE_OPENCORE_AMR) [have_amr=yes] LIBAMR_LIBADD="-lopencore-amrnb", 
		AC_DEFINE_UNQUOTED(HAVE_OPENCORE_AMR, 0, HAVE_OPENCORE_AMR) [have_amr=no]
	))	  
	# if amr not found and requested then, die.
  	test $have_amr:$want_amr = no:yes &&  
		AC_MSG_ERROR([You requested AMR audio codec but not found...die])
fi
AM_CONDITIONAL([USE_AMR], [test $have_amr = yes])


########################################################
# OPUS 
# --with-opus / --without-opus as argument to configure
########################################################
have_opus=check
want_opus=check
path_opus=undef
AC_SUBST(LIBOPUS_LIBADD, "")
AC_ARG_WITH(opus,
[  --with-opus=PATH             Build with OPUS audio codec. PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_opus=yes
      path_opus=undef
    elif test "x$withval" = "xno"; then
      want_opus=no
      path_opus=undef
    elif test "$withval"; then
      want_opus=yes
      path_opus=$withval
    fi,
)
# only if --without-opus not used
if test $want_opus != no; then
	# check for lib and headers
	AC_CHECK_HEADERS(opus/opus.h,
       	AC_CHECK_LIB(opus, opus_decoder_create, AC_DEFINE_UNQUOTED(HAVE_LIBOPUS, 1, HAVE_LIBOPUS) [have_opus=yes] LIBOPUS_LIBADD="-lopus", 
		AC_DEFINE_UNQUOTED(HAVE_LIBOPUS, 0, HAVE_LIBOPUS) [have_opus=no]
	))	  
	# if opus not found and requested then, die.
  	test $have_opus:$want_opus = no:yes &&  
		AC_MSG_ERROR([You requested OPUS audio codec but not found...die])
fi
AM_CONDITIONAL([USE_OPUS], [test $have_opus = yes])


########################################################
# SPEEX
# --with-speex / --without-speex as argument to configure
########################################################
have_speex=check
want_speex=check
path_speex=undef
AC_SUBST(LIBSPEEX_LIBADD, "")
AC_ARG_WITH(speex,
[  --with-speex=PATH             Build with SPEEX audio codec. PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_speex=yes
      path_speex=undef
    elif test "x$withval" = "xno"; then
      want_speex=no
      path_speex=undef
    elif test "$withval"; then
      want_speex=yes
      path_speex=$withval
    fi,
)
# only if --without-speex not used
if test $want_speex != no; then
	# check lib and headers
	AC_CHECK_HEADERS(speex/speex.h,
       	AC_CHECK_LIB(speex, speex_decode_int, AC_DEFINE_UNQUOTED(HAVE_LIB_SPEEX, 1, HAVE_LIB_SPEEX) [have_speex=yes] LIBSPEEX_LIBADD="-lspeex", 
		AC_DEFINE_UNQUOTED(HAVE_LIB_SPEEX, 0, HAVE_LIB_SPEEX) [have_speex=no]
	))
	# if speex not found and requested then, die.
  	test $have_speex:$want_speex = no:yes &&  
		AC_MSG_ERROR([You requested SPEEX audio codec but not found...die])
fi
AM_CONDITIONAL([USE_SPEEX], [test $have_speex = yes])


########################################################
# SPEEXDSP
# --with-speexdsp / --without-speexdsp as argument to configure
########################################################
have_speexdsp=check
want_speexdsp=check
path_speexdsp=undef
AC_SUBST(LIBSPEEXDSP_LIBADD, "")
AC_ARG_WITH(speexdsp,
[  --with-speexdsp=PATH             Build with SPEEX DSP (audio resampler, jitter buffer...). PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_speexdsp=yes
      path_speexdsp=undef
    elif test "x$withval" = "xno"; then
      want_speexdsp=no
      path_speexdsp=undef
    elif test "$withval"; then
      want_speexdsp=yes
      path_speexdsp=$withval
    fi,
)
# only if --without-speexdsp not used
if test $want_speexdsp != no; then
	# check main lib
	AC_CHECK_HEADERS(speex/speex_preprocess.h,
       	AC_CHECK_LIB(speexdsp, speex_preprocess_state_init, AC_DEFINE_UNQUOTED(HAVE_SPEEX_DSP, 1, HAVE_SPEEX_DSP) [have_speexdsp=yes] LIBSPEEXDSP_LIBADD="-lspeexdsp", 
		AC_DEFINE_UNQUOTED(HAVE_SPEEX_DSP, 0, HAVE_SPEEX_DSP) [have_speexdsp=no])
	)
	# check for jitter buffer
	AC_CHECK_HEADERS(speex/speex_jitter.h,
       	AC_CHECK_LIB(speexdsp, jitter_buffer_init, AC_DEFINE_UNQUOTED(HAVE_SPEEX_JB, 1, HAVE_SPEEX_JB), 
		AC_DEFINE_UNQUOTED(HAVE_SPEEX_JB, 0, HAVE_SPEEX_JB))
	)
	# check for resampler
	AC_CHECK_HEADERS(speex/speex_resampler.h,
       	AC_CHECK_LIB(speexdsp, speex_resampler_init, AC_DEFINE_UNQUOTED(HAVE_SPEEX_RESAMPLER, 1, HAVE_SPEEX_RESAMPLER), 
		AC_DEFINE_UNQUOTED(HAVE_SPEEX_RESAMPLER, 0, HAVE_SPEEX_RESAMPLER))
	)
	# check for denoiser and echo canceler
	AC_CHECK_HEADERS(speex/speex_echo.h,
       	AC_CHECK_LIB(speexdsp, speex_echo_playback, AC_DEFINE_UNQUOTED(HAVE_SPEEX_DENOISE, 1, HAVE_SPEEX_DENOISE), 
		AC_DEFINE_UNQUOTED(HAVE_SPEEX_DENOISE, 0, HAVE_SPEEX_DENOISE))
	)
	# if speex not found and requested then, die.
  	test $have_speexdsp:$want_speexdsp = no:yes &&  
		AC_MSG_ERROR([You requested SPEEX DSP but not found...die])
fi
AM_CONDITIONAL([USE_SPEEXDSP], [test $have_speexdsp = yes])

# Deprecated options: to be removed
AC_ARG_ENABLE(speexresampler, [ --enable-speexresampler IMPORTANT: deprecated to be removed], , )
AC_ARG_ENABLE(speexdenoiser, [ --enable-speexdenoiser IMPORTANT: deprecated to be removed], , )
AC_ARG_ENABLE(speexjb, [ --enable-speexjb IMPORTANT: deprecated to be removed], , )

########################################################
# GSM
# --with-gsm / --without-gsm as argument to configure
########################################################
have_gsm=check
want_gsm=check
path_gsm=undef
AC_SUBST(LIBGSM_LIBADD, "")
AC_ARG_WITH(gsm,
[  --with-gsm=PATH             Build with GSM audio codec. PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_gsm=yes
      path_gsm=undef
    elif test "x$withval" = "xno"; then
      want_gsm=no
      path_gsm=undef
    elif test "$withval"; then
      want_gsm=yes
      path_gsm=$withval
    fi,
)
# only if --without-gsm not used
if test $want_gsm != no; then
	# check lib and headers
	AC_CHECK_HEADERS(gsm.h,
       	AC_CHECK_LIB(gsm, gsm_create, AC_DEFINE_UNQUOTED(HAVE_LIBGSM, 1, HAVE_LIBGSM) [have_gsm=yes] LIBGSM_LIBADD="-lgsm", 
		AC_DEFINE_UNQUOTED(HAVE_LIBGSM, 0, HAVE_LIBGSM) [have_gsm=no]
	))
	# if gsm not found and requested then, die.
  	test $have_gsm:$want_gsm = no:yes &&  
		AC_MSG_ERROR([You requested GSM audio codec but not found...die])
fi
AM_CONDITIONAL([USE_GSM], [test $have_gsm = yes])


########################################################
# G729
# --with-g729 / --without-g729 as argument to configure
########################################################
have_g729=check
want_g729=check
path_g729=undef
AC_SUBST(LIBG729_LIBADD, "")
AC_ARG_WITH(g729,
[  --with-g729=PATH             Build with G.729 audio codec. PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_g729=yes
      path_g729=undef
    elif test "x$withval" = "xno"; then
      want_g729=no
      path_g729=undef
    elif test "$withval"; then
      want_g729=yes
      path_g729=$withval
    fi,
)
# only if --without-g729 not used
if test $want_g729 != no; then
	# check lib and headers
	AC_CHECK_HEADERS(g729b/typedef.h g729b/ld8a.h,
       	AC_CHECK_LIB(g729b, Init_Decod_ld8a, AC_DEFINE_UNQUOTED(HAVE_G729, 1, HAVE_G729) [have_g729=yes] LIBG729_LIBADD="-lg729b", 
		AC_DEFINE_UNQUOTED(HAVE_G729, 0, HAVE_G729) [have_g729=no]), [],
		[[#ifdef HAVE_G729B_TYPEDEF_H
     		# include <g729b/typedef.h>
     		#endif
     		]])
	
	# if g729 not found and requested then, die.
  	test $have_g729:$want_g729 = no:yes &&  
		AC_MSG_ERROR([You requested G729 audio codec but not found...die])
fi
AM_CONDITIONAL([USE_G729], [test $have_g729 = yes])


########################################################
# ILBC
# --with-ilbc / --without-ilbc as argument to configure
########################################################
have_ilbc=check
want_ilbc=check
path_ilbc=undef
AC_SUBST(LIBILBC_LIBADD, "")
AC_ARG_WITH(ilbc,
[  --with-ilbc=PATH             Build with ILBC audio codec. PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_ilbc=yes
      path_ilbc=undef
    elif test "x$withval" = "xno"; then
      want_ilbc=no
      path_ilbc=undef
    elif test "$withval"; then
      want_ilbc=yes
      path_ilbc=$withval
    fi,
)
# only if --without-ilbc not used
if test $want_ilbc != no; then
	# check lib and headers
	AC_CHECK_HEADERS(ilbc/iLBC_define.h ilbc/iLBC_encode.h ilbc/iLBC_decode.h,
       	AC_CHECK_LIB(iLBC, initDecode, AC_DEFINE_UNQUOTED(HAVE_ILBC, 1, HAVE_ILBC) [have_ilbc=yes] LIBILBC_LIBADD="-liLBC", 
		AC_DEFINE_UNQUOTED(HAVE_ILBC, 0, HAVE_ILBC) [have_ilbc=no], [-lm]
	))
	# if gsm not found and requested then, die.
  	test $have_ilbc:$want_ilbc = no:yes &&  
		AC_MSG_ERROR([You requested ILBC audio codec but not found...die])
fi
AM_CONDITIONAL([USE_ILBC], [test $have_ilbc = yes])


AC_CONFIG_FILES(
Makefile
doubango.pc

tinySAK/Makefile
tinySAK/tinySAK.pc

tinyNET/Makefile
tinyNET/tinyNET.pc

tinyHTTP/Makefile
tinyHTTP/tinyHTTP.pc

tinyXCAP/Makefile
tinyXCAP/tinyXCAP.pc

tinyIPSec/Makefile
tinyIPSec/tinyIPSec.pc

tinySMS/Makefile
tinySMS/tinySMS.pc

tinySIGCOMP/Makefile
tinySIGCOMP/tinySIGCOMP.pc

tinySDP/Makefile
tinySDP/tinySDP.pc

tinyRTP/Makefile
tinyRTP/tinyRTP.pc

tinyMSRP/Makefile
tinyMSRP/tinyMSRP.pc

tinyMEDIA/Makefile
tinyMEDIA/tinyMEDIA.pc

tinyDAV/Makefile
tinyDAV/tinyDAV.pc

tinySIP/Makefile
tinySIP/tinySIP.pc
)
AC_OUTPUT




AC_MSG_NOTICE([
 
**************************************************************************
* 				CONGRATULATIONS
**************************************************************************
Host setup:          ${host}
Install prefix:      ${prefix}
Compiler:            ${CC}

FFmpeg:              $have_ffmpeg
VP8 video codec:     $have_vpx
OPUS audio codec:    $have_opus
ILBC audio codec:    $have_ilbc
G.729 audio codec:   $have_g729
GSM audio codec:     $have_gsm
AMR audio codec:     $have_amr
SPEEX audio codec:   $have_speex
G.722 audio codec:   yes
G.711 audio codec:   yes

YUV:                 $have_yuv
SPEEX DSP:           $have_speexdsp

SSL:                 $have_ssl
DTLS-SRTP:           $have_dtls_srtp
DTLS:                $have_dtls

SRTP:                $have_srtp

Monotonic timers:    $have_rt
RESOLV:              $have_resolv

DEBUG:               $debug_default

Report any issue at https://groups.google.com/group/doubango

])