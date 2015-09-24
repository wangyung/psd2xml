#pragma once
#ifndef PSDLOGGER_H
#define PSDLOGGER_H

// Intent to be subclassed and passed to PsdFile()
// to override the logging target.
// The subclass is reasonable to implement onMessage()
class PSDFILE_API PsdLogger
{
public:
    virtual ~PsdLogger();

    void formatMessage(const char *fmt, ...);

    virtual void onMessage(const char *msg);

    static PsdLogger *getDefaultInstance();

protected:
    PsdLogger();

private:
    char mMessage[2048];
};

#ifdef LIBPSDFILE_INTERNAL

#define LOG_ALWAYS(...) (LOG_INSTANCE)->formatMessage(__VA_ARGS__)

#ifdef NDEBUG
#define LOG_DEBUG(...) ((void) 0)
#else
#define LOG_DEBUG(...) LOG_ALWAYS(__VA_ARGS__)
#endif

#endif // LIBPSDFILE_INTERNAL

#endif
