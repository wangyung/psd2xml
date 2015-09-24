#include "pch.h"
#include "PsdLogger.h"

PsdLogger::PsdLogger()
{
}

PsdLogger::~PsdLogger()
{
}

void PsdLogger::formatMessage(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(mMessage, sizeof(mMessage), fmt, args);
    onMessage(mMessage);
}

void PsdLogger::onMessage(const char *msg)
{
    fputs(msg, stderr);
    if (msg[strlen(msg) - 1] != '\n')
        fputc('\n', stderr);
}

PsdLogger *PsdLogger::getDefaultInstance()
{
    static PsdLogger defaultInstance;

    return &defaultInstance;
}
