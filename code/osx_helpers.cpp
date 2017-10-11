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

void UpdateWindowTitle(GLFWwindow* window, const char* baseName, Version_t* platformVersion, Version_t* appVersion)
{
	char windowTitle[128] = {};
	
	#if DEBUG
		snprintf(windowTitle, ArrayCount(windowTitle),
			"%s (Platform %u.%u:%03u App %u.%u:%03u)", baseName,
			platformVersion->major, platformVersion->minor, platformVersion->build,
			appVersion->major, appVersion->minor, appVersion->build);
	#else
		snprintf(windowTitle, ArrayCount(windowTitle),
			"%s (v%u.%u)", baseName,
			appVersion->major, appVersion->minor);
	#endif
	
	glfwSetWindowTitle(window, windowTitle);
}

pid_t popen2(const char* command, int* stdInHandleOut, int* stdOutHandleOut)
{
    int p_stdin[2], p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0) { return -1; }

    pid = fork();

    if (pid < 0) { return pid; }
    else if (pid == 0)
    {
        close(p_stdin[1]);
        dup2(p_stdin[0], 0);
        close(p_stdout[0]);
        dup2(p_stdout[1], 1);

        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl");
        exit(1);
    }

    if (stdInHandleOut == NULL) { close(p_stdin[1]); }
    else { *stdInHandleOut = p_stdin[1]; }

    if (stdOutHandleOut == NULL) { close(p_stdout[0]); }
    else { *stdOutHandleOut = p_stdout[0]; }

    return pid;
}
