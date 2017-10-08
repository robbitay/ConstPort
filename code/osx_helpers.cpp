/*
File:   osx_helpers.cpp
Author: Taylor Robbins
Date:   10\07\2017
Description: 
	** Holds any helpful functions used by the platform 
*/

const char* GetErrnoName(i32 errorNo)
{
	switch (errorNo)
	{
		case EPERM:           return "EPERM";
		case ENOENT:          return "ENOENT";
		case ESRCH:           return "ESRCH";
		case EINTR:           return "EINTR";
		case EIO:             return "EIO";
		case ENXIO:           return "ENXIO";
		case E2BIG:           return "E2BIG";
		case ENOEXEC:         return "ENOEXEC";
		case EBADF:           return "EBADF";
		case ECHILD:          return "ECHILD";
		case EDEADLK:         return "EDEADLK";
		case ENOMEM:          return "ENOMEM";
		case EACCES:          return "EACCES";
		case EFAULT:          return "EFAULT";
		case ENOTBLK:         return "ENOTBLK";
		case EBUSY:           return "EBUSY";
		case EEXIST:          return "EEXIST";
		case EXDEV:           return "EXDEV";
		case ENODEV:          return "ENODEV";
		case ENOTDIR:         return "ENOTDIR";
		case EISDIR:          return "EISDIR";
		case EINVAL:          return "EINVAL";
		case ENFILE:          return "ENFILE";
		case EMFILE:          return "EMFILE";
		case ENOTTY:          return "ENOTTY";
		case ETXTBSY:         return "ETXTBSY";
		case EFBIG:           return "EFBIG";
		case ENOSPC:          return "ENOSPC";
		case ESPIPE:          return "ESPIPE";
		case EROFS:           return "EROFS";
		case EMLINK:          return "EMLINK";
		case EPIPE:           return "EPIPE";
		case EDOM:            return "EDOM";
		case ERANGE:          return "ERANGE";
		case EAGAIN:          return "EAGAIN";
		case EINPROGRESS:     return "EINPROGRESS";
		case EALREADY:        return "EALREADY";
		case ENOTSOCK:        return "ENOTSOCK";
		case EDESTADDRREQ:    return "EDESTADDRREQ";
		case EMSGSIZE:        return "EMSGSIZE";
		case EPROTOTYPE:      return "EPROTOTYPE";
		case ENOPROTOOPT:     return "ENOPROTOOPT";
		case EPROTONOSUPPORT: return "EPROTONOSUPPORT";
		case ESOCKTNOSUPPORT: return "ESOCKTNOSUPPORT";
		case ENOTSUP:         return "ENOTSUP";
		case EPFNOSUPPORT:    return "EPFNOSUPPORT";
		case EAFNOSUPPORT:    return "EAFNOSUPPORT";
		case EADDRINUSE:      return "EADDRINUSE";
		case EADDRNOTAVAIL:   return "EADDRNOTAVAIL";
		case ENETDOWN:        return "ENETDOWN";
		case ENETUNREACH:     return "ENETUNREACH";
		case ENETRESET:       return "ENETRESET";
		case ECONNABORTED:    return "ECONNABORTED";
		case ECONNRESET:      return "ECONNRESET";
		case ENOBUFS:         return "ENOBUFS";
		case EISCONN:         return "EISCONN";
		case ENOTCONN:        return "ENOTCONN";
		case ESHUTDOWN:       return "ESHUTDOWN";
		case ETOOMANYREFS:    return "ETOOMANYREFS";
		case ETIMEDOUT:       return "ETIMEDOUT";
		case ECONNREFUSED:    return "ECONNREFUSED";
		case ELOOP:           return "ELOOP";
		case ENAMETOOLONG:    return "ENAMETOOLONG";
		case EHOSTDOWN:       return "EHOSTDOWN";
		case EHOSTUNREACH:    return "EHOSTUNREACH";
		case ENOTEMPTY:       return "ENOTEMPTY";
		case EPROCLIM:        return "EPROCLIM";
		case EUSERS:          return "EUSERS";
		case EDQUOT:          return "EDQUOT";
		case ESTALE:          return "ESTALE";
		case EREMOTE:         return "EREMOTE";
		case EBADRPC:         return "EBADRPC";
		case ERPCMISMATCH:    return "ERPCMISMATCH";
		case EPROGUNAVAIL:    return "EPROGUNAVAIL";
		case EPROGMISMATCH:   return "EPROGMISMATCH";
		case EPROCUNAVAIL:    return "EPROCUNAVAIL";
		case ENOLCK:          return "ENOLCK";
		case ENOSYS:          return "ENOSYS";
		case EFTYPE:          return "EFTYPE";
		case EAUTH:           return "EAUTH";
		case ENEEDAUTH:       return "ENEEDAUTH";
		case EPWROFF:         return "EPWROFF";
		case EDEVERR:         return "EDEVERR";
		case EOVERFLOW:       return "EOVERFLOW";
		case EBADEXEC:        return "EBADEXEC";
		case EBADARCH:        return "EBADARCH";
		case ESHLIBVERS:      return "ESHLIBVERS";
		case EBADMACHO:       return "EBADMACHO";
		case ECANCELED:       return "ECANCELED";
		case EIDRM:           return "EIDRM";
		case ENOMSG:          return "ENOMSG";
		case EILSEQ:          return "EILSEQ";
		case ENOATTR:         return "ENOATTR";
		case EBADMSG:         return "EBADMSG";
		case EMULTIHOP:       return "EMULTIHOP";
		case ENODATA:         return "ENODATA";
		case ENOLINK:         return "ENOLINK";
		case ENOSR:           return "ENOSR";
		case ENOSTR:          return "ENOSTR";
		case EPROTO:          return "EPROTO";
		case ETIME:           return "ETIME";
		case EOPNOTSUPP:      return "EOPNOTSUPP";
		case ENOPOLICY:       return "ENOPOLICY";
		case ENOTRECOVERABLE: return "ENOTRECOVERABLE";
		case EOWNERDEAD:      return "EOWNERDEAD";
		case EQFULL:          return "EQFULL";
		default: return "EUNKNOWN";
	};
}

